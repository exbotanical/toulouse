#define ASM_SOURCE 1

#include "mem/segments.h"

#define CHECK_NEEDS_SCHEDULE                                    \
  jmp    2f                                                    ;\

// Check if the stack has changed i.e. a nested interrupt has occurred
#define CHECK_NESTED_INT                                        \
  cmpw   $(KERNEL_CS), 0x38(%esp)                              ;\
  je     2f                                                    ;\

#define CHECK_SIGNALS                                          ;\
  call   sig_get                                              ;\
  testl  $0xFFFFFFFF, %eax                                     ;\
  jz     1f                                                    ;\
  movl   %esp, %eax                                            ;\
  pushl  %eax                                                  ;\
  call   sig_handle                                            ;\
  addl   $4, %esp                                              ;\

#define PERSIST_SEGMENTS                                        \
  pusha                                                        ;\
  pushl  %ds                                                   ;\
  pushl  %es                                                   ;\
  pushl  %fs                                                   ;\
  pushl  %gs                                                   ;\

#define RESTORE_SEGMENTS                                        \
  popl   %gs                                                   ;\
  popl   %fs                                                   ;\
  popl   %es                                                   ;\
  popl   %ds                                                   ;\
  popa                                                         ;\
  addl   $4, %esp                                              ;\

#define EXEC_IRQ_HANDLER(irq)                                   \
  pushl  $irq                                                  ;\
  call   irq_handler                                           ;\
  addl   $4, %esp                                              ;\

#define EXEC_IRQ_BOTTOM_HALF                                    \
  sti                                                          ;\
  call   irq_bottom_half_exec                                  ;\

#define EXEC_EXCEPTION_HANDLER(exception)                       \
  pushl  $exception                                            ;\
  call   trap_handle                                           ;\
  addl   $4, %esp                                              ;\

#define BUILD_IRQ(irq_num, name)                                \
.align 4                                                       ;\
.global name; name:                                            ;\
  pushl  $0                                                    ;\
  PERSIST_SEGMENTS                                             ;\
  EXEC_IRQ_HANDLER(irq_num)                                    ;\
  EXEC_IRQ_BOTTOM_HALF                                         ;\
  CHECK_NESTED_INT                                             ;\
  CHECK_SIGNALS                                                ;\
1:                                                             ;\
  CHECK_NEEDS_SCHEDULE                                         ;\
2:                                                             ;\
  RESTORE_SEGMENTS                                             ;\

#define BUILD_EXCEPTION(ex_num, name)                           \
.align 4                                                       ;\
.global name; name:                                            ;\
  PERSIST_SEGMENTS                                             ;\
  EXEC_EXCEPTION_HANDLER(ex_num)                               ;\
  EXEC_IRQ_BOTTOM_HALF                                         ;\
  CHECK_NESTED_INT                                             ;\
  CHECK_SIGNALS                                                ;\
1:                                                             ;\
  CHECK_NEEDS_SCHEDULE                                         ;\
2:                                                             ;\
  RESTORE_SEGMENTS                                             ;\
  iret                                                         ;\

// Not all interrupts have an error code - for those we just push zero.
#define BUILD_EXCEPTION_DEFAULT_ERRCODE(ex_num, name)           \
.align 4                                                       ;\
.global name; name:                                            ;\
  pushl  $0                                                    ;\
  PERSIST_SEGMENTS                                             ;\
  EXEC_EXCEPTION_HANDLER(ex_num)                               ;\
  EXEC_IRQ_BOTTOM_HALF                                         ;\
  CHECK_NESTED_INT                                             ;\
  CHECK_SIGNALS                                                ;\
1:                                                             ;\
  CHECK_NEEDS_SCHEDULE                                         ;\
2:                                                             ;\
  RESTORE_SEGMENTS                                             ;\
  iret                                                         ;\

.align 4
.global syscall; syscall:
  // Persist the syscall number
  pushl  %eax
  PERSIST_SEGMENTS
  // arg 5
	pushl	 %edi
	pushl	 %esi
	pushl	 %edx
	pushl	 %ecx
  // ..arg 1
	pushl	 %ebx
  // syscall number
	pushl	 %eax
	call	 syscall_exec
	addl	 $24, %esp
  // Save the retval
	movl	 %eax, 0x2C(%esp)

	EXEC_IRQ_BOTTOM_HALF
	CHECK_SIGNALS
	CHECK_NEEDS_SCHEDULE

