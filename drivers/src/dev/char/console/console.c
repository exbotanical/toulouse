#include "drivers/dev/char/console/console.h"

#include "drivers/dev/char/console/csi.h"
#include "drivers/dev/char/console/sysconsole.h"
#include "drivers/dev/char/keyboard.h"
#include "drivers/dev/char/ps2.h"
#include "drivers/dev/char/tty/termbits.h"
#include "drivers/dev/char/tty/tty.h"
#include "drivers/dev/char/vga.h"
#include "drivers/dev/char/video.h"
#include "drivers/dev/device.h"
#include "drivers/dev/kd.h"
#include "fs/ops.h"
#include "kconfig.h"
#include "lib/ctype.h"
#include "lib/string.h"
#include "mem/alloc.h"
#include "mem/page.h"

static vconsole_t consoles[NUM_CONSOLES + 1];

short int  current_console;
short int* vc_screen[NUM_CONSOLES + 1];

static fs_operations_t tty_driver_fs_ops = {
  .flags              = 0,
  .fsdev              = 0,
  // TODO:
  // .open               = tty_open,
  // .close              = tty_close,
  .read               = tty_read,
  .write              = tty_write,
  // .ioctl              = tty_ioctl,
  // .llseek             = tty_llseek,
  .readdir            = NULL,
  .readdir64          = NULL,
  .mmap               = NULL,
  // .select             = tty_select,
  .readlink           = NULL,
  .followlink         = NULL,
  .bmap               = NULL,
  .lookup             = NULL,
  .rmdir              = NULL,
  .link               = NULL,
  .unlink             = NULL,
  .symlink            = NULL,
  .mkdir              = NULL,
  .mknod              = NULL,
  .truncate           = NULL,
  .create             = NULL,
  .rename             = NULL,
  .read_block         = NULL,
  .write_block        = NULL,
  .read_inode         = NULL,
  .write_inode        = NULL,
  .ialloc             = NULL,
  .ifree              = NULL,
  .statfs             = NULL,
  .read_superblock    = NULL,
  .remount_fs         = NULL,
  .write_superblock   = NULL,
  .release_superblock = NULL,
};

static device_t tty_device = {
  .name              = "vconsole",
  .major             = VCONSOLE_MAJOR,
  .minors            = {0, 0, 0, 0, 0, 0, 0, 0},
  .minor_block_sizes = 0,
  .device_data       = NULL,
  .fs_ops            = &tty_driver_fs_ops,
  .requests_queue    = NULL,
  .xfer_data         = NULL,
  .next              = NULL,
};

static device_t console_device = {
  .name              = "console",
  .major             = SYSCON_MAJOR,
  .minors            = {0, 0, 0, 0, 0, 0, 0, 0},
  .minor_block_sizes = 0,
  .device_data       = NULL,
  .fs_ops            = &tty_driver_fs_ops,
  .requests_queue    = NULL,
  .xfer_data         = NULL,
  .next              = NULL,
};

static bool
is_vconsole (unsigned short int dev) {
  if (DEVICE_MAJOR(dev) == VCONSOLE_MAJOR && DEVICE_MINOR(dev) <= NUM_CONSOLES) {
    return true;
  }

  return false;
}

static void
set_default_color_attr (vconsole_t* vc) {
  vc->color_attr = DEFAULT_MODE;
  vc->bold       = 0;
  vc->underline  = 0;
  vc->blink      = 0;
  vc->reverse    = 0;
}

static void
init_vt_mode (vconsole_t* vc) {
  vc->vt_mode.mode   = VT_AUTO;
  vc->vt_mode.waitv  = 0;
  vc->vt_mode.relsig = 0;
  vc->vt_mode.acqsig = 0;
  vc->vt_mode.frsig  = 0;
  vc->vc_mode        = KD_TEXT;
  vc->tty->pid       = 0;
  vc->switch_tty     = -1;
}

static void
vconsole_start (tty_t* tty) {
  vconsole_t* vc = (vconsole_t*)tty->data;
  if (!vc->scrlock_on) {
    return;
  }

  vc->led_status &= ~LED_SCRLBIT;
  vc->scrlock_on  = false;
  ps2_set_leds(vc->led_status);
}

static void
vconsole_stop (tty_t* tty) {
  vconsole_t* vc = (vconsole_t*)tty->data;
  if (vc->scrlock_on) {
    return;
  }

  vc->led_status |= LED_SCRLBIT;
  vc->scrlock_on  = true;
  ps2_set_leds(vc->led_status);
}

static void
vconsole_delete_tab (tty_t* tty) {
  vconsole_t*   vc    = (vconsole_t*)tty->data;
  cblock_t*     cb    = tty->cooked_q.head;
  int           col   = 0;
  unsigned char count = 0;

  while (cb) {
    for (int i = 0; i < cb->next_write_index; i++) {
      if (i >= cb->next_read_index) {
        unsigned char ch = cb->data[i];
        if (ch == '\t') {
          while (!vc->tty->tab_stop[++col]);
        } else {
          col++;
          if (ISCTRL(ch) && !ISSPACE(ch) && tty->termios.c_lflag & ECHOCTL) {
            col++;
          }
        }
        col %= vc->columns;
      }
    }
    cb = cb->next;
  }

  count = vc->x - col;
  while (count--) {
    charq_put_char(&tty->write_q, '\b');
  }
}

