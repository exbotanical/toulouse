#include "drivers/dev/char/keyboard.h"

#include "arch/x86.h"
#include "drivers/dev/char/charq.h"
#include "drivers/dev/char/console/console.h"
#include "drivers/dev/char/keymap.h"
#include "drivers/dev/char/ps2.h"
#include "drivers/dev/char/sysrq.h"
#include "drivers/dev/char/tty/tty.h"
#include "drivers/dev/char/video.h"
#include "drivers/dev/device.h"
#include "interrupt/irq.h"
#include "kconfig.h"
#include "proc/lock.h"
#include "proc/proc.h"
#include "proc/sleep.h"

#define DELAY_250  0x00 /* Typematic delay at 250ms (default) */
#define DELAY_500  0x40 /* Typematic delay at 500ms */
#define DELAY_750  0x80 /* Typematic delay at 750ms */
#define DELAY_1000 0xC0 /* Typematic delay at 1000ms */
#define RATE_30    0x00 /* Typematic rate at 30.0 reports/sec (default) */

extern diacritic_t grave_table[NUM_DIACR];

static interrupt_bh_t keyboard_irq_callback = {
  .flags = 0,
  .fn    = &keyboard_bh_irq,
  .next  = NULL,
};

static interrupt_t keyboard_config_irq = {
  .ticks   = 0,
  .name    = "keyboard",
  .handler = &keyboard_irq,
  .next    = NULL,
};

static unsigned char e0_keys[128] = {
  0,      0,      0,     0,      0,       0,       0, 0,      /* 0x00-0x07 */
  0,      0,      0,     0,      0,       0,       0, 0,      /* 0x08-0x0F */
  0,      0,      0,     0,      0,       0,       0, 0,      /* 0x10-0x17 */
  0,      0,      0,     0,      E0ENTER, RCTRL,   0, 0,      /* 0x18-0x1F */
  0,      0,      0,     0,      0,       0,       0, 0,      /* 0x20-0x27 */
  0,      0,      0,     0,      0,       0,       0, 0,      /* 0x28-0x2F */
  0,      0,      0,     0,      0,       E0SLASH, 0, 0,      /* 0x30-0x37 */
  ALTGR,  0,      0,     0,      0,       0,       0, 0,      /* 0x38-0x3F */
  0,      0,      0,     0,      0,       0,       0, E0HOME, /* 0x40-0x47 */
  E0UP,   E0PGUP, 0,     E0LEFT, 0,       E0RIGHT, 0, E0END,  /* 0x48-0x4F */
  E0DOWN, E0PGDN, E0INS, E0DEL,  0,       0,       0, 0,      /* 0x50-0x57 */
  0,      0,      0,     0,      0,       0,       0, 0,      /* 0x58-0x5f */
  0,      0,      0,     0,      0,       0,       0, 0,      /* 0x60-0x67 */
  0,      0,      0,     0,      0,       0,       0, 0,      /* 0x68-0x6F */
  0,      0,      0,     0,      0,       0,       0, 0,      /* 0x70-0x77 */
  0,      0,      0,     0,      0,       0,       0, 0       /* 0x78-0x7F */
};

/**
 * Indicates we're using a PS/2 keyboard device
 */
static unsigned char is_ps2          = 0;

/**
 * Keyboard info
 *
 * 0 -  indicates the keyboard type or protocol version
 * 1 -  additional info like the specific keyboard model or features
 */
static unsigned char kbd_identify[2] = {0, 0};

/**
 * Stores the original scanset prior to our changing it
 */
static unsigned char og_scan_set     = 0;

diacritic_t* diacr;

bool ctrl_alt_del                      = true;
bool any_key_to_reboot                 = false;

static bool shift_pressed              = false;
static bool leds_on                    = false;
static bool altgr_pressed              = false;
static bool ctrl_pressed               = false;
static bool alt_pressed                = false;
static bool extkey_pressed             = false;
static bool altsysrq_pressed           = false;

/**
 * Represents which dead key is pressed, or zero if none
 *
 * A dead key is a key that doesn't immediately produce a character.
 * Instead, it modifies the next keypress to produce an accented or diacritical character.
 */
static unsigned char   deadkey_pressed = 0;
static int             sysrq_op        = 0;
volatile unsigned char ack             = 0;

static char          do_switch_console = -1;
static unsigned char do_buf_scroll     = 0;
static unsigned char do_setleds        = 0;
static unsigned char do_tty_stop       = 0;
static unsigned char do_tty_start      = 0;
static unsigned char do_sysrq          = 0;

