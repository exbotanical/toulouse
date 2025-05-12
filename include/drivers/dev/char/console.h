#ifndef DRIVER_DEV_CHAR_CONSOLE_H
#define DRIVER_DEV_CHAR_CONSOLE_H

#include "lib/types.h"

#define NUM_CONSOLES      12
#define CONSOLES_MAJOR    4

#define CONSOLE_HAS_FOCUS 0x0001
#define CONSOLE_CLEARED   0x0002

#define CONSOLE_COLS      video.columns
#define CONSOLE_LINES     video.lines
#define CONSOLE_SIZE      (video.columns * video.lines)

#define COLOR_BLACK       0x0000
#define COLOR_BLUE        0x0100
#define COLOR_GREEN       0x0200
#define COLOR_CYAN        0x0300
#define COLOR_RED         0x0400
#define COLOR_MAGENTA     0x0500
#define COLOR_BROWN       0x0600
#define COLOR_WHITE       0x0700
#define BG_BLACK          0x0000
#define BG_BLUE           0x1000
#define BG_GREEN          0x2000
#define BG_CYAN           0x3000
#define BG_RED            0x4000
#define BG_MAGENTA        0x5000
#define BG_BROWN          0x6000
#define BG_WHITE          0x7000

#define DEFAULT_MODE      (COLOR_WHITE | BG_BLACK)
#define CLEAR_MEM         (DEFAULT_MODE | ' ')

#define CURSOR_MODE_OFF   0
#define CURSOR_MODE_ON    1
#define CURSOR_MODE_COND  2

/**
 * Bitmask for the lower 5 bits of the register, which control the start scanline
 * (vertical position) of the cursor inside a character cell.
 */
#define CURSOR_MASK       0x1F

/**
 * This is the 6th bit (bit 5).
 * When set, it tells the VGA hardware to disable the cursor entirely.
 */
#define CURSOR_DISABLE    0x20

typedef struct {
  /**
   * Current column
   */
  int x;
  /**
   * Current row
   */
  int y;

  int top;
  int rows;
  int columns;

  unsigned char *main_buffer;
  short int     *back_buffer;

  int flags;

  unsigned short int color_attr;

  bool screen_locked;
} vconsole_t;

void console_init(void);

#endif /* DRIVER_DEV_CHAR_CONSOLE_H */
