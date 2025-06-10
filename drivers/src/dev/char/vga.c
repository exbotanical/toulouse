#include "drivers/dev/char/vga.h"

#include "arch/eflags.h"
#include "arch/interrupt.h"
#include "arch/x86.h"
#include "drivers/dev/char/console/console.h"
#include "drivers/dev/char/video.h"
#include "init/bios.h"
#include "lib/string.h"
#include "mem/base.h"

typedef enum {
  SCREEN_STAT_ON,
  SCREEN_STAT_OFF,
} screen_status_t;

static screen_status_t screen_status;

/**
 * Mapping table for ISO/IEC 8859-1:1998 (aka latin1, IBM819, CP819), same as in Linux.
 * Used to translate ISO 8859-1 character codes to displayable characters in the VGA text mode
 * font.
 */
static const char *iso8859 = "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
                             "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
                             " !\"#$%&'()*+,-./0123456789:;<=>?"
                             "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_"
                             "`abcdefghijklmnopqrstuvwxyz{|}~\0"
                             "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
                             "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
                             "\377\255\233\234\376\235\174\025\376\376\246\256\252\055\376\376"
                             "\370\361\375\376\376\346\024\371\376\376\247\257\254\253\376\250"
                             "\376\376\376\376\216\217\222\200\376\220\376\376\376\376\376\376"
                             "\376\245\376\376\376\376\231\376\350\376\376\376\232\376\376\341"
                             "\205\240\203\376\204\206\221\207\212\202\210\211\215\241\214\213"
                             "\376\244\225\242\223\376\224\366\355\227\243\226\201\376\376\230";

// TODO: generic
static bool
vga_console_has_focus (vconsole_t *vc) {
  return vc->flags & CONSOLE_HAS_FOCUS;
}

/**
 * Turns the VGA screen back on if it was previously turned off.
 *
 * This function interacts with the VGA attribute controller to re-enable
 * screen output. It performs the necessary reads to reset the flip-flop
 * before writing to the attribute controller.
 */
static void
vga_screen_on (vconsole_t *vc) {
  unsigned int flags = eflags_get();
  int_disable();

  if (screen_status == SCREEN_STAT_OFF) {
    // Read from input status register 1 (clears the flip-flop)
    inb(VGA_INPUT_STAT1);
    // Read from mono status port (some hardware expects this to reset state)
    inb(VGA_MONO_STAT_PORT);
    // Write to attribute controller to re-enable the display
    outb(VGA_ATTR_CONTROLLER, VGA_ATTR_CONTROLLER_PAS);
  }

  eflags_set(flags);
}

/**
 * Turns the VGA screen off by blanking the display.
 *
 * This function disables (blanks) the VGA screen by writing to the
 * VGA attribute controller. It follows the standard procedure of
 * resetting the internal flip-flop before sending the blanking command.
 */
static void
vga_screen_off (vconsole_t *vc) {
  unsigned int flags = eflags_get();
  int_disable();

  screen_status = SCREEN_STAT_OFF;

  // Reset flip-flop by reading from VGA status registers
  inb(VGA_INPUT_STAT1);     // Typically 0x3DA (color) or 0x3BA (mono)
  inb(VGA_MONO_STAT_PORT);  // Possibly redundant, used for completeness
  // Write 0 to the attribute controller index port to disable (blank) display
  outb(VGA_ATTR_CONTROLLER, 0);

  eflags_set(flags);
}

/**
 * Writes a character to the virtual console at the current cursor position.
 *
 * This function writes a character `c` to the back buffer of the virtual console
 * and, if the console has focus, also writes it to the video memory and stores it
 * in the scrollback history buffer. The character is translated from ISO 8859 encoding
 * and combined with the current color attributes.
 *
 * @param vc Pointer to the virtual console structure.
 * @param c  Character to write.
 */
static void
vga_put_char (vconsole_t *vc, unsigned char c) {
  short int *main_buffer  = (short int *)vc->main_buffer;
  short int *back_buffer  = vc->back_buffer;

  c                       = iso8859[c];

  int next_index          = (vc->y * vc->columns) + vc->x;
  int next_value          = vc->color_attr | c;

  back_buffer[next_index] = next_value;

  if (vga_console_has_focus(vc)) {
    main_buffer[next_index]                                                 = next_value;
    vconsole_scrollback_history_buffer[(video.buf_y * vc->columns) + vc->x] = next_value;
  }
}

