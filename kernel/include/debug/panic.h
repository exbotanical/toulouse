#ifndef PANIC_H
#define PANIC_H

#include "arch/x86.h"
#include "common/compiler.h"
#include "common/types.h"

extern volatile bool is_in_panic;

static inline void noreturn
die (void) {
  while (true) {
    halt();
  }
}

void noreturn k_panic(char* msg);

#endif /* PANIC_H */
