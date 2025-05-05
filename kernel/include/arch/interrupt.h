#ifndef ARCH_INTERRUPT_H
#define ARCH_INTERRUPT_H

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

/**
 * Are interrupts enabled?
 */
static bool
int_enabled (void) {
  return eflags_get() & EFLAGS_INT_ENABLED;
}

#endif /* ARCH_INTERRUPT_H */