.global syscall_ret; syscall_ret:
	RESTORE_SEGMENTS
	iret

BUILD_IRQ(0, irq_0)
BUILD_IRQ(1, irq_1)
BUILD_IRQ(2, irq_2)
BUILD_IRQ(3, irq_3)
BUILD_IRQ(4, irq_4)
BUILD_IRQ(5, irq_5)
BUILD_IRQ(6, irq_6)
BUILD_IRQ(7, irq_7)
BUILD_IRQ(8, irq_8)
BUILD_IRQ(9, irq_9)
BUILD_IRQ(10, irq_10)
BUILD_IRQ(11, irq_11)
BUILD_IRQ(12, irq_12)
BUILD_IRQ(13, irq_13)
BUILD_IRQ(14, irq_14)
BUILD_IRQ(15, irq_15)

.align 4
.global irq_unknown; irq_unknown:
  pushl   $0
  PERSIST_SEGMENTS
  call   irq_unknown_handler
  RESTORE_SEGMENTS
  iret

BUILD_EXCEPTION_DEFAULT_ERRCODE(0, exception_0)
BUILD_EXCEPTION_DEFAULT_ERRCODE(1, exception_1)
BUILD_EXCEPTION_DEFAULT_ERRCODE(2, exception_2)
BUILD_EXCEPTION_DEFAULT_ERRCODE(3, exception_3)
BUILD_EXCEPTION_DEFAULT_ERRCODE(4, exception_4)
BUILD_EXCEPTION_DEFAULT_ERRCODE(5, exception_5)
BUILD_EXCEPTION_DEFAULT_ERRCODE(6, exception_6)

.align 4
.global exception_7; exception_7:
  pushl  $0
  PERSIST_SEGMENTS
  EXEC_EXCEPTION_HANDLER(0x7)
  clts
  EXEC_IRQ_BOTTOM_HALF
  CHECK_NESTED_INT
  CHECK_SIGNALS
1:
  CHECK_NEEDS_SCHEDULE
2:
  RESTORE_SEGMENTS
  iret

BUILD_EXCEPTION(8, exception_8)
BUILD_EXCEPTION_DEFAULT_ERRCODE(9, exception_9)
BUILD_EXCEPTION(10, exception_10)
BUILD_EXCEPTION(11, exception_11)
BUILD_EXCEPTION(12, exception_12)
BUILD_EXCEPTION(13, exception_13)
BUILD_EXCEPTION(14, exception_14)
BUILD_EXCEPTION_DEFAULT_ERRCODE(15, exception_15)
BUILD_EXCEPTION_DEFAULT_ERRCODE(16, exception_16)
BUILD_EXCEPTION(17, exception_17)
BUILD_EXCEPTION_DEFAULT_ERRCODE(18, exception_18)
BUILD_EXCEPTION_DEFAULT_ERRCODE(19, exception_19)
BUILD_EXCEPTION_DEFAULT_ERRCODE(20, exception_20)
BUILD_EXCEPTION_DEFAULT_ERRCODE(21, exception_21)
BUILD_EXCEPTION_DEFAULT_ERRCODE(22, exception_22)
BUILD_EXCEPTION_DEFAULT_ERRCODE(23, exception_23)
BUILD_EXCEPTION_DEFAULT_ERRCODE(24, exception_24)
BUILD_EXCEPTION_DEFAULT_ERRCODE(25, exception_25)
BUILD_EXCEPTION_DEFAULT_ERRCODE(26, exception_26)
BUILD_EXCEPTION_DEFAULT_ERRCODE(27, exception_27)
BUILD_EXCEPTION_DEFAULT_ERRCODE(28, exception_28)
BUILD_EXCEPTION_DEFAULT_ERRCODE(29, exception_29)
BUILD_EXCEPTION_DEFAULT_ERRCODE(30, exception_30)
BUILD_EXCEPTION_DEFAULT_ERRCODE(31, exception_31)
