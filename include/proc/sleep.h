#ifndef SLEEP_H
#define SLEEP_H

#include "proc/proc.h"
// TODO: Use these consistently
#include "lib/types.h"

int  sleep(void *addr, proc_inttype state);
void wakeup(void *addr);
void sleep_init(void);

#endif /* SLEEP_H */
