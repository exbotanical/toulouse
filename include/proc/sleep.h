#ifndef SLEEP_H
#define SLEEP_H

#include "proc/proc.h"
// TODO: Use these consistently
#include "lib/types.h"

#define SLEEP_FN(fn) func_as_ptr((void (*)(void))fn)

static inline void *
func_as_ptr (void (*fn)(void)) {
  union {
    void *p;
    void (*f)(void);
  } u = {.f = fn};

  return u.p;
}

/**
 * Puts the current process to sleep on addr
 */
int sleep(void *addr, proc_inttype state);

/**
 * Wakes up any processes sleeping on addr
 */
void wakeup(void *addr);

/**
 * Wakes up a specific process p
 */
void wakeup_proc(proc_t *p);

void sleep_init(void);

#endif /* SLEEP_H */
