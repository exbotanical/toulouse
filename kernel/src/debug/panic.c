#include "debug/panic.h"

#include "arch/interrupt.h"
#include "debug/stacktrace.h"
#include "drivers/console/vga.h"

volatile bool is_in_panic = false;

static void
dispatch_proc_interrupts (void) {
  // TODO:
}

void noreturn
k_panic (char* msg) {
  int_disable();

  if (is_in_panic) {
    die();
  }

  access_once(is_in_panic) = true;
  barrier();

  dispatch_proc_interrupts();

  vga_console_writestr("\nKERNEL PANIC: ");
  vga_console_writestr(msg);
  vga_console_writestr("\n");

  dump_stack_trace();

  die();
}
