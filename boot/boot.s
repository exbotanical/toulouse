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
  mov $INIT_MSG, %si
  call print_string
  jmp _start


# The BIOS loads our bootloader from the first sector of the disk; it loads nothing more.
# Presumably, our OS is larger than 512b therefore we must read it into memory from disk.

# dh = num_sectors_to_load
disk_load:
  push %dx

  # BIOS ISR for reading sector(s) from disk
  # https://www.ctyme.com/intr/rb-0607.htm
  mov $0x02, %ah
  # Read %dh sectors
  mov %dh, %al
  # Start reading from the second sector (i.e. the one after the 512b bootloader)
  mov $2, %cl
  # Select cylinder 0
  mov $0, %ch
  # Select head 0
  mov $0, %dh
  int $0x13

  # The BIOS will set the carry flag if there was a fault
  jc disk_error

  # Restore %dx - the high bits is the original num sectors arg
  pop %dx
  # Check the number of sectors actually read and compare to expected (%dh)
  cmp %al, %dh
  jne disk_error

  ret

disk_error:
  mov $DISK_ERR_MSG, %si
  call print_string
  jmp .

.include "boot/utils.s"

INIT_MSG: .asciz "Bootloader loaded"
DISK_ERR_MSG: .asciz "Disk read error"
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
