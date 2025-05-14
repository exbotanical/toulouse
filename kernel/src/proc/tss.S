.align 4
# Performs a manual task switch.
# Saves the current task's context (esp, eip), switches to another task's context (its esp, eip, cr3), and resumes it.
# The original task is paused, and will resume later at label 1:, continuing right after the do_switch() call.
.global do_switch; do_switch:
  pusha                      # Push all general-purpose registers onto the stack to preserve CPU state
  pushfl                     # Push EFLAGS to preserve flags

  # Save state so we can return to this exact point later when switching back
  movl   %esp, %ebx          # Grab top of stack
  addl   $0x24, %ebx         # Skip over the stuff we just pushed and go to the arguments on the stack
  movl   0x4(%ebx), %eax     # Save the stack pointer to prev->tss.esp
  movl   %esp, (%eax)

  # Persist the previous task so it continues right after the switch at $1f
  movl   0x8(%ebx), %eax     # Save the instruction pointer to prev->tss.eip
  movl   $1f, (%eax)         # Point the IP at $1f

  # Setup the stack of the new task
  movl   0xC(%ebx), %esp     # Load next->tss.esp into the stack pointer
  pushl  0x10(%ebx)          # Push the saved instruction pointer of the next task onto its stack
                             # so the ret at the end will jump to this instruction and continue execution in the new task

  # Load the new task's page directory (in cr3, for paging)
  movl   0x14(%ebx), %eax    # Load next->tss.cr3 into %cr3
  ltr    0x18(%ebx)          # Load Task Register (TSS selector)
  movl   %eax, %cr3          # Switch to next task's page table

  # Pops the return address from the stack (which we just pushed as next->eip) and jumps to it.
  # This starts executing the new process exactly where it left off.
  ret
  # Finish the switch-in process by restoring everything we saved at the start
1:
  popfl
  popa
  ret
