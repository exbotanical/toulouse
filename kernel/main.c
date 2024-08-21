#include <stdint.h>

#include "kernel.h"

void
kernel_start (void) {
  uint16_t* vga = (uint16_t*)(0xB80000);

  vga[0]        = 0x0341;
}
