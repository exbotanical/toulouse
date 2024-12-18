.code32

#define ASM_SOURCE 1

#include "mem/segments.h"
#include "init/multiboot.h"

.extern tmp_paging_init
.extern bss_init
.extern gdt_init
.extern get_last_boot_addr
.extern kmain

// This is the kernel entry-point.
// There's two ways we can end up here:
// 1. The bootloader implemented in boot/ loaded this code into memory, then
// jumped to it via a long jump; or
// 2. A different bootloader - such as GRUB - was used, in which case it will
// have jumped directly to this code.
// Effectively, both are doing the same thing. For //2, the host disk will
// copied just the kernel and not the full image with boot/ prepended.

// This allows us to support any bootloader that implements the multiboot spec.
// We're going to use multiboot 1 for now.

#define CR0_MP      0x00000002    // bit 01: enable monitor coprocessor
#define CR0_NE      0x00000020    // bit 05: enable native x87 FPU mode
#define CR0_WP      0x00010000    // bit 16: enable write protect (for CoW)
#define CR0_AM      0x00040000    // bit 18: enable alignment checking
#define CR0_PG      0x80000000    // bit 31: enable paging

// Flags basically provide metadata about our multiboot setup.
// e.g. we promise all loaded boot modules will be aligned on 4kb page boundaries.
// and the bootloader should supply a memory map
#define FLAGS       MULTIBOOT_PAGE_ALIGN | MULTIBOOT_MEMORY_INFO
// Yes, the magic word really must be this (or 0x2BADB002 if we're a bootloader).
#define MAGIC       MULTIBOOT_HEADER_MAGIC
#define CHECKSUM    -(MAGIC + FLAGS)

.section .setup, "a"

.align 4
tmp_gdtr:
  .word ((3 * 8) - 1)
tmp_gdta:
  .long tmp_gdt

.align 4
tmp_gdt:
  // null desc
  .word    0x0000
  .word    0x0000
  .word    0x0000
  .word    0x0000

  // kernel code
  .word    0xFFFF                 // segment limit 15-00
  .word    0x0000                 // base address 15-00
  .byte    0x00                   // base address 23-16
  .byte    0x9A                   // P=1 DPL=00 S=1 TYPE=1010 (exec/read)
  .byte    0xCF                   // G=1 DB=1 0=0 AVL=0 SEGLIM=1111
  .byte    GDT_BASE >> 24         // base address 31-24

  // kernel data
  .word    0xFFFF                 // segment limit 15-00
  .word    0x0000                 // base address 15-00
  .byte    0x00                   // base address 23-16
  .byte    0x92                   // P=1 DPL=00 S=1 TYPE=0010 (read/write)
  .byte    0xCF                   // G=1 DB=1 0=0 AVL=0 SEGLIM=1111
  .byte    GDT_BASE >> 24         // base address 31-24

.align 4
multiboot_header:
  .long   MAGIC
  .long   FLAGS
  .long   CHECKSUM

  .long   0x0                     // header_addr
  .long   0x0                     // load_addr
  .long   0x0                     // load_end_addr
  .long   0x0                     // bss_end_addr
  .long   0x0                     // entry_addr

  .long   0x0                     // mode_type
  .long   0x0                     // width
  .long   0x0                     // height
  .long   0x0                     // depth

.align 4
.global _start; _start:
  cli
  lgdt    tmp_gdtr
  movw    $KERNEL_DS, %cx
  movw    %cx, %ds
  movw    %cx, %es
  movw    %cx, %fs
  movw    %cx, %gs
  movw    %cx, %ss
  // Jump to higher half with an absolute jump.
  ljmp    $KERNEL_CS, $start_kernel

.text
.align 4
.type start_kernel, @function
start_kernel:
  // Setup the kernel stack
  movl    $KERNEL_PAGE_OFFSET + 0x10000, %esp
  pushl   $0                      // Reset EFLAGS
  popf

  // Enable paging. From this point onward, addresses are translated by the MMU.
  pushl   %ebx                    // Persist multiboot info
  pushl   %eax                    // Persist multiboot magic value
  call    tmp_paging_init         // Setup a temporary pagedir
  movl    %eax, %cr3

  movl    %cr0, %eax
  andl    $0x00000011, %eax       // Disable everything but preserve ET and PE (Grub) TODO: wut
  orl     $CR0_PG, %eax
  orl     $CR0_AM, %eax
  orl     $CR0_WP, %eax
  orl     $CR0_NE, %eax
  orl     $CR0_MP, %eax
  movl    %eax, %cr0

  call    bss_init
  call    gdt_init
  call    get_last_boot_addr

  popl    %ecx                    // restore multiboot magic num
  popl    %ebx                    // restore multiboot info

  pushl   %eax                    // last boot addr
  pushl   %ebx
  pushl   %ecx

  call    kmain                   // Enter the high-level kernel

  jmp     .
