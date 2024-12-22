#ifndef DEV_DRIVER_CONSOLE_H
#define DEV_DRIVER_CONSOLE_H

#include "drivers/dev/pci.h"

/**
 * VGA text mode
 */
#define VIDEO_MODE_VGA_TXT      0x01
/**
 * x86 framebuffer
 */
#define VIDEO_MODE_VESAFB       0x02
#define VIDEO_CONS_DEFAULT_COLS 80
#define VIDEO_CONS_DEFAULT_ROWS 25

typedef struct {
  pci_device_t *pci_dev;
  int           flags;
  unsigned int *address;
  int           port;
  int           memsize;
  unsigned char signature[32];
  int           columns;
  int           rows;
  int           buf_y;
  int           buf_top;
  int           version;
  int           width;
  int           height;
  int           char_width;
  int           char_height;
  int           bpp;
  int           pixelwidth;
  int           pitch;
  int           rowsize;
  /**
   * Size of screen based on resolution
   */
  int           size;
  /**
   * Size of screen based on columns x rows
   */
  int           vsize;
} video_props_t;

extern video_props_t video;

#endif /* DEV_DRIVER_CONSOLE_H */
