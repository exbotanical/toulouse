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
# We're going to use multiboot 1 for now (TODO: impl multiboot 2).

# Zero means we promise all loaded boot modules will be aligned on 4kb page boundaries.
.set ALIGNMENT,   1 << 0
# Says the bootloader should supply a memory map
# TODO: actually do this in /boot
.set MEMINFO,     1 << 1
# Flags basically provide metadata about our multiboot setup.
.set FLAGS,       ALIGNMENT | MEMINFO
# Yes, the magic work really must be this (or 0x2BADB002 if we're a bootloader).
.set MAGIC,       0x1BADB002
.set CHECKSUM,    -(MAGIC + FLAGS)

.section .multiboot_header, "a"
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

.section .bootstrap_stack, "aw", @nobits
# Allocate an initial stack (amount is arbitrary - it just must be sufficient enough for initial use)
stack_bottom:
.skip 0x4000 # 16kb
stack_top:

# Preallocate pages used for paging. Don't hard-code addresses and assume they
# are available, as the bootloader might have loaded its multiboot structures or
# modules there. This lets the bootloader know it must avoid the addresses.
.section .bss, "aw", @nobits
	.align 4096
boot_page_directory:
	.skip 4096
boot_page_table1:
	.skip 4096
# Further page tables may be required if the kernel grows beyond 3 MiB.

.section .entry, "a"# The kernel entry point.
.type _start, @function
_start:
	# Physical address of boot_page_table1.
	movl $(boot_page_table1 - 0xC0000000), %edi
	# First address to map is address 0.
	# TODO: Start at the first kernel page instead. Alternatively map the first
	#       1 MiB as it can be generally useful, and there's no need to
	#       specially map the VGA buffer.
	movl $0, %esi
	# Map 1023 pages. The 1024th will be the VGA text buffer.
	movl $1023, %ecx
1:
	# Only map the kernel.
	cmpl $_kernel_start, %esi
	jl 2f
	cmpl $(_kernel_end - 0xC0000000), %esi
	jge 3f

	# Map physical address as "present, writable". Note that this maps
	# .text and .rodata as writable. Mind security and map them as non-writable.
	movl %esi, %edx
	orl $0x003, %edx
	movl %edx, (%edi)

2:
	# Size of page is 4096 bytes.
	addl $4096, %esi
	# Size of entries in boot_page_table1 is 4 bytes.
	addl $4, %edi
	# Loop to the next entry if we haven't finished.
	loop 1b

3:
	# Map VGA video memory to 0xC03FF000 as "present, writable".
	movl $(0x000B8000 | 0x003), boot_page_table1 - 0xC0000000 + 1023 * 4

	# The page table is used at both page directory entry 0 (virtually from 0x0
	# to 0x3FFFFF) (thus identity mapping the kernel) and page directory entry
	# 768 (virtually from 0xC0000000 to 0xC03FFFFF) (thus mapping it in the
	# higher half). The kernel is identity mapped because enabling paging does
	# not change the next instruction, which continues to be physical. The CPU
	# would instead page fault if there was no identity mapping.

	# Map the page table to both virtual addresses 0x00000000 and 0xC0000000.
	movl $(boot_page_table1 - 0xC0000000 + 0x003), boot_page_directory - 0xC0000000 + 0
	movl $(boot_page_table1 - 0xC0000000 + 0x003), boot_page_directory - 0xC0000000 + 768 * 4

	# Set cr3 to the address of the boot_page_directory.
	movl $(boot_page_directory - 0xC0000000), %ecx
	movl %ecx, %cr3

	# Enable paging and the write-protect bit.
	movl %cr0, %ecx
	orl $0x80010000, %ecx
	movl %ecx, %cr0

	# Jump to higher half with an absolute jump.
	lea 4f, %ecx
	jmp *%ecx

.section .text

4:
	# At this point, paging is fully set up and enabled.

	# Unmap the identity mapping as it is now unnecessary.
	movl $0, boot_page_directory + 0

	# Reload crc3 to force a TLB flush so the changes to take effect.
	movl %cr3, %ecx
	movl %ecx, %cr3

	# Set up the stack.
	mov $stack_top, %esp

	# Enter the high-level kernel.
	call kmain

	# Infinite loop if the system has nothing more to do.
	cli
1:	hlt
	jmp 1b


















HELLO:.asciz  " HELLO MOTO"
.equ VIDEO_MEMORY, 0xB8000

print_ln_32:
  call print_string_32
  call print_newline_32
  ret

print_string_32:
  pusha
  mov $VIDEO_MEMORY, %edx

print_string_32_loop:
  # Set char at the string pointer in the low bits
  movb (%ebx), %al
  # Set the attributes in the high bits (white fg, black bg)
  mov $0x0F, %ah

  # Null term check
  cmp $0, %al
  je print_string_32_done

  # Place our filled out %ax register into the VGA cell
  mov %ax, (%edx)
  # Next character (str++)
  inc %ebx
  # Next VGA position
  add $2, %edx
  jmp print_string_32_loop

print_string_32_done:
  popa
  ret

print_newline_32:
  mov $VIDEO_MEMORY, %edx

  movb $0x0a, %al
  movb $0x0d, %al
  # Set the attributes in the high bits (white fg, black bg)
  mov $0x0F, %ah
  mov %ax, (%edx)

  add $2, %edx

  ret
