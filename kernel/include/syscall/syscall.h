#ifndef SYSCALL_H
#define SYSCALL_H

#include "interrupt/signal.h"

/**
 * Sets up and invokes a syscall.
 */
extern void syscall(void);

/**
 * Actually executes the syscall.
 *
 * @param num The syscall number
 * @param arg_1
 * @param arg_2
 * @param arg_3
 * @param arg_4
 * @param arg_5
 * @param sc
 * @return int
 */
int syscall_exec(
  unsigned int   num,
  int            arg_1,
  int            arg_2,
  int            arg_3,
  int            arg_4,
  int            arg_5,
  sig_context_t* sc
);

#endif /* SYSCALL_H */
