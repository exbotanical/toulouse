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

static inline uint32_t
eflags_get (void) {
  unsigned int eflags;
  asm volatile("pushfl\n\t"
               "popl %0"
               : "=r"(eflags)
               :
               : "memory");
  return eflags;
}

static inline void
eflags_set (uint32_t eflags) {
  asm volatile("pushl %0; popfl\n\t" : : "r"(eflags) : "memory");
}

#endif /* ARCH_EFLAGS_H */
