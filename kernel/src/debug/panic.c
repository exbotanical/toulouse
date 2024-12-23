#include "debug/panic.h"

#include "arch/interrupt.h"
#include "debug/stacktrace.h"
#include "kernel.h"

volatile bool is_in_panic = false;

static void
dispatch_proc_interrupts (void) {
  // TODO:
}

void noreturn
__kpanic (void) {
  if (access_once(is_in_panic)) {
    kernel_stop();
  }

  access_once(is_in_panic) = true;
  barrier();

  dump_stack_trace();

  kernel_stop();
}