static void
putc (tty_t* tty, unsigned char ch) {
  if (tty->open_count) {
    if (charq_put_char(&tty->read_q, ch) < 0) {
      if (tty->termios.c_iflag & IMAXBEL) {
        // TODO:
        // vconsole_beep();
      }
    }
  }
}

static void
puts (tty_t* tty, char* seq) {
  if (tty->open_count) {
    char ch;
    while ((ch = *(seq++))) {
      putc(tty, ch);
    }
  }
}

static void
keyboard_identify (void) {
  // Disable the keyboard to prevent conflicts while we set it up
  ps2_write(PS2_DATA_PORT, PS2_KB_DISABLE);
  if (!ps2_await_ack()) {
    // TODO: log warn
  } else {
    is_ps2++;
  }

  // Identify the keyboard type
  ps2_write(PS2_DATA_PORT, PS2_DEV_IDENTIFY);
  if (!ps2_await_ack()) {
    // TODO: log warn
  } else {
    is_ps2++;
  }

  kbd_identify[0] = ps2_read(PS2_DATA_PORT);
  kbd_identify[1] = ps2_read(PS2_DATA_PORT);

  // Persist PS/2 config
  char config     = 0;
  ps2_write(PS2_COMMAND_PORT, PS2_CMD_RECV_CONFIG);
  config = ps2_read(PS2_DATA_PORT);

  // Unset translation. We're going to retrieve the current scan set and don't want anything in our
  // way
  ps2_write(PS2_COMMAND_PORT, PS2_CMD_SEND_CONFIG);
  ps2_write(PS2_DATA_PORT, config & ~0x40);
  ps2_write(PS2_DATA_PORT, PS2_KB_GETSETSCAN);
  if (ps2_await_ack()) {
    // TODO: log warn
  }

  ps2_write(PS2_DATA_PORT, 0);
  if (ps2_await_ack()) {
    // TODO: log warn
  }

  og_scan_set = ps2_read(PS2_DATA_PORT);
  if (og_scan_set != 2) {
    ps2_write(PS2_DATA_PORT, PS2_KB_GETSETSCAN);
    ps2_write(PS2_DATA_PORT, 2);
    if (ps2_await_ack()) {
      // TODO: log warn
    }
  }

  // Restore state
  ps2_write(PS2_COMMAND_PORT, PS2_CMD_SEND_CONFIG);
  ps2_write(PS2_DATA_PORT, config);

  ps2_write(PS2_DATA_PORT, PS2_DEV_ENABLE);
  if (ps2_await_ack()) {
    // printk("WARNING: %s(): ACK not received on enable command!\n", __FUNCTION__);
  }

  ps2_clear_buffer();
}

