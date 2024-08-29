#include "arch/mmu.h"
#define KERNBASE 0x80000000         // First kernel virtual address
#define V2P_WO(x) ((x) - KERNBASE)

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
# Says we will supply a memory map
.set MEMINFO,     1 << 0
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

.globl _start
_start = V2P_WO(entry)

.type entry, @function
entry:
  hlt
  # Turn on page size extension for 4Mbyte pages
  // movl    %cr4, %eax
  // orl     $(CR4_PSE), %eax
  // movl    %eax, %cr4
  // # Set page directory
  // movl    $(V2P_WO(entrypgdir)), %eax
  // movl    %eax, %cr3
  // # Turn on paging.
  // movl    %cr0, %eax
  // orl     $(CR0_PG|CR0_WP), %eax
  // movl    %eax, %cr0
