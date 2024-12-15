#ifndef INTERRUPT_SIGNAL_H
#define INTERRUPT_SIGNAL_H

#include "common/types.h"

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
} sigcontext_t;

bool sig_send(void);
bool sig_test(void);
void sig_handle(void);

#endif /* INTERRUPT_SIGNAL_H */
