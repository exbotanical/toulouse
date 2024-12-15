#include "init/multiboot.h"

#include "drivers/console/vga.h"
#include "lib/string.h"

void
multiboot_init (unsigned int magic, unsigned int mbi_ptr) {
  // multiboot_info_t* mbi = (multiboot_info_t*)mbi_ptr;

  if (magic != MULTIBOOT_BOOTLOADER_MAGIC) {
    vgaprintf("WARNING: invalid multiboot magic number: 0x%x. Assuming 4MB of RAM.\n", magic);
  }
}
