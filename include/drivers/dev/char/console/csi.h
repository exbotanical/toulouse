#ifndef DRIVER_DEV_CHAR_CONSOLE_CSI_H
#define DRIVER_DEV_CHAR_CONSOLE_CSI_H

#include "drivers/dev/char/video.h"

#define CSI_J_CUR2END   0 /* Clear from cursor to end of screen */
#define CSI_J_START2CUR 1 /* Clear from start of screen to cursor */
#define CSI_J_SCREEN    2 /* Clear entire screen */

#define CSI_K_CUR2END   0 /* Clear from cursor to end of line */
#define CSI_K_START2CUR 1 /* Clear from start of line to cursor */
#define CSI_K_LINE      2 /* Clear entire line */

static void
csi_J (vconsole_t* vc, int mode) {
  int from;
  int count;

  switch (mode) {
    // Erase Down <ESC>[J
    case CSI_J_CUR2END: {
      from  = (vc->y * vc->columns) + vc->x;
      count = vc->columns - vc->x;
      video.write_screen(vc, from, count, vc->color_attr);
      from  = ((vc->y + 1) * vc->columns);
      count = CONSOLE_SIZE - from;
      break;
    }

    // Erase Up <ESC>[1J
    case CSI_J_START2CUR: {
      from  = vc->y * vc->columns;
      count = vc->x + 1;
      video.write_screen(vc, from, count, vc->color_attr);
      from  = 0;
      count = vc->y * vc->columns;
      break;
    }

    // Erase Screen <ESC>[2J
    case CSI_J_SCREEN: {
      from  = 0;
      count = CONSOLE_SIZE;
      break;
    }

    default: {
      return;
    }
  }

  video.write_screen(vc, from, count, vc->color_attr);
}

static void
csi_K (vconsole_t* vc, int mode) {
  int from;
  int count;

  switch (mode) {
    // Erase End of Line <ESC>[K
    case CSI_K_CUR2END: {
      from  = (vc->y * vc->columns) + vc->x;
      count = vc->columns - vc->x;
      break;
    }

    // Erase Start of Line <ESC>[1K
    case CSI_K_START2CUR: {
      from  = vc->y * vc->columns;
      count = vc->x + 1;
      break;
    }

    // Erase Line <ESC>[2K
    case CSI_K_LINE: {
      from  = vc->y * vc->columns;
      count = vc->columns;
      break;
    }

    default: {
      return;
    }
  }

  video.write_screen(vc, from, count, vc->color_attr);
}

static void
csi_X (vconsole_t* vc, int count) {
  int from = (vc->y * vc->columns) + vc->x;

  count    = count > (vc->columns - vc->x) ? vc->columns - vc->x : count;
  video.write_screen(vc, from, count, vc->color_attr);
}

static void
csi_L (vconsole_t* vc, int count) {
  if (count > (vc->rows - vc->top)) {
    count = vc->rows - vc->top;
  }

  while (count--) {
    video.scroll_screen(vc, vc->y, CONS_SCROLL_DOWN);
  }
}

static void
csi_M (vconsole_t* vc, int count) {
  if (count > (vc->rows - vc->top)) {
    count = vc->rows - vc->top;
  }

  while (count--) {
    video.scroll_screen(vc, vc->y, CONS_SCROLL_UP);
  }
}

static void
csi_P (vconsole_t* vc, int count) {
  if (count > vc->columns) {
    count = vc->columns;
  }

  while (count--) {
    video.delete_char(vc);
  }
}

static void
csi_at (vconsole_t* vc, int count) {
  if (count > vc->columns) {
    count = vc->columns;
  }

  while (count--) {
    video.insert_char(vc);
  }
}

#endif /* DRIVER_DEV_CHAR_CONSOLE_CSI_H */
