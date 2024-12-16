#ifndef ARCH_CPU_H
#define ARCH_CPU_H

#include "arch/x86.h"
#include "common/compiler.h"
#include "common/types.h"

static inline noreturn void
cpu_idle (void) {
  while (true) {
    halt();
  }
}

#endif /* ARCH_CPU_H */
