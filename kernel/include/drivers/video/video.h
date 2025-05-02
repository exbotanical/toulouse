#ifndef VIDEO_DEV_H
#define VIDEO_DEV_H

#include "drivers/video/console.h"

/**
 * VGA text mode
 */
#define VIDEO_MODE_VGA_TXT           0x01
/**
 * x86 framebuffer
 */
#define VIDEO_MODE_VESAFB            0x02

/**
 * Maximum number of screens worth of scrollback buffer
 * that we keep at any given moment in time
 */
#define VIDEO_MAX_SCROLLBACK_SCREENS 6

#define VIDEO_CONS_DEFAULT_COLS      80
#define VIDEO_CONS_DEFAULT_ROWS      25

/**
 * Video device properties. Many of these are typically derived from multiboot info passed by the
 * bootloader. Otherwise, the kernel may need to make BIOS calls (for things like whether VGA is
 * supported), and the BIOS may not be available anymore.
 */
typedef struct {
  // pci_device_t *pci_dev;
  int           flags;
  unsigned int *address;
  int           port;
  int           memsize;
  unsigned char signature[32];
  int           lines;
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

  void (*put_char)(vconsole_t *, unsigned char);
  void (*insert_char)(vconsole_t *);
  void (*delete_char)(vconsole_t *);
  void (*update_cursor_pos)(vconsole_t *);
  void (*show_cursor)(vconsole_t *, int);
  void (*get_cursor_pos)(vconsole_t *);
  void (*write_screen)(vconsole_t *, int, int, short int);
  void (*clear_screen)(vconsole_t *);
  void (*scroll_screen)(vconsole_t *, int, int);
  void (*restore_screen)(vconsole_t *);
  void (*screen_on)(vconsole_t *);
  void (*buf_scroll)(vconsole_t *, int);
  void (*cursor_blink)(unsigned int);
} video_props_t;

extern video_props_t video;
extern short int    *video_scrollback_history_buffer;

void video_init(void);

#endif /* VIDEO_DEV_H */
