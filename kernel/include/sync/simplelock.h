#ifndef SIMPLELOCK_H
#define SIMPLELOCK_H

#include "arch/eflags.h"
#include "arch/interrupt.h"

static unsigned int lock = 0;

static int
simplelock_lock (unsigned int type) {
  unsigned int flags = eflags_get();
  int_disable();

  int ret  = lock & type;
  lock    |= type;

  eflags_set(flags);

  return ret;
}

static int
simplelock_unlock (unsigned int type) {
  unsigned int flags = eflags_get();
  int_disable();

  int ret  = lock & type;
  lock    &= ~type;

  eflags_set(flags);

  return ret;
}

#endif /* SIMPLELOCK_H */
