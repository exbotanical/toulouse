# The BIOS loads us into 16-bit real mode for backward compatibility purposes (i.e. old 8086 processors).
# That is, in this mode memory addresses are calculated using 16-bit values;
# we can directly address up to 64kb of memory, starting from address 0x00000 to 0xFFFFF.
# Therefore, we instruct the assembler to generate 16-bit machine code here,
# affecting the size of registers, instructions, and memory addresses.
# Note, we can use the segment registers to reference addresses up to 1Mb.
.code16

stage1_start:
.include "boot/stage1/main.s"
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
# Note: 510 because of the 2 byte magic num at the end
.fill 510 - (. - stage1_start), 1, 0

# Magic boot sector word:
.word 0xaa55
stage1_end:

stage2_start:
.include "boot/stage2/main.s"
.align 512
stage2_end:

kernel_start:
.fill 512 - (. - kernel_start), 1, 0
kernel_end:
