#ifndef KERNEL_H
#define KERNEL_H

#include "fs/elf.h"
#include "kstat.h"
#include "lib/compiler.h"

/**
 * A global pointer to kernel stats and state.
 */
extern kstat_t kstat;

/**
 * A global store for the last static kernel page boundary.
 */
extern unsigned int real_last_addr;

extern elf32_shdr *symtab;
extern elf32_shdr *strtab;

/**
 * Stop the kernel, release any dynamic kernel resources, and idle the CPU.
 */
void noreturn kernel_stop(void);

#endif /* KERNEL_H */