void
keyboard_irq (int _num, sig_context_t* _sc) {
  // Grab the current console - we'll redirect the keyboard input to it
  tty_t*      tty      = tty_get(DEVICE_MKDEV(VCONSOLE_MAJOR, current_console));
  vconsole_t* vc       = (vconsole_t*)tty->data;

  unsigned char s_code = inb(PS2_DATA_PORT);

  // If it was an ACK, early exit; we're done
  if (s_code == DEV_ACK) {
    ack = 1;
    return;
  }

  // Make sure we schedule the bottom half handler
  keyboard_irq_callback.flags |= IRQ_BH_ACTIVE;

  // In raw mode, we do no handling; just put it in the tty buffer because presumably some userspace
  // program wants to handle it
  if (tty->kbd_state.mode == KBD_MODE_RAW) {
    // putc(tty, s_code); // TODO:
    return;
  }

  // Detect extended scancode prefix (e.g. alt+arrows)
  if (s_code == EXTKEY) {
    extkey_pressed = true;
    return;
  }

  // Note we mask the input key bitfield on the most significant (7th) bit to extract the key
  // itself. That is, we are masking the press/release bit.
  unsigned short key;
  if (extkey_pressed) {
    key = e0_keys[s_code & 0x7F];
  } else {
    key = s_code & 0x7F;
  }

  if (tty->kbd_state.mode == KBD_MODE_MEDRAW) {
    // By masking with 0x80, we're extracting the press/release bit
    // putc(tty, key | (s_code & 0x80)); TODO:
    extkey_pressed = false;
    return;
  }

  // Note: multiplying by NUM_MODIFIERS scales the index to the start of that key's modifier block
  // inside keymap.
  key = keymap[NUM_MODIFIERS * (s_code & 0x7F)];

  // bit 7 enabled means a key has been released
  // Note: NUM_SCODES is 128, or 0x80 in hex, which is also the pressed/released bit
  if (s_code & NUM_SCODES) {
    switch (key) {
      case CTRL:
      case LCTRL:
      case RCTRL: {
        ctrl_pressed = false;
        break;
      }

      case ALT: {
        if (!extkey_pressed) {
          alt_pressed      = false;
          altsysrq_pressed = false;
        } else {
          altgr_pressed = false;
        }
        break;
      }

      case SHIFT:
      case LSHIFT:
      case RSHIFT: {
        if (!extkey_pressed) {
          shift_pressed = false;
        }
        break;
      }

      case CAPS:
      case NUMS:
      case SCRL: {
        leds_on = false;
        break;
      }

      default: break;
    }

    extkey_pressed = false;
    return;
  }

  // Non-release key
  switch (key) {
    case CAPS: {
      if (!leds_on) {
        vc->led_status  ^= LED_CAPSBIT;
        vc->capslock_on  = !vc->capslock_on;
        do_setleds       = 1;
      }
      leds_on = true;
      return;
    }

    case NUMS: {
      if (!leds_on) {
        vc->led_status ^= LED_NUMSBIT;
        vc->numlock_on  = !vc->capslock_on;
        do_setleds      = 1;
      }
      leds_on = true;
      return;
    }

    case SCRL: {
      if (!leds_on) {
        // pause/resume tty
        if (vc->scrlock_on) {
          do_tty_start = 1;
        } else {
          do_tty_stop = 1;
        }
      }
      leds_on = true;
      return;
    }

    case CTRL:
    case LCTRL:
    case RCTRL: {
      ctrl_pressed = true;
      return;
    }

    case ALT: {
      if (!extkey_pressed) {
        alt_pressed = true;
      } else {
        altgr_pressed = true;
      }
      return;
    }

    case SHIFT:
    case LSHIFT:
    case RSHIFT: {
      shift_pressed  = true;
      extkey_pressed = false;
      return;
    }

    default: break;
  }

  if (ctrl_pressed && alt_pressed && key == DEL) {
    if (ctrl_alt_del) {
      // TODO:
      // reboot();
    } else {
      // TODO:
      // send_sig(&proc_list[PROC_INIT_PID], SIGINT);
    }

    return;
  }

  unsigned short* keymap_line = &keymap[NUM_MODIFIERS * (s_code & 0x7F)];
  unsigned char   mod         = 0;

  keymap_line                 = &keymap[(s_code & 0x7F) * NUM_MODIFIERS];
  mod                         = 0;

  if (vc->capslock_on && (keymap_line[MOD_BASE] & LETTER_KEYS)) {
    mod = !vc->capslock_on ? shift_pressed : vc->capslock_on - shift_pressed;
  } else if (shift_pressed && !extkey_pressed) {
    mod = 1;
  }
  if (altgr_pressed) {
    mod = 2;
  }
  if (ctrl_pressed) {
    mod = 4;
  }
  if (alt_pressed) {
    mod = 8;
  }

  key = keymap_line[mod];

  if (key >= AF1 && key <= AF12) {
    do_switch_console = key - CONS_KEYS;
    return;
  }

  if (shift_pressed && (key == PGUP)) {
    do_buf_scroll = CONS_SCROLL_UP;
    return;
  }

  if (shift_pressed && (key == PGDN)) {
    do_buf_scroll = CONS_SCROLL_DOWN;
    return;
  }

  if (extkey_pressed && (s_code == SLASH_NPAD)) {
    key = SLASH;
  }

  if (any_key_to_reboot) {
    // TODO:
    // reboot();
  }

  // Split the key into two components:
  // The high byte, which indicates what kind of key this is (function key, special key, etc.)
  unsigned short type = key & 0xFF00;
  // The low byte, which indicates the specific key number or code within that type
  unsigned char  c    = key & 0xFF;

  if (altsysrq_pressed) {
    type &= ~META_KEYS;
  }

  switch (type) {
    case FN_KEYS: {
      if (c > (sizeof(fn_seq) / sizeof(char*))) {
        // TODO: log warn unrecognized key
        break;
      }

      puts(tty, fn_seq[c]);
      break;
    }

    case SPEC_KEYS: {
      switch (key) {
        case CR: {
          putc(tty, TAG_CTRL('M'));
          break;
        }

        case SYSRQ: {
          altsysrq_pressed = true;
          break;
        }

        default: {
          break;
        }
      }
      break;
    }

    case PAD_KEYS: {
      if (!vc->numlock_on) {
        puts(tty, pad_seq[c]);
      } else {
        putc(tty, pad_chars[c]);
      }
      break;
    }

    case DEAD_KEYS: {
      if (!deadkey_pressed) {
        switch (c) {
          case GRAVE ^ DEAD_KEYS:
            deadkey_pressed = 1;
            diacr           = grave_table;
            break;
          case ACUTE ^ DEAD_KEYS:
            deadkey_pressed = 2;
            diacr           = acute_table;
            break;
          case CIRCM ^ DEAD_KEYS:
            deadkey_pressed = 3;
            diacr           = circm_table;
            break;
          case DIERE ^ DEAD_KEYS:
            deadkey_pressed = 5;
            diacr           = diere_table;
            break;
        }
        return;
      }

      c               = diacr_chars[c];
      deadkey_pressed = 0;
      putc(tty, c);

      break;
    }

    case META_KEYS: {
      putc(tty, '\033');
      putc(tty, c);
      break;
    }

    case LETTER_KEYS: {
      if (deadkey_pressed) {
        for (int n = 0; n < NUM_DIACR; n++) {
          if (diacr[n].letter == c) {
            c = diacr[n].code;
          }
        }
      }

      putc(tty, c);
      break;
    }

    default: {
      if (altsysrq_pressed) {
        switch (c) {
          case 'l':
            sysrq_op = SYSRQ_STACK;
            do_sysrq = 1;
            break;
          case 'm':
            sysrq_op = SYSRQ_MEMORY;
            do_sysrq = 1;
            break;
          case 't':
            sysrq_op = SYSRQ_TASKS;
            do_sysrq = 1;
            break;
          default:
            sysrq_op = SYSRQ_UNDEF;
            do_sysrq = 1;
            break;
        }
        break;
      }

      if (deadkey_pressed && c == ' ') {
        c = diacr_chars[deadkey_pressed - 1];
      }

      putc(tty, c);
      break;
    }
  }

  deadkey_pressed = false;
}

