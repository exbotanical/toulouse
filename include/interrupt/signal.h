#ifndef INTERRUPT_SIGNAL_H
#define INTERRUPT_SIGNAL_H

#include "lib/types.h"

#define NUM_SIGNALS    32

/* We define here all the signal names listed in POSIX (1003.1-2008);
   as of 1003.1-2013, no additional signals have been added by POSIX.
   We also define here signal names that historically exist in every
   real-world POSIX variant (e.g. SIGWINCH).

   Signals in the 1-15 range are defined with their historical numbers.
   For other signals, we use the BSD numbers.
   There are two unallocated signal numbers in the 1-31 range: 7 and 29.
   Signal number 0 is reserved for use as kill(pid, 0), to test whether
   a process exists without sending it a signal.  */

/* ISO C99 signals.  */
#define SIGINT         2  /* Interactive attention signal.  */
#define SIGILL         4  /* Illegal instruction.  */
#define SIGABRT        6  /* Abnormal termination.  */
#define SIGFPE         8  /* Erroneous arithmetic operation.  */
#define SIGSEGV        11 /* Invalid access to storage.  */
#define SIGTERM        15 /* Termination request.  */

/* Historical signals specified by POSIX. */
#define SIGHUP         1  /* Hangup.  */
#define SIGQUIT        3  /* Quit.  */
#define SIGTRAP        5  /* Trace/breakpoint trap.  */
#define SIGKILL        9  /* Killed.  */
#define SIGPIPE        13 /* Broken pipe.  */
#define SIGALRM        14 /* Alarm clock.  */

/* Archaic names for compatibility.  */
#define SIGIO          SIGPOLL /* I/O now possible (4.2 BSD).  */
#define SIGIOT         SIGABRT /* IOT instruction, abort() on a PDP-11.  */
#define SIGCLD         SIGCHLD /* Old System V name */

/* Other */
#define SIGSTKFLT      16 /* Stack fault (obsolete).  */
#define SIGPWR         30 /* Power failure imminent.  */

/* Historical signals specified by POSIX. */
#define SIGBUS         7  /* Bus error.  */
#define SIGSYS         31 /* Bad system call.  */

/* New(er) POSIX signals (1003.1-2008, 1003.1-2013).  */
#define SIGURG         23 /* Urgent data is available at a socket.  */
#define SIGSTOP        19 /* Stop, unblockable.  */
#define SIGTSTP        20 /* Keyboard stop.  */
#define SIGCONT        18 /* Continue.  */
#define SIGCHLD        17 /* Child terminated or stopped.  */
#define SIGTTIN        21 /* Background read from control terminal.  */
#define SIGTTOU        22 /* Background write to control terminal.  */
#define SIGPOLL        29 /* Pollable event occurred (System V).  */
#define SIGXFSZ        25 /* File size limit exceeded.  */
#define SIGXCPU        24 /* CPU time limit exceeded.  */
#define SIGVTALRM      26 /* Virtual timer expired.  */
#define SIGPROF        27 /* Profiling timer expired.  */
#define SIGUSR1        10 /* User-defined signal 1.  */
#define SIGUSR2        12 /* User-defined signal 2.  */

/* Nonstandard signals found in all modern POSIX systems
   (including both BSD and Linux).  */
#define SIGWINCH       28 /* Window size change (4.3 BSD, Sun).  */

/* Archaic names for compatibility.  */
#define SIGIO          SIGPOLL /* I/O now possible (4.2 BSD).  */
#define SIGIOT         SIGABRT /* IOT instruction, abort() on a PDP-11.  */
#define SIGCLD         SIGCHLD /* Old System V name */

/**
 * Default behavior - there is no custom, user-defined handler
 */
#define SIGHANDLER_DFL ((sig_handler_t)0)

/**
 * Ignore the signal
 */
#define SIGHANDLER_IGN ((sig_handler_t)1)

typedef enum {
  SIGSENDER_KERNEL,
  SIGSENDER_USER,
} signal_sender;

// See: https://www.gnu.org/software/libc/manual/html_node/Signal-Sets.html
typedef unsigned int sig_set_t;

typedef void (*sig_handler_t)(int);

typedef struct {
  unsigned int gs;
  unsigned int fs;
  unsigned int es;
  unsigned int ds;
  unsigned int edi;
  unsigned int esi;
  unsigned int ebp;
  unsigned int esp;
  int          ebx;
  int          edx;
  int          ecx;
  int          eax;
  int          err;
  unsigned int eip;
  unsigned int cs;
  unsigned int eflags;
  unsigned int og_esp;
  unsigned int og_ss;
} sig_context_t;

typedef struct {
  sig_handler_t sa_handler;
  sig_set_t     sa_mask;
  int           sa_flags;
  void (*sa_restorer)(void);
} sig_action_t;

void sig_handle(void);

/**
 * Checks whether the current process has any pending, unblocked signals, and if
 * so, returns the first one that should be delivered
 */
int sig_get(void);

#endif /* INTERRUPT_SIGNAL_H */
