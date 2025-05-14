#include "proc/sched.h"

#include "arch/interrupt.h"
#include "mem/segments.h"
#include "proc/proc.h"

bool needs_resched = false;

/**
 * Performs a manual context switch, swapping out the current proc task state to that of `next`.
 */
static void
do_context_switch (proc_t* next) {
  INTERRUPTS_OFF();

  proc_t* prev = proc_current;
  sched_set_tss(next);
  proc_current = next;
  do_switch(&prev->tss.esp, &prev->tss.eip, next->tss.esp, next->tss.eip, next->tss.cr3, TSS);

  INTERRUPTS_ON();
}

void
sched_set_tss (proc_t* p) {
  //  Get GDT entry for TSS so we can modify it
  seg_desc_t* g = &gdt[TSS / sizeof(seg_desc_t)];
  // Set the low 24 bits of the base address of the TSS.
  g->low_base   = (unsigned int)&p->tss;
  // Set type and flags ; mark as TSS and present
  g->low_flags  = SD_TSS_PRESENT;
  // Write high 8 bits of base to complete the 32-bit base address
  g->hi_base    = (char)(((unsigned int)&p->tss) >> 24);
}

void
sched_run (void) {
  // Allow the current running process to consume its CPU time slice
  if (!needs_resched && proc_current_is_running()
      && proc_current_has_remaining_cpu_time_remaining()) {
    return;
  }
  needs_resched = false;

  proc_t* proc_to_run_next;
  while (true) {
    int count        = -1;
    // Fallback to idle proc if no other process is eligible for scheduling at this time
    proc_to_run_next = &proc_list[PROC_IDLE_PID];

    // Find the running process with the highest remaining CPU time slice
    proc_t* p_iter   = proc_running_list;
    while (p_iter) {
      if (p_iter->remaining_cpu_time > count) {
        count            = p_iter->remaining_cpu_time;
        proc_to_run_next = p_iter;
      }

      p_iter = p_iter->next_running;
    }

    if (count) {
      break;
    }

    // If no processes were eligible, recharge all of them by resetting their time slice to
    // `priority`
    p_iter = proc_running_list;
    while (p_iter) {
      p_iter->remaining_cpu_time = p_iter->priority;
      p_iter                     = p_iter->next_running;
    }
  }

  // If the current process isn't the selected one, switch to the new process.
  if (proc_current != proc_to_run_next) {
    do_context_switch(proc_to_run_next);
  }
}

void
sched_init (void) {
  // TODO:
}