void
keyboard_bh_irq (sig_context_t* sc) {
  tty_t*      tty = tty_get(DEVICE_MKDEV(VCONSOLE_MAJOR, current_console));
  vconsole_t* vc  = (vconsole_t*)tty->data;

  video.screen_on(vc);
  keyboard_irq_callback.flags |= IRQ_BH_ACTIVE;

  if (do_switch_console >= 0) {
    vconsole_select(do_switch_console);
    do_switch_console = -1;
  }

  if (do_buf_scroll) {
    video.buf_scroll(vc, do_buf_scroll);
    do_buf_scroll = 0;
  }

  if (do_setleds) {
    ps2_set_leds(vc->led_status);
    do_setleds = 0;
  }

  if (do_tty_start) {
    tty->start(tty);
    do_tty_start = do_tty_stop = 0;
  }

  if (do_tty_stop) {
    tty->stop(tty);
    do_tty_start = do_tty_stop = 0;
  }

  if (do_sysrq) {
    do_sysrq = 0;
    // TODO:
    // sysrq(sysrq_op);
  }

  tty = &tty_table[0];
  // TODO: fix
  for (unsigned int i = 0; i < NUM_CONSOLES; i++, tty++) {
    if (!tty->read_q.size) {
      continue;
    }

    if (tty->kbd_state.mode == KBD_MODE_RAW || tty->kbd_state.mode == KBD_MODE_MEDRAW) {
      wakeup(func_as_ptr((void (*)(void))tty_read));
      continue;
    }

    if (lock_area(AREA_TTY_READ)) {
      keyboard_irq_callback.flags |= IRQ_BH_ACTIVE;
      continue;
    }

    tty->input(tty);
    unlock_area(AREA_TTY_READ);
  }
}

void
keyboard_init (void) {
  tty_t*      tty = tty_get(DEVICE_MKDEV(VCONSOLE_MAJOR, current_console));
  vconsole_t* vc  = (vconsole_t*)tty->data;

  video.screen_on(vc);
  video.cursor_blink((unsigned int)vc);

  irq_bottom_half_register(&keyboard_irq_callback);
  if (irq_register(KEYBOARD_IRQ, &keyboard_config_irq) == RET_OK) {
    irq_enable(KEYBOARD_IRQ);
  }

  // Reset the device
  ps2_write(PS2_DATA_PORT, PS2_DEV_RESET);
  if (!ps2_await_ack()) {
    // TODO: log warning
  }

  int errno;
  if ((errno = ps2_read(PS2_DATA_PORT)) != DEV_RESET_OK) {
    // TODO: log warning
  }

  ps2_clear_buffer();

  keyboard_identify();

  // Configure keyboard repeat rate and delay
  ps2_write(PS2_DATA_PORT, PS2_DEV_RATE);
  ps2_await_ack();
  ps2_write(PS2_DATA_PORT, DELAY_250 | RATE_30);
  ps2_await_ack();
}
