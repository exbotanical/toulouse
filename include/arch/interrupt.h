#ifndef ARCH_INTERRUPT_H
#define ARCH_INTERRUPT_H

#include "arch/eflags.h"
#include "lib/types.h"

#define INTERRUPTS_OFF()             \
  unsigned int flags = eflags_get(); \
  int_disable()

#define INTERRUPTS_ON() eflags_set(flags);

/**
 * Disables interrupts.
 */
void int_disable(void);

/**
 * Enables interrupts.
 */
void int_enable(void);

/**
 * Are interrupts enabled?
 */
bool int_enabled(void);

#endif /* ARCH_INTERRUPT_H */
