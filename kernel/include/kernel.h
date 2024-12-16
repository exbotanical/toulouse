#ifndef KERNEL_H
#define KERNEL_H

#include "common/compiler.h"

extern int k_param_memsize;
extern int k_param_extmemsize;

void noreturn kernel_stop(void);

#endif /* KERNEL_H */