/**
 * Inserts a blank space at the current cursor position in the virtual console.
 *
 * This function shifts all characters to the right of the cursor by one position
 * to make space for a new character. It works on both the back buffer and, if the
 * console is currently active, the video memory. The last character in the line is lost.
 *
 * @param vc Pointer to the virtual console structure.
 */
static void
vga_insert_char (vconsole_t *vc) {
  short int *main_buffer = (short int *)vc->main_buffer;
  short int *back_buffer = vc->back_buffer;

  int n                  = vc->x;
  int offset             = (vc->y * vc->columns) + vc->x;

  short int tmp_char;
  short int last_char = CLEAR_MEM;

  // Shift characters to the right until the end of the line
  while (n++ < vc->columns) {
    if (vga_console_has_focus(vc)) {
      kmemcpy(&tmp_char, main_buffer + offset, 1);
      kmemset(main_buffer + offset, last_char, 1);
    }

    kmemcpy(&tmp_char, back_buffer + offset, 1);
    kmemset(back_buffer + offset, last_char, 1);

    last_char = tmp_char;
    offset++;
  }
}

/**
 * Deletes the character at the current cursor position in the virtual console.
 *
 * This function deletes a character at the cursor by shifting all characters to the right
 * of the cursor one position to the left. The last character in the line is cleared.
 * It applies the change to both the back buffer and, if the console has focus, the video memory.
 *
 * @param vc Pointer to the virtual console structure.
 */
static void
vga_delete_char (vconsole_t *vc) {
  short int *main_buffer = (short int *)vc->main_buffer;
  short int *back_buffer = vc->back_buffer;

  int offset             = (vc->y * vc->columns) + vc->x;
  int count              = vc->columns - vc->x;

  if (vga_console_has_focus(vc)) {
    kmemcpy(main_buffer + offset, main_buffer + offset + 1, count);
    kmemset(main_buffer + offset + count, CLEAR_MEM, 1);
  }

  kmemcpy(back_buffer + offset, back_buffer + offset + 1, count);
  kmemset(back_buffer + offset + count, CLEAR_MEM, 1);
}

/**
 * Controls the visibility of the hardware text-mode VGA cursor.
 *
 * This function enables, disables, or conditionally shows the text cursor based on the mode.
 * It modifies VGA hardware registers to control the cursor appearance.
 *
 * @param mode One of:
 *   - CURSOR_MODE_ON: Force cursor to be shown.
 *   - CURSOR_MODE_OFF: Force cursor to be hidden.
 *   - CURSOR_MODE_COND: Show cursor only if VPF_CURSOR_ON flag is set.
 */
static void
vga_show_cursor (vconsole_t *vc, int mode) {
  int status;

  switch (mode) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
    case CURSOR_MODE_COND:
      if (!(video.flags & VPF_CURSOR_ON)) {
        break;
      }
#pragma GCC diagnostic pop

    case CURSOR_MODE_ON:
      outb(video.port + CRT_INDEX, CRT_CURSOR_STR);
      status = inb(video.port + CRT_DATA);
      outb(video.port + CRT_DATA, status & CURSOR_MASK);
      video.flags |= VPF_CURSOR_ON;
      break;

    case CURSOR_MODE_OFF:
      outb(video.port + CRT_INDEX, CRT_CURSOR_STR);
      status = inb(video.port + CRT_DATA);
      outb(video.port + CRT_DATA, status | CURSOR_DISABLE);
      video.flags &= ~VPF_CURSOR_ON;
      break;
  }
}

/**
 * Reads the hardware VGA cursor position using I/O ports; combines the high and low bytes into a
 * single value, then translates that linear position into (x, y) screen coordinates based on the
 * console's width.
 */
static void
vga_get_cursor_pos (vconsole_t *vc) {
  unsigned short int pos;

  outb(video.port + CRT_INDEX, CRT_CURSOR_POS_HI);
  pos = inb(video.port + CRT_DATA) << 8;
  outb(video.port + CRT_INDEX, CRT_CURSOR_POS_LO);
  pos   |= inb(video.port + CRT_DATA);

  vc->x  = pos % vc->columns;
  vc->y  = pos / vc->columns;
}

/**
 * Updates the hardware VGA cursor to match the current cursor position in the given virtual console
 */
