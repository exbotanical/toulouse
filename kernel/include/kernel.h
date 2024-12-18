#ifndef KERNEL_H
#define KERNEL_H

#include "common/compiler.h"
#include "kstat.h"

extern k_stat       kstat;
extern unsigned int real_last_addr;

void noreturn kernel_stop(void);

#endif /* KERNEL_H */
