#ifndef EFLAGS_H
#define EFLAGS_H

#include "common/types.h"

// Interrupt enabled flag.
#define EFLAGS_INT_ENABLED (1 << 9)

// The only flags that are modifiable by user processes.
#define EFLAGS_USER_MASK   0x8CFF

static inline uint32_t
eflags_get (void) {
  unsigned int eflags;
  __asm__ __volatile__("pushf\n\t"
                       "pop %0"
                       : "=r"(eflags));
}

static inline void
eflags_set (uint32_t eflags) {
  __asm__ __volatile__("pop %%ecx\n\t"
                       "popf\n\t"
                       // Restore the stack frame
                       "sub $4, %%esp\n\t"
                       // Return to the caller's stack frame
                       "jmp *%%ecx"
                       :
                       : "g"(eflags)
                       : "ecx", "memory");
}

#endif /* EFLAGS_H */
