#include "drivers/dev/char/framebuf.h"

#include "drivers/dev/device.h"
#include "lib/types.h"

static device_t framebuf_device = {
  .name              = "fb",
  .major             = 29U,
  .minors            = {0, 0, 0, 0, 0, 0, 0, 0},
  .minor_block_sizes = 0,
  .device_data       = NULL,
};

void
framebuf_init (void) {
  framebuf_device;
}
