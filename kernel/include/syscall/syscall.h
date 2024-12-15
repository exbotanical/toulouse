#ifndef SYSCALL_H
#define SYSCALL_H

#include "interrupt/signal.h"

extern void syscall(void);
int         syscall_exec(
          unsigned int  num,
          int           arg_1,
          int           arg_2,
          int           arg_3,
          int           arg_4,
          int           arg_5,
          sigcontext_t* sc
        );

#endif /* SYSCALL_H */
