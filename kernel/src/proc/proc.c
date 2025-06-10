#include "proc/proc.h"

#include "arch/interrupt.h"
#include "lib/compiler.h"
#include "lib/errno.h"
#include "lib/string.h"
#include "mem/alloc.h"
#include "mem/base.h"
#include "proc/lock.h"
#include "proc/sleep.h"

proc_t *proc_current;

proc_t *proc_list;
proc_t *proc_list_head;
proc_t *proc_list_tail;

proc_t *proc_running_list;

/**
 * Head pointer into linked list of free processes
 */
static proc_t *proc_free_list;

/**
 * Number of processes in the free list
 */
static unsigned int proc_free_list_size = 0;

static resource_t lock                  = {.locked = 0, .wanted = 0};

bool
proc_is_orphaned_pgrp (pid_t pgid) {
  lock_resource(&lock);

  proc_t *p = proc_list_head->next;
  while (p) {
    if (p->pgid == pgid) {
      if (p->state != PROC_ZOMBLEY) {
        proc_t *pp = p->parent;
        if (pp->pgid != pgid || pp->sid == p->sid) {
          return false;
        }
      }
    }

    p = p->next;
  }

  unlock_resource(&lock);
  return true;
}

proc_t *
proc_get_next_zombley (proc_t *parent) {
  proc_t *p_iter = proc_list_head->next;
  while (p_iter) {
    if (p_iter->parent == parent && p_iter->state == PROC_ZOMBLEY) {
      return p_iter;
    }
    p_iter = p_iter->next;
  }

  return NULL;
}

pid_t
proc_release_zombley (proc_t *p) {
  pid_t pid = p->pid;
  // Free the kernel-mode stack allocated for this process
  kfree(p->tss.esp0);
  // One less page
  p->rss--;
  // Free the page directory allocated to this process
  kfree(P2V(p->tss.cr3));
  // Another less page
  p->rss--;

  proc_t *parent = p->parent;
  proc_release(p);

  if (parent) {
    parent->children--;
  }

  return pid;
}

void
proc_release (proc_t *p) {
  lock_resource(&lock);

  if (p == proc_list_tail) {
    if (proc_list_head == proc_list_tail) {
      proc_list_head = proc_list_tail = NULL;
    } else {
      proc_list_tail       = proc_list_tail->prev;
      proc_list_tail->next = NULL;
    }
  } else {
    p->prev->next = p->next;
    p->next->prev = p->prev;
  }

  // TODO: Fix root Makefile test targets et al
  kmemset(p, 0, sizeof(proc_t));
  p->next        = proc_free_list;
  proc_free_list = p;
  proc_free_list_size++;

  unlock_resource(&lock);
}

overridable void
proc_runnable (proc_t *p) {
  if (p->state == PROC_RUNNING) {
    // TODO: log - process with pid already running
    return;
  }

  INTERRUPTS_OFF();

  // TODO: separate fn
  if (proc_running_list) {
    p->next_running                 = proc_running_list;
    proc_running_list->prev_running = p;
  }
  proc_running_list = p;

  p->state          = PROC_RUNNING;

  INTERRUPTS_ON();
}

overridable void
proc_not_runnable (proc_t *p, proc_state state) {
  INTERRUPTS_OFF();

  if (p->next_running) {
    p->next_running->prev_running = p->prev_running;
  }
  if (p->prev_running) {
    p->prev_running->next_running = p->next_running;
  }
  if (p == proc_running_list) {
    proc_running_list = p->next_running;
  }
  p->prev_running = p->next_running = NULL;
  p->state                          = state;

  INTERRUPTS_ON();
}

void
proc_init (void) {
  kmemset(proc_list, 0, proc_list_size);

  // Fill up the free list by reverse-populating it
  proc_free_list = NULL;

  unsigned int n = (proc_list_size / sizeof(proc_t)) - 1;
  do {
    proc_t *p      = &proc_list[n];
    p->next        = proc_free_list;
    proc_free_list = p;
    proc_free_list_size++;
  } while (n--);

  proc_list_head = proc_list_tail = NULL;
}
