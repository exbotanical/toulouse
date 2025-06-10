#ifndef DRIVER_DEV_CHAR_VIDEO_H
#define DRIVER_DEV_CHAR_VIDEO_H

#include "drivers/dev/char/console/console.h"
#include "lib/types.h"

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
  int           rows;
  int           columns;
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
  int size;
  /**
   * Size of screen based on columns x rows
   */
  int vsize;

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

/**
 * This buffer is used only in the active vconsole. Every time a vconsole is switched, the screen
 * contents of the new vconsole are copied back to this buffer.
 * Only the visible screen is copied; switching vconsoles means losing the scrollback history.
 */
extern short int *vconsole_scrollback_history_buffer;

static inline bool
video_using_vga (void) {
  return video.flags & VIDEO_MODE_VGA_TXT;
}

static inline bool
video_using_vesa_framebuffer (void) {
  return video.flags & VIDEO_MODE_VESAFB;
}

void video_init(void);

#endif /* DRIVER_DEV_CHAR_VIDEO_H */
