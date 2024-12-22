#ifndef KERNEL_H
#define KERNEL_H

#include "common/compiler.h"
#include "kstat.h"

/**
 * A global pointer to kernel stats and state.
 */
extern k_stat kstat;

/**
 * A global store for the last static kernel page boundary.
 */
extern unsigned int real_last_addr;

/**
 * Stop the kernel, release any dynamic kernel resources, and idle the CPU.
 */
void noreturn kernel_stop(void);

#endif /* KERNEL_H */
