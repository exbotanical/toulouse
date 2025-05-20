#ifndef SLEEP_H
#define SLEEP_H

#include "proc/proc.h"
// TODO: Use these consistently
#include "lib/types.h"

static inline void *
func_as_ptr (void (*fn)(void)) {
  union {
    void *p;
    void (*f)(void);
  } u = {.f = fn};

  return u.p;
}

/**
 * Put the current process to sleep on addr
 */
int sleep(void *addr, proc_inttype state);

/**
 * Wakeup any processes sleeping on addr
 */
void wakeup(void *addr);
void sleep_init(void);

#endif /* SLEEP_H */
