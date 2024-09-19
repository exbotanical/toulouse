#include "kmain.h"

#include "debug/panic.h"
#include "drivers/console/vga.h"
#include "mem/manager.h"

void
kmain (multiboot_info_t *mbi) {
  vga_globl_console_init();
  memm_init(mbi);
}
