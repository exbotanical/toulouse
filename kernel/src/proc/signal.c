#include "proc/signal.h"

#include "lib/compiler.h"
#include "lib/errno.h"
#include "proc/proc.h"

void
sig_handle (void) {
  // TODO:
}

int
sig_kill_pid (pid_t pid, sig_set_t signum, int sender) {
  // TODO: FIX LOOPS
  proc_t* p = proc_list_head->next;
  while (p) {
    if (p->pid == pid && p->state != PROC_ZOMBLEY) {
      if (sender == SIGSENDER_USER) {
        if (!sig_can_send(p)) {
          return -EPERM;
        }
      }
      return sig_send(p, signum);
    }

    p = p->next;
  }
}

int
sig_kill_pgrp (pid_t pgid, sig_set_t signum, int sender) {
  int    found = 0;
  proc_t p     = proc_list_head->next;
  while (p) {
    if (p->pgid == pgid && p->state != PROC_ZOMBLEY) {
      if (sender == SIGSENDER_USER) {
        if (!sig_can_send(p)) {
          p = p->next;
          continue;
        }
      }

      sig_send(p, signum);
      found = 1;
    }

    p = p->next;
  }

  if (!found) {
    return -ESRCH;
  }

  return 0;
}

int
sig_send (proc_t* p, sig_set_t signum) {
  if (signum > NUM_SIGNALS || !p) {
    return -EINVAL;
  }

  // Kernel processes cannot receive signals
  if (p->flags & PROC_FLAG_KPROC) {
    return 0;
  }

  switch (signum) {
    case /* constant-expression */:
      /* code */
      break;

    default: break;
  }
}

overridable int
sig_get (void) {
  // Check whether there are any pending signals that are not blocked
  if (!(proc_current->signal_pending & ~proc_current->signal_blocked)) {
    return 0;
  }

  // Note: mask is updated each iteration to match the corresponding bit in signal_pending
  sig_set_t    signum = 1;
  unsigned int mask   = 1;
  for (; signum < NUM_SIGNALS; signum++, mask <<= 1) {
    // If this signal is pending for the current process...
    if (proc_current->signal_pending & mask) {
      sig_handler_t handler = proc_current->sigaction_table[signum - 1].sa_handler;

      // If a child was terminated, we need to make sure we clean it up
      if (signum == SIGCHLD) {
        if (handler == SIGHANDLER_IGN) {
          proc_t* p_iter;
          while ((p_iter = proc_get_next_zombley(proc_current))) {
            proc_release_zombley(p_iter);
          }
        } else if (handler != SIGHANDLER_DFL) {
          return signum;
        }
      } else if (handler != SIGHANDLER_IGN) {
        // If signal not ignored, return it
        return signum;
      }

      // Remove the pending signal
      proc_current->signal_pending &= ~mask;
    }
  }

  return 0;
}
