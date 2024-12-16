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

void noreturn __k_panic(void);

// TODO: dyn buffer
#define k_panic(fmt, ...)                           \
  vgaprintf("[PANIC] Exception in %s\n", __func__); \
  vgaprintf(fmt, __VA_ARGS__);                      \
  __k_panic();

#endif /* PANIC_H */
