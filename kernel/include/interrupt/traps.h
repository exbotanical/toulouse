#ifndef INTERRUPT_TRAPS_H
#define INTERRUPT_TRAPS_H

#include "interrupt/signal.h"

#define NUM_EXCEPTIONS 32

typedef struct {
  char* name;
  void (*handler)(unsigned int, sigcontext_t*);
  char errcode;
} trap_t;

void trap_handle(unsigned int, sigcontext_t);

void trap_divide_error(unsigned int, sigcontext_t*);
void trap_debug(unsigned int, sigcontext_t*);
void trap_nmi_interrupt(unsigned int, sigcontext_t*);
void trap_breakpoint(unsigned int, sigcontext_t*);
void trap_overflow(unsigned int, sigcontext_t*);
void trap_bound(unsigned int, sigcontext_t*);
void trap_invalid_opcode(unsigned int, sigcontext_t*);
void trap_no_math_coprocessor(unsigned int, sigcontext_t*);
void trap_double_fault(unsigned int, sigcontext_t*);
void trap_coprocessor_segment_overrun(unsigned int, sigcontext_t*);
void trap_invalid_tss(unsigned int, sigcontext_t*);
void trap_segment_not_present(unsigned int, sigcontext_t*);
void trap_stack_segment_fault(unsigned int, sigcontext_t*);
void trap_general_protection(unsigned int, sigcontext_t*);
void trap_page_fault(unsigned int, sigcontext_t*);
void trap_reserved(unsigned int, sigcontext_t*);
void trap_floating_point_error(unsigned int, sigcontext_t*);
void trap_alignment_check(unsigned int, sigcontext_t*);
void trap_machine_check(unsigned int, sigcontext_t*);
void trap_simd_fault(unsigned int, sigcontext_t*);

#endif /* INTERRUPT_TRAPS_H */
