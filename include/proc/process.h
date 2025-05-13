#ifndef PROC_PROCESS_H
#define PROC_PROCESS_H

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

typedef struct proc proc_t;

struct proc {};

extern proc_t *proc_current;
extern proc_t *proc_table;

extern unsigned int proc_table_size;

void proc_init(void);

#endif /* PROC_PROCESS_H */
