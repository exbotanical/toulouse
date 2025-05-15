#include "proc/sleep.h"

#include "arch/interrupt.h"
#include "kconfig.h"
#include "lib/compiler.h"
#include "lib/string.h"
#include "proc/proc.h"
#include "proc/sched.h"

/**
 * The number of buckets in the sleep hash table.
 * Approx ~10% of processes
 */
#define NUM_SLEEP_HASH_TABLE_BUCKETS ((NUM_PROCS * 10) / 100)

/**
 * Computes a hash key for the sleep hash table
 */
#define TO_SLEEP_TABLE_HASH(addr)    ((addr) % (NUM_SLEEP_HASH_TABLE_BUCKETS))

/**
 * A hash table for storing sleeping processes. Keys are the address of a resource being slept on,
 * and values are a linked list of processes sleeping on that resource
 */
static proc_t *sleep_hash_table[NUM_SLEEP_HASH_TABLE_BUCKETS];

/**
 * Put the current process to sleep on addr
 */
overridable int
sleep (void *addr, proc_inttype state) {
  INTERRUPTS_OFF();

  int signum = 0;

  // If we're in an interruptible sleep, and there's a signal,
  // return the signal - we need to handle it
  if (state == PROC_INTERRUPTIBLE) {
    if ((signum = sig_get())) {
      goto done;
    }
  }

  if (proc_current->state == PROC_SLEEPING) {
    signum = 0;
    goto done;
  }

  int      key  = TO_SLEEP_TABLE_HASH((unsigned int)addr);
  proc_t **head = &sleep_hash_table[key];

  // Insert the current process as the head of the list
  if (!*head) {
    *head                  = proc_current;
    (*head)->prev_sleeping = (*head)->next_sleeping = NULL;
  } else {
    proc_current->prev_sleeping = NULL;
    proc_current->next_sleeping = *head;
    (*head)->prev_sleeping      = proc_current;
    *head                       = proc_current;
  }

  proc_current->sleep_addr = addr;

  if (state == PROC_UNINTERRUPTIBLE) {
    proc_current->flags |= PROC_FLAG_NOTINTERRUPT;
  }
  proc_not_runnable(proc_current, PROC_SLEEPING);

  sched_run();

  // Check again for signals
  signum = 0;
  if (state == PROC_INTERRUPTIBLE) {
    signum = sig_get();
  }

done:
  INTERRUPTS_ON();
  return signum;
}

/**
 * Wakeup any processes sleeping on addr
 */
overridable void
wakeup (void *addr) {
  INTERRUPTS_OFF();

  // Grab the bucket from the hash table
  int      key  = TO_SLEEP_TABLE_HASH((unsigned int)addr);
  proc_t **head = &sleep_hash_table[key];

  // Find the processes sleeping on addr
  while (*head) {
    // Once found...
    if ((*head)->sleep_addr == addr) {
      // Clear the sleep state, non-interruptible sleep flag, and restore the CPU quantum
      (*head)->sleep_addr          = NULL;
      (*head)->remaining_cpu_time  = (*head)->priority;
      (*head)->flags              &= ~PROC_FLAG_NOTINTERRUPT;

      // Make it runnable again and indicate we need to reschedule
      proc_runnable(*head);
      needs_resched = true;

      // Remove from the sleeping list
      if ((*head)->next_sleeping) {
        (*head)->next_sleeping->prev_sleeping = (*head)->prev_sleeping;
      }
      if ((*head)->prev_sleeping) {
        (*head)->prev_sleeping->next_sleeping = (*head)->next_sleeping;
      }

      // If it was the list head, we need to update the head pointer itself
      if (head == &sleep_hash_table[key]) {
        *head = (*head)->next_sleeping;
        continue;
      }
    }

    if (*head) {
      head = &(*head)->next_sleeping;
    }
  }

  INTERRUPTS_ON();
}

void
sleep_init (void) {
  proc_running_list = NULL;
  kmemset(sleep_hash_table, 0, sizeof(sleep_hash_table));
}
