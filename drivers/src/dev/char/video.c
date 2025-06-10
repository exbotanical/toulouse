#include "drivers/dev/char/video.h"

#include "drivers/dev/char/framebuf.h"
#include "drivers/dev/char/vga.h"
#include "lib/string.h"

video_props_t video;

void
video_init (void) {
  kmemset(
    vconsole_scrollback_history_buffer,
    0,
    video.columns + video.rows * MAX_SCROLLBACK_SCREENS * 2 * sizeof(short int)
  );

  if (video_using_vga()) {
    vga_init();
  } else if (video_using_vesa_framebuffer()) {
    framebuf_init();
  }
}
