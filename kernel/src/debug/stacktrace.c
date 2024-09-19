#include "debug/stacktrace.h"

#include "common/types.h"
#include "drivers/console/vga.h"
#include "lib/string.h"

static inline uint32_t
get_address (uint32_t vaddr, uint32_t off) {
  return ((uint32_t *)vaddr)[off];
}

void
dump_stack_trace (void) {
  vga_console_writestr("Stack trace:\n");

  uint32_t last_ebp = 0, ebp, eip;
  asm("mov %%ebp, %0" : "=r"(ebp));

  eip = get_address(ebp, 1);

  for (uint32_t frame = 0; eip && ebp && ebp > last_ebp && frame < MAX_STACK_FRAMES; frame++) {
    char s[64];
    k_itoa(eip - 1, s, 16);

    vga_console_writestr("    0x");
    vga_console_writestr(s);
    vga_console_writestr("\n");

    last_ebp = ebp;
    ebp      = get_address(ebp, 0);
    eip      = get_address(ebp, 1);
  }

  vga_console_writestr("Stack trace end\n");
}
