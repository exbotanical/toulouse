#ifndef SLEEP_H
#define SLEEP_H

#include "proc/proc.h"
// TODO: Use these consistently
#include "types.h"

int  sleep(void *addr, proc_state state);
void wakeup(void *addr);
void sleep_init(void);

#endif /* SLEEP_H */
