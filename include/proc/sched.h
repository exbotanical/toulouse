#ifndef SCHED_H
#define SCHED_H

#include "lib/types.h"
#include "proc/proc.h"

extern bool needs_resched;

/**
 * Performs a TSS task switch
 */
extern void do_switch(
  unsigned int      *prev_esp_ptr,
  unsigned int      *prev_eip_ptr,
  unsigned int       idle_esp,
  unsigned int       idle_eip,
  unsigned int       idle_cr3,
  unsigned short int tss
);

/**
 * Installs a new TSS descriptor into the GDT that points to `p->tss`,
 * so when the CPU loads this GDT entry (via `ltr`), it knows where to find the TSS for this
 * process.
 */
void sched_set_tss(proc_t *p);

/**
 * Decides which process should run next via Round Robin scheduling
 */
void sched_run(void);

/**
 * Initialize scheduler resources
 */
void sched_init(void);

#endif /* SCHED_H */
