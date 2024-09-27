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

  vga_printf("\nKERNEL PANIC: %s\n", msg);

  dump_stack_trace();

  die();
}
