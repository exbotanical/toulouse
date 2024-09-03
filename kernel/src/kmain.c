#include "kmain.h"

#include "drivers/console/vga.h"

void
kmain (void) {
  uint16_t* vga = (uint16_t*)0xC03FF000;
  vga[0]        = 'X' | (0x01 | (0x04 << 4)) << 8;

  // vga_globl_console_init();
  // vga_console_writestr(global_vga_con, "HELLO MOO");
}
