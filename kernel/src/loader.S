.code32
#include "arch/mmu.h"

.global _start

# This is the kernel entry-point.
# There's two ways we can end up here:
# 1. The bootloader implemented in boot/ loaded this code into memory, then
# jumped to it via a long jump; or
# 2. A different bootloader - such as GRUB - was used, in which case it will
# have jumped directly to this code.
# Effectively, both are doing the same thing. For #2, the host disk will
# copied just the kernel and not the full image with boot/ prepended.

# This allows us to support any bootloader that implements the multiboot spec.
# We're going to use multiboot 1 for now.

# Zero means we promise all loaded boot modules will be aligned on 4kb page boundaries.
.set ALIGNMENT,   1 << 0
# Says the bootloader should supply a memory map
.set MEMINFO,     1 << 1
# Flags basically provide metadata about our multiboot setup.
.set FLAGS,       ALIGNMENT | MEMINFO
# Yes, the magic work really must be this (or 0x2BADB002 if we're a bootloader).
.set MAGIC,       0x1BADB002
.set CHECKSUM,    -(MAGIC + FLAGS)

.section .multiboot_header, "a"
.align 4
.long    MAGIC
.long    FLAGS
.long    CHECKSUM

.section .bootstrap_stack, "aw", @nobits
# Allocate an initial stack (amount is arbitrary - it just must be sufficient enough for initial use)
stack_bottom:
.skip 0x4000 # 16kb
stack_top:

.section .entry, "a"
.type _start, @function
_start:
	lea     start_kernel, %ecx      # Jump to higher half with an absolute jump.
	jmp     *%ecx

.text
start_kernel:

	mov     $stack_top, %esp        # Set up the stack.
  push    %esi                    # Push the multi_boot info onto the stack
	call    kmain                   # Enter the high-level kernel

	cli
  jmp     .
