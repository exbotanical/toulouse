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
gdt_start:

gdt_null:
  # The aforementioned 8-byte NULL descriptor at the beginning
  .long 0x0
  .long 0x0

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
