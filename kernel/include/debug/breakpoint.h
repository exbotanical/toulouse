#ifndef DEBUG_BREAKPOINT_H
#define DEBUG_BREAKPOINT_H

#ifdef DEBUG

/**
 * Create a pseudo-breakpoint for debugging. Not at risk of compiler mangling.
 */
static inline void
pseudo_breakpoint () {
  asm volatile("1: jmp 1b");
}

#endif

#endif /* DEBUG_BREAKPOINT_H */
