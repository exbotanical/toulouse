#ifndef ARCH_EFLAGS_H
#define ARCH_EFLAGS_H

#include "lib/types.h"

/**
 * Interrupt enabled flag.
 */
#define EFLAGS_INT_ENABLED (1 << 9)

/**
 * The only flags that are modifiable by user processes.
 */
#define EFLAGS_USER_MASK   0x8CFF

uint32_t eflags_get(void);

void eflags_set(uint32_t eflags);

#endif /* ARCH_EFLAGS_H */
