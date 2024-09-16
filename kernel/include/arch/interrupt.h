#ifndef INTERRUPT_H
#define INTERRUPT_H

#include "arch/eflags.h"
#include "common/types.h"

/**
 * Disables interrupts.
 */
static inline void
int_disable (void) {
  asm volatile("cli");
}

/**
 * Enables interrupts.
 */
static inline void
int_enable (void) {
  asm volatile("sti");
}

static bool
int_enabled (void) {
  return eflags_get() & EFLAGS_INT_ENABLED;
}

static void
int_save (uint32_t *flags) {
  *flags = int_enabled();
  int_disable();
}

static void
int_store (uint32_t flags) {
  if (flags & EFLAGS_INT_ENABLED) {
    int_enable();
  } else {
    int_disable();
  }
}

#endif /* INTERRUPT_H */
