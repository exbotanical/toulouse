#include "drivers/dev/char/video.h"

#include "drivers/dev/char/framebuf.h"
#include "drivers/dev/char/vga.h"
#include "lib/string.h"

video_props_t video;

short int* video_scrollback_history_buffer;

bool
video_using_vga (void) {
  return video.flags & VPF_VGA;
}

bool
video_using_vesa_framebuffer (void) {
  return video.flags & VPF_VESAFB;
}

void
video_init (void) {
  kmemset(
    video_scrollback_history_buffer,
    0,
    video.columns + video.lines * VIDEO_MAX_SCROLLBACK_SCREENS * 2 * sizeof(short int)
  );

  if (video_using_vga()) {
    vga_init();
  } else if (video_using_vesa_framebuffer()) {
    framebuf_init();
  }
}
