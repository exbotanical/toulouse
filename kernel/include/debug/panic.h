#ifndef PANIC_H
#define PANIC_H

#include <stdarg.h>

#include "arch/cpu.h"
#include "arch/x86.h"
#include "common/compiler.h"
#include "common/types.h"
#include "drivers/console/vga.h"

extern volatile bool is_in_panic;

static inline void noreturn
die (void) {
  cpu_idle();
}

void noreturn __kpanic(void);

// TODO: dyn buffer
#define kpanic(fmt, ...)                          \
  kprintf("[PANIC] Exception in %s\n", __func__); \
  kprintf(fmt, __VA_ARGS__);                      \
  __kpanic();

#endif /* PANIC_H */
