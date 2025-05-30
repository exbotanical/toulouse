#ifndef PROC_PROCESS_H
#define PROC_PROCESS_H

#include "interrupt/signal.h"
#include "lib/types.h"

#define IO_BITMAP_SIZE         8192 /* 8192*8bit = all I/O address space */

/**
 * PID of the idle process
 */
#define PROC_IDLE_PID          0

/**
 * PID of the init process i.e. /sbin/init
 */
#define PROC_INIT_PID          1

/**
 * Flag indicating this process is an internal kernel process
 */
#define PROC_FLAG_KPROC        0x00000001

/**
 * Flag indicating this process has performed a sys_execve
 */
#define PROC_FLAG_PEXEC        0x00000002

/**
 * Flag indicating this process should use the real UID in permissions checks
 */
#define PROC_FLAG_USEREAL      0x00000004

/**
 * Flag indicating this process is in a non-interruptible sleeping state
 */
#define PROC_FLAG_NOTINTERRUPT 0x00000008

/**
 * Process id type
 */
typedef uint32_t pid_t;

typedef enum {
  /**
   * The process is in a running state
   */
  PROC_RUNNING = 1,

  /**
   * The process is in a sleeping state
   */
  PROC_SLEEPING,

  /**
   * The process is a zombie
   *
   * "Holy fuck! Jacob is a Zombley!"
   * If you know, you know.
   */
  PROC_ZOMBLEY,

  /**
   * The process is stopped
   */
  PROC_STOPPED,

  /**
   * The process is idle
   */
  PROC_IDLE,
} proc_state;

typedef enum {
  PROC_INTERRUPTIBLE = 1,
  PROC_UNINTERRUPTIBLE,
} proc_inttype;

/**
 * Represents the Intel 386 Task Switch State (TSS)
 */
typedef struct {
  unsigned int       prev_tss;
  unsigned int       esp0;
  unsigned int       ss0;
  unsigned int       esp1;
  unsigned int       ss1;
  unsigned int       esp2;
  unsigned int       ss2;
  unsigned int       cr3;
  unsigned int       eip;
  unsigned int       eflags;
  unsigned int       eax;
  unsigned int       ecx;
  unsigned int       edx;
  unsigned int       ebx;
  unsigned int       esp;
  unsigned int       ebp;
  unsigned int       esi;
  unsigned int       edi;
  unsigned int       es;
  unsigned int       cs;
  unsigned int       ss;
  unsigned int       ds;
  unsigned int       fs;
  unsigned int       gs;
  unsigned int       ldt;
  unsigned short int debug_trap;
  unsigned short int io_bitmap_addr;
  unsigned char      io_bitmap[IO_BITMAP_SIZE + 1];
} i386_tss_t;

typedef struct proc proc_t;

struct proc {
  /**
   * Pointer to the parent process
   */
  proc_t *parent;

  /**
   * The number of children that belong to this process
   */
  int children;

  pid_t      pid;
  proc_state state;
  i386_tss_t tss;

  /**
   * Resident Set Size
   *
   * See: https://en.wikipedia.org/wiki/Resident_set_size
   */
  unsigned int rss;

  /**
   * Remaining CPU time slice
   */
  int remaining_cpu_time;

  /**
   * Priority of the process
   * Represents the default amount of CPU time the process should receive per scheduling round
   */
  int priority;

  /**
   * Address of the resource, if any, that this process is sleeping on.
   * Used for lookups in sleep proc hash table
   */
  void *sleep_addr;

  /**
   * Bitmask of signals sent to this process but not yet handled
   */
  sig_set_t signal_pending;
  /**
   * Bitmask of signals currently blocked by the process
   */
  sig_set_t signal_blocked;
  sig_set_t signal_executing;

  /**
   * A table of signal handlers for various signals
   */
  sig_action_t sigaction_table[NUM_SIGNALS];

  int flags;

  proc_t *prev;
  proc_t *next;
  proc_t *prev_sleeping;
  proc_t *next_sleeping;
  proc_t *prev_running;
  proc_t *next_running;

  tty_t *controlling_tty;
};

/**
 * A pointer to the currently running process that is actively switched to
 */
extern proc_t *proc_current;

/**
 * A linked list of all processes
 */
extern proc_t *proc_list;

/**
 * The size of the proc list - TBD at runtime, and set in memory initialization
 */
extern unsigned int proc_list_size;

/**
 * A linked list of currently running processes
 */
extern proc_t *proc_running_list;

/**
 * Returns a bool indicating whether the current process is in a running state
 */
static inline bool
proc_current_is_running (void) {
  return proc_current->state == PROC_RUNNING;
}

/**
 * Returns a bool indicating whether the current process has any remaining CPU time
 */
static inline bool
proc_current_has_remaining_cpu_time_remaining (void) {
  return proc_current->remaining_cpu_time > 0;
}

/**
 * Given a parent process, returns its first child process whose status is ZOMBLEEEEYY.
 */
proc_t *proc_get_next_zombley(proc_t *parent);

/**
 * Releases a given zombley process and frees its allocated resources
 */
pid_t proc_release_zombley(proc_t *p);

/**
 * Release a process and put it back into the free list
 */
void proc_release(proc_t *p);

/**
 * Set the process
 */
void proc_runnable(proc_t *p);

void proc_not_runnable(proc_t *p, proc_state state);

/**
 * Initialize process tables
 */
void proc_init(void);

#endif /* PROC_PROCESSsleep_address_H */
