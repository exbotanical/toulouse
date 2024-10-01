#include "kmain.h"

#include "drivers/console/vga.h"

void
kmain (unsigned int magic, unsigned int mbi) {
  vga_global_console_init();

  vgaprintf("MAGICK: 0x%X\n", magic);
}
