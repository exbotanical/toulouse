#ifndef BREAKPOINT_H
#define BREAKPOINT_H

#ifdef DEBUG

static inline void
pseudo_breakpoint () {
  asm volatile("1: jmp 1b");
}

#endif

#endif /* BREAKPOINT_H */
