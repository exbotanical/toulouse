#include "kmain.h"

#include "debug/panic.h"
#include "drivers/console/vga.h"
#include "lib/string.h"
#include "mem/manager.h"

void
kmain (multiboot_info_t *mbi) {
  vga_globl_console_init();
  mm_init(mbi);
}
