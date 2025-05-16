#include "interrupt/signal.h"

#include "lib/compiler.h"
#include "proc/proc.h"

void
sig_handle (void) {
  // TODO:
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
