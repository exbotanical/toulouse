.code16
load_protected:
  # Disable interrupts
  # We do this because once we switch to 32-bit protected mode, the IVT that the BIOS
  # has established at the beginning of memory will be moot. If the CPU somehow did map
  # an interrupt to a BIOS routine at this point, the CPU would crash.
  cli
  # Load the GDT
  lgdt (gdt_descriptor)
  # Set the first bit of the %cr0 control register
  mov %cr0, %eax
  or $0x01, %eax
  mov %eax, %cr0

  # Far jump (i.e. into a new segment) to the 32-bit section.
  # By doing a far jump, we force a CPU cache flush and thereby
  # get rid of any pre-fetched 16-bit instructions.
  jmp *CODE_SEG + init_prot_mode

.code32
init_prot_mode:
  # Update the segment registers and point them at the GDT data selector
  mov $DATA_SEG, %ax
  mov %ax, %ds
  mov %ax, %ss
  mov %ax, %es
  mov %ax, %fs
  mov %ax, %gs

  # Point the stack at the top of the free memory
  mov $0x90000, %ebp
  mov %ebp, %esp
  call begin_prot_mode