static void
vga_update_cursor_pos (vconsole_t *vc) {
  if (vga_console_has_focus(vc)) {
    // Calculate the linear position of the cursor in the VGA text buffer
    // Each row has `vc->columns` columns; the position is row * columns + column
    unsigned short int pos = (vc->y * vc->columns) + vc->x;
    // Select the high byte of the cursor position register in the VGA controller
    outb(video.port + CRT_INDEX, CRT_CURSOR_POS_HI);
    // Send the high byte (upper 8 bits) of the cursor position to the VGA controller
    outb(video.port + CRT_DATA, (pos >> 8) & 0xFF);
    // Select the low byte of the cursor position register in the VGA controller
    outb(video.port + CRT_INDEX, CRT_CURSOR_POS_LO);
    // Send the low byte (lower 8 bits) of the cursor position to the VGA controller
    outb(video.port + CRT_DATA, (pos & 0xFF));
  }
}

/**
 * Writes data with a specified color to a section of the screen.
 *
 * This function updates both the front and back buffers of the VGA screen for the specified range
 * of positions, effectively coloring a segment of the screen. If the console is focused, the front
 * buffer is updated as well.
 *
 * @param vc Pointer to the virtual console.
 * @param from The starting index of the range to be updated.
 * @param count The number of screen cells to update.
 * @param color The color (attribute) to fill the specified screen section.
 */
static void
vga_write_screen (vconsole_t *vc, int from, int count, short int color) {
  short int *main_buffer = (short int *)vc->main_buffer;
  short int *back_buffer = vc->back_buffer;

  kmemset(back_buffer + from, color, count);

  if (vga_console_has_focus(vc)) {
    kmemset(main_buffer + from, color, count);
  }
}

/**
 * Clears the screen of the virtual console.
 *
 * This function clears the entire screen by filling it with a 'clear memory' value,
 * and it updates the front buffer if the console has focus. It also marks the console
 * as cleared to prevent redundant clearing. Additionally, it hides the cursor after clearing.
 *
 * @param vc Pointer to the virtual console.
 */
static void
vga_clear_screen (vconsole_t *vc) {
  if (vc->flags & CONSOLE_CLEARED) {
    return;
  }

  short int *main_buffer = (short int *)vc->main_buffer;
  if (vga_console_has_focus(vc)) {
    kmemset(main_buffer, CLEAR_MEM, CONSOLE_SIZE);
  }

  vc->flags |= CONSOLE_CLEARED;
  vga_show_cursor(vc, CURSOR_MODE_OFF);
}

/**
 * Restores the screen from the back buffer to the front buffer.
 *
 * This function restores the screen content from the back buffer (which stores the last
 * known screen state) to the front buffer (the visible screen memory). It only updates the
 * front buffer if the console is currently focused.
 *
 * @param vc Pointer to the virtual console.
 */
static void
vga_restore_screen (vconsole_t *vc) {
  short int *main_buffer = (short int *)vc->main_buffer;

  if (vga_console_has_focus(vc)) {
    kmemcpy(main_buffer, vc->back_buffer, CONSOLE_SIZE);
  }
}

void
vga_init (void) {
  short int *bios_data = (short int *)(KERNEL_PAGE_OFFSET + BIOS_DATA_AREA);

  if ((*bios_data & BIOS_DATA_AREA_MONODISPLAY) == BIOS_DATA_AREA_MONODISPLAY) {
    video.address = (void *)(VGA_MONO_ADDR + KERNEL_PAGE_OFFSET);
    video.port    = VGA_MONO_PORT;
    kstrcpy((char *)video.signature, VGA_MONO_SIG);
  } else {
    video.address = (void *)(VGA_COLOR_ADDR + KERNEL_PAGE_OFFSET);
    video.port    = VGA_COLOR_PORT;
    kstrcpy((char *)video.signature, VGA_COLOR_SIG);
  }

  video.put_char          = vga_put_char;
  video.insert_char       = vga_insert_char;
  video.delete_char       = vga_delete_char;
  video.update_cursor_pos = vga_update_cursor_pos;
  video.show_cursor       = vga_show_cursor;
  video.get_cursor_pos    = vga_get_cursor_pos;
  video.write_screen      = vga_write_screen;
  video.clear_screen      = vga_clear_screen;
  // video.scroll_screen     = vga_scroll_screen;
  video.restore_screen    = vga_restore_screen;
  video.screen_on         = vga_screen_on;
  // video.buf_scroll        = vga_buf_scroll;
  // video.cursor_blink      = vga_cursor_blink;

  kmemcpy(vconsole_scrollback_history_buffer, video.address, CONSOLE_SIZE * 2);
}
