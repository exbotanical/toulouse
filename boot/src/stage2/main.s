.code16
mov $INIT_REAL_MODE_STAGE2_MSG, %si
call print_ln_16

# We now need to (try to) enable the A20 line.
# We'll check if it's enabled and try other methods if needed once in 32-bit protected mode.
mov $ATTEMPT_A20_VIA_BIOS_MSG, %si
call print_ln_16
call try_enable_a20_via_bios

# Get the memory map from the BIOS
mov $READ_MMAP_MSG, %si
call print_ln_16
call mmap_init

# Disable interrupts
# We do this because once we switch to 32-bit protected mode, the IVT that the BIOS
# has established at the beginning of memory will be moot. If the CPU somehow did map
# an interrupt to a BIOS routine at this point, the CPU would crash.
cli
# Load the GDT
lgdt gdt_descriptor

# Set the first bit of the %cr0 control register
mov %cr0, %eax
or $0x1, %eax
mov %eax, %cr0

# Far jump (i.e. into a new segment) to the 32-bit section.
# By doing a far jump, we force a CPU cache flush and thereby
# get rid of any pre-fetched 16-bit instructions.
jmp $CODE_SEG, $load_pm

# Note: Once we jump into 32-bit protected mode, we have 4GB of addressable memory.
# We'll also no longer have access to the BIOS, which means in order to read from disk,
# we must write our own disk driver...and we'll need to read from disk because the BIOS only
# loads the 512b MBR.
.code32
load_pm:
  # Update the segment registers and point them at the GDT data selector
  mov $DATA_SEG, %ax
  mov %ax, %ds
  mov %ax, %ss
  mov %ax, %es
  mov %ax, %fs
  mov %ax, %gs

  # Move the stack sufficiently far away from the bootloader.
  mov $0x9000, %ebp
  mov %ebp, %esp

  mov $INIT_PROT_MODE_MSG, %ebx
  call print_string_32

  call is_a20_enabled
  cmp $1, %eax
  je a20_enabled

  # TODO: test
  # TODO: improve w/retries, extra docs (+ bit checks etc)
  mov $A20_NOT_ENABLED_BIOS_MSG, %ebx
  call print_ln_32

  call enable_a20_via_keyctrl
  call is_a20_enabled
  cmp $1, %eax
  je a20_enabled

  mov $A20_NOT_ENABLED_KEYCTRL_MSG, %ebx
  call print_ln_32

  call enable_a20_via_fast
  call is_a20_enabled
  cmp $1, %eax
  je a20_enabled

  mov $A20_NOT_ENABLED_FAST_MSG, %ebx
  call print_ln_32
  hlt

a20_enabled:
  mov $A20_ENABLED_MSG, %ebx
  call print_ln_32

  call load_kernel

  jmp .

INIT_REAL_MODE_STAGE2_MSG: .asciz "Stage 2 bootloader loaded in 16-bit real mode"
ATTEMPT_A20_VIA_BIOS_MSG: .asciz "Attempting to enable the A20 line via the BIOS"
READ_MMAP_MSG: .asciz "Reading in memory map from BIOS"
INIT_PROT_MODE_MSG: .asciz "Stage 2 bootloader loaded in 32-bit protected mode"
A20_NOT_ENABLED_BIOS_MSG: .asciz "The A20 line could not be enabled via the BIOS. Attempting enablement via keyboard controller..."
A20_NOT_ENABLED_KEYCTRL_MSG: .asciz "The A20 line could not be enabled via the keyboard controller. Attempting enablement via fast A20..."
A20_NOT_ENABLED_FAST_MSG: .asciz "The A20 line could not be enabled via the Fast A20. Non-recoverable exception. Halting..."
A20_ENABLED_MSG: .asciz "The A20 line has been enabled."

.include "src/common/utils-16.s"
.include "src/stage2/mmap.s"
.include "src/common/utils-32.s"
.include "src/stage2/a20.s"
.include "src/stage2/gdt.s"
