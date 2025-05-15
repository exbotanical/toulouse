#include "arch/interrupt.h"

#include "lib/compiler.h"

overridable void
int_disable (void) {
  asm volatile("cli");
}

overridable void
int_enable (void) {
  asm volatile("sti");
}

overridable bool
int_enabled (void) {
  return eflags_get() & EFLAGS_INT_ENABLED;
}
