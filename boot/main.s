# The BIOS loads us into 16-bit real mode for backward compatibility purposes (i.e. old 8086 processors).
# That is, in this mode memory addresses are calculated using 16-bit values;
# we can directly address up to 64kb of memory, starting from address 0x00000 to 0xFFFFF.
# Therefore, we instruct the assembler to generate 16-bit machine code here,
# affecting the size of registers, instructions, and memory addresses.
# Note, we can use the segment registers to reference addresses up to 1Mb.
.code16

# This is our entry-point at 0x7C00
.global _start

# https://wiki.osdev.org/FAT#BPB_(BIOS_Parameter_Block)
# Prevent BIOS from overwriting the next ~30 bytes of our code on some devices e.g. USB
# In short, the BIOS *may* fill in default values for the BPB.
# We fill in 33 null bytes so if the BIOS does this, we're good.
_start:
  jmp start

.space 33, 0

start:
  # Ensures the segment register %cs becomes 0x7c00 (0x00 + the 0x7c00 offset)
  # i.e. far jump changes the code segment
  jmp $0x00, $load_rm

# Must be the first code; all addresses are relative to this offset
load_rm:
  # Make sure the segments are pointed at the origin.
  # Normally, we'd point %ds at 0x7C0, which will point the data segment at 0x7C00.
  # e.g. 0x7C0 * 16 -> 0x7C00
  # But here, we point them at 0x00 because we're using a linker directive to essentially do ORG 0x7C00 (see linker.ld).
  # Essentially, we cannot assume the BIOS has done this for us.
  cli
  mov $0x00, %ax
  mov %ax, %ds
  mov %ax, %es
  mov %ax, %ss
  # Set the stack pointer explicitly
  mov $0x7C00, %sp
  sti

  mov $INIT_REAL_MODE_MSG, %si
  call print_string
  call print_newline

  call try_enable_a20_via_bios
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
  mov $0x90000, %ebp
  mov %ebp, %esp

  mov $INIT_PROT_MODE_MSG, %ebx
  call print_string_32

  # We now need to enable the A20 line.


  jmp .

### GDT ###
# Addressing no longer works the way it did in 16-bit real mode.
# Now, we must define a data structure called the Global Descriptor Table (GDT)
# to tell the CPU how we want to segment memory.

# In short: rather than multiply the value of a segment register by 16 and then add to it
# the offset, a segment register becomes an index to a particular segment descriptor (SD) in the GDT.

# From: https://www.cs.bham.ac.uk/~exr/lectures/opsys/10_11/lectures/os-dev.pdf
# "
# The simplest workable configuration of segment registers is described by Intel as the
# "basic flat model", whereby two overlapping segments are defined that cover the full 4 GB
# of addressable memory, one for code and the other for data.
# The fact that in this model these two segments overlap means that there is no attempt to protect one segment from
# the other, nor is there any attempt to use the paging features for virtual memory.

# In addition to the code and data segments, the CPU requires that the first entry
# in the GDT purposely be an invalid null descriptor (i.e. a structure of 8 zero bytes).
# The null descriptor is a simple mechanism to catch mistakes where we forget to set a
# particular segment register before accessing an address, which is easily done if we had
# some segment registers set to 0x0 and forgot to update them to the appropriate segment
# descriptors after switching to protected mode. If an addressing attempt is made with the
# null descriptor, then the CPU will raise an exception (interrupt).
# "

# Note - we'll be using paging later so much of the GDT setup isn't going to be all that
# important for long.
gdt_start:

gdt_null:
  # The aforementioned 8-byte NULL descriptor at the beginning
  .quad 0x0

# GDT entry for the code segment - base=0x00000000, length=0xFFFFF
gdt_code:
  # Segment length i.e. "limit"
  .word 0xFFFF
  # Segment base (bits 0-15)
  .word 0x0
  # Segment base (bits 16-23)
  .byte 0x0
  # Flags (8 bits)
  # 1st flags -> (present)1 (privilege)00 (descriptor type)1 -> 1001 b
  # type flags -> (code)1 (conforming)0 (readable)1 (accessed)0 -> 1010 b
  .byte 0b10011010
  # Flags (4 bits) + segment length (bits 16-19)
  # 2nd flags -> (granularity)1 (32 - bit default)1 (64 - bit seg)0 (AVL)0 -> 1100 b
  .byte 0b11001111
  # Segment base (bits 24-31)
  .byte 0x0

# Now the data segment. The flags are the same as the code segment except for the type flags
gdt_data:
  .word 0xFFFF
  .word 0x0
  .byte 0x0
  .byte 0b10010010
  .byte 0b11001111
  .byte 0x0

gdt_end:

gdt_descriptor:
  .word gdt_end - gdt_start - 1
  .long gdt_start

.equ CODE_SEG, gdt_code - gdt_start
.equ DATA_SEG, gdt_data - gdt_start

.include "boot/utils-16.s"
.include "boot/utils-32.s"
.include "boot/a20.s"

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
