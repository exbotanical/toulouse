#ifndef PROCESS_H
#define PROCESS_H

#include "interrupt/signal.h"

#define IO_BITMAP_SIZE 8192 /* 8192*8bit = all I/O address space */

/* Intel 386 Task Switch State */
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

typedef struct process process_t;

struct process {
  i386_tss_t         tss;
  process_t         *ppid; /* pointer to parent process */
  int                pid;  /* process ID */
  int                pgid; /* process group ID */
  int                sid;  /* session ID */
  int                flags;
  // int                groups[NGROUPS_MAX];
  int                children; /* number of children */
  // struct tty        *ctty;      /* controlling terminal */
  int                state;     /* process state */
  int                priority;
  int                cpu_count; /* time of process running */
  unsigned int       start_time;
  int                exit_code;
  void              *sleep_address;
  unsigned short int uid;  /* real user ID */
  unsigned short int gid;  /* real group ID */
  unsigned short int euid; /* effective user ID */
  unsigned short int egid; /* effective group ID */
  unsigned short int suid; /* saved user ID */
  unsigned short int sgid; /* saved group ID */
  // unsigned short int fd[OPEN_MAX];
  // unsigned char      fd_flags[OPEN_MAX];
  // struct inode      *root;
  // struct inode      *pwd; /* process working directory */
  unsigned int       entry_address;
  // char               argv0[NAME_MAX + 1];
  int                argc;
  char             **argv;
  int                envc;
  char             **envp;
  char               pidstr[5]; /* PID number converted to string */
  // struct vma        *vma_table; /* virtual memory-map addresses */
  unsigned int       brk_lower; /* lower limit of the heap section */
  unsigned int       brk;       /* current limit of the heap */
  unsigned int       sigpending;
  unsigned int       sigblocked;
  unsigned int       sigexecuting;
  sig_action_t       sigaction[NUM_SIGNALS];
  sig_context_t      sc[NUM_SIGNALS]; /* each signal has its own context */
  unsigned int       sp;              /* current process' stack frame */
  // struct rusage      usage;    /* process resource usage */
  // struct rusage      cusage;   /* children resource usage */
  unsigned int       it_real_interval;
  unsigned int       it_real_value;
  unsigned int       it_virt_interval;
  unsigned int       it_virt_value;
  unsigned int       it_prof_interval;
  unsigned int       it_prof_value;
  unsigned int       timeout;
  // struct rlimit      rlim[RLIM_NLIMITS];
  unsigned int       rss;
  unsigned short int umask;
  unsigned char      loopcnt; /* nested symlinks counter */

  process_t *prev;
  process_t *next;
  process_t *prev_sleep;
  process_t *next_sleep;
  process_t *prev_run;
  process_t *next_run;
};

extern process_t *proc_current;
extern process_t *proc_table;

extern unsigned int proc_table_size;

void proc_init(void);

#endif /* PROCESS_H */