static void
vconsole_reset (tty_t* tty) {
  vconsole_t* vc = (vconsole_t*)tty->data;

  vc->top        = 0;
  // TODO: video.rows
  vc->rows       = video.lines;
  vc->columns    = video.columns;
  vc->check_x    = false;

  vc->led_status = 0;
  ps2_set_leds(vc->led_status);

  vc->scrlock_on = vc->numlock_on = vc->capslock_on = false;
  vc->has_esc = vc->has_bracket = vc->has_semicolon = vc->has_question = false;

  vc->tmp_storage_1 = vc->tmp_storage_2 = 0;
  vc->tmp_storage_num_entries           = 0;
  kmemset(vc->tmp_storage, 0, sizeof(vc->tmp_storage));

  set_default_color_attr(vc);
  vc->saved_x = vc->saved_y = 0;

  for (int n = 0; n < MAX_TAB_COLS; n++) {
    if (!(n % TAB_SIZE)) {
      vc->tty->tab_stop[n] = true;
    } else {
      vc->tty->tab_stop[n] = false;
    }
  }

  termios_reset(tty);
  vc->tty->win.ws_row    = vc->rows - vc->top;
  vc->tty->win.ws_col    = vc->columns;
  vc->tty->win.ws_xpixel = 0;
  vc->tty->win.ws_ypixel = 0;
  vc->tty->flags         = 0;

  init_vt_mode(vc);
  vc->flags &= ~CONSOLE_CLEARED;
  video.update_cursor_pos(vc);
}

static void
vconsole_write (tty_t* tty) {
  vconsole_t* vc = (vconsole_t*)tty->data;

  if (vc->flags & CONSOLE_HAS_FOCUS) {
    if (video.buf_top) {
      video.restore_screen(vc);
      video.buf_top = 0;
      video.show_cursor(vc, CURSOR_MODE_ON);
      video.update_cursor_pos(vc);
    }
  }

  unsigned char ch         = 0;
  bool          is_numeric = false;

  while (!vc->scrlock_on && tty->write_q.size > 0) {
    ch = charq_get_char(&tty->write_q);

    // Parse Control Sequence Introducer (CSI)
    if (vc->has_esc) {
      if (vc->has_bracket) {
        if (IS_NUMERIC(ch)) {
          is_numeric = true;
          // TODO:
          is_numeric;

          if (vc->has_semicolon) {
            // Converts char digit to number
            vc->tmp_storage_2 *= 10;
            vc->tmp_storage_2 += ch - '0';
          } else {
            vc->tmp_storage_1 *= 10;
            vc->tmp_storage_1 += ch - '0';
          }

          vc->tmp_storage[vc->tmp_storage_num_entries] *= 10;
          vc->tmp_storage[vc->tmp_storage_num_entries] += ch - '0';
          continue;
        }

        switch (ch) {
          case ';': {
            vc->has_semicolon = true;
            vc->tmp_storage_2 = 0;
            vc->tmp_storage_num_entries++;
            continue;
          }

          case '?': {
            vc->has_question = true;
            continue;
          }

          case '@': {
            // Insert Character(s) <ESC>[ n @`
            vc->tmp_storage_1 = !vc->tmp_storage_1 ? true : vc->tmp_storage_1;
            csi_at(vc, vc->tmp_storage_1);
            vc->has_esc = false;
            continue;
          }

          default: break;
        }
      }
    }
  }
}

void
vconsole_select (int new_cons) {}

void
console_init (void) {
  for (int num = 1; num <= NUM_CONSOLES; num++) {
    deviceno_t devnum = DEVICE_MKDEV(VCONSOLE_MAJOR, num);

    if (tty_register(devnum) == RET_OK) {
      tty_t* tty              = tty_get(devnum);
      tty->data               = (void*)&consoles[num];

      tty->stop               = vconsole_stop;
      tty->start              = vconsole_start;
      tty->delete_tab         = vconsole_delete_tab;
      tty->reset              = vconsole_reset;
      tty->input              = tty_cook_input;
      tty->output             = vconsole_write;

      vconsole_t this_console = consoles[num];

      this_console.tty        = tty;

      if (video_using_vga()) {
        this_console.back_buffer = (short int*)kmalloc(PAGE_SIZE);
      }
      if (video_using_vesa_framebuffer()) {
        this_console.back_buffer = vc_screen[num];
      }

      this_console.main_buffer = NULL;
      kmemset(this_console.back_buffer, CLEAR_MEM, CONSOLE_SIZE);
      vconsole_reset(tty);
    }
  }

  current_console         = 1;

  vconsole_t this_console = consoles[current_console];

  video.show_cursor(&this_console, CURSOR_MODE_ON);
  this_console.main_buffer  = (unsigned char*)video.address;
  this_console.flags       |= CONSOLE_HAS_FOCUS;

  if (video_using_vga()) {
    kmemcpy(this_console.main_buffer, video.address, CONSOLE_SIZE);
  }

  video.get_cursor_pos(&this_console);
  video.update_cursor_pos(&this_console);
  video.buf_y   = this_console.y;
  video.buf_top = 0;

  DEVICE_SET_MINOR(console_device.minors, 0);
  DEVICE_SET_MINOR(console_device.minors, 1);

  for (unsigned int n = 0; n < NUM_CONSOLES; n++) {
    DEVICE_SET_MINOR(tty_device.minors, n);
  }

  device_register(DEVTYPE_CHAR, &console_device);
  device_register(DEVTYPE_CHAR, &tty_device);

  tty_t* tty;

  unsigned int syscon = 0;
  // TODO: consistent iter var name
  for (unsigned int i = 0; i < NUM_SYSCONSOLES; i++) {
    if (is_vconsole(sysconsole_table[i].dev) && (tty = tty_get(sysconsole_table[i].dev))) {
      if (!syscon) {
        syscon = tty->devnum;
      }

      sysconsole_register(tty);
    }
  }

  if (syscon) {
    tty = tty_get(syscon);
    // TODO: Flush anything that has been logged up until now into the first console
    // flush_log_buf(tty);
  }
}
