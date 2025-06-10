#ifndef INTERRUPT_TRAPS_H
#define INTERRUPT_TRAPS_H

#include "proc/signal.h"

#define NUM_EXCEPTIONS 32

typedef struct {
  char* name;
  void (*handler)(unsigned int trap_num, sig_context_t* sc);
  char errcode;
} trap_t;

void trap_handle(unsigned int trap_num, sig_context_t sc);

void trap_divide_error(unsigned int trap_num, sig_context_t* sc);
void trap_debug(unsigned int trap_num, sig_context_t* sc);
void trap_nmi_interrupt(unsigned int trap_num, sig_context_t* sc);
void trap_breakpoint(unsigned int trap_num, sig_context_t* sc);
void trap_overflow(unsigned int trap_num, sig_context_t* sc);
void trap_bound(unsigned int trap_num, sig_context_t* sc);
void trap_invalid_opcode(unsigned int trap_num, sig_context_t* sc);
void trap_no_math_coprocessor(unsigned int trap_num, sig_context_t* sc);
void trap_double_fault(unsigned int trap_num, sig_context_t* sc);
void trap_coprocessor_segment_overrun(unsigned int trap_num, sig_context_t* sc);
void trap_invalid_tss(unsigned int trap_num, sig_context_t* sc);
void trap_segment_not_present(unsigned int trap_num, sig_context_t* sc);
void trap_stack_segment_fault(unsigned int trap_num, sig_context_t* sc);
void trap_general_protection(unsigned int trap_num, sig_context_t* sc);
void trap_page_fault(unsigned int trap_num, sig_context_t* sc);
void trap_reserved(unsigned int trap_num, sig_context_t* sc);
void trap_floating_point_error(unsigned int trap_num, sig_context_t* sc);
void trap_alignment_check(unsigned int trap_num, sig_context_t* sc);
void trap_machine_check(unsigned int trap_num, sig_context_t* sc);
void trap_simd_fault(unsigned int trap_num, sig_context_t* sc);

#endif /* INTERRUPT_TRAPS_H */
