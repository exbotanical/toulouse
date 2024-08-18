# The BIOS loads us into 16-bit real mode for backward compatibility purposes (i.e. old 8086 processors).
# That is, in this mode memory addresses are calculated using 16-bit values;
# we can directly address up to 64kb of memory, starting from address 0x00000 to 0xFFFFF.
# Therefore, we instruct the assembler to generate 16-bit machine code here,
# affecting the size of registers, instructions, and memory addresses.

# Note, we can use the segment registers to reference addresses up to 1Mb.
.code16

# This is our entry-point at 0x7C00
.global _start

# Must be the first code; all addresses are relative to this offset 0x7C00
_start:
  # Move the stack sufficiently far away from 0x7C00 to avoid overwriting it
  # when we read from disk.
  mov $0x9000, %bp
  mov %bp, %sp

  mov $INIT_REAL_MODE_MSG, %si
  call print_string
  call print_newline
  call load_protected

.include "boot/gdt.s"
.include "boot/utils-16.s"
.include "boot/utils-32.s"
.include "boot/pm.s"

.code32
begin_prot_mode:
  mov $INIT_PROT_MODE_MSG, %ebx
  call print_string_32
  hlt

INIT_REAL_MODE_MSG: .asciz "Bootloader loaded in 16-bit real mode"
INIT_PROT_MODE_MSG: .asciz "Bootloader loaded in 32-bit protected mode"

# When the machine boots, the BIOS doesn't know how to load the OS;
# it delegates this responsibility to the boot sector.
# The boot sector is expected to be 512 bytes found in the first sector of the disk.
# The BIOS checks that bytes 511 and 512 of the ostensible boot sector equal 0xAA55
# in order to verify it is indeed bootable.

# Fill the remainder of the output so it totals 512 bytes:
# . is the current location counter
# _start is the start of the current section
# 2nd arg: 1 (byte) is the size of the fill
# 3rd arg: 0 is the value to use in the fill
.fill 510 - (. - _start), 1, 0

# Magic boot sector word:
.word 0xaa55
