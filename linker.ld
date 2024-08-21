ENTRY(_start)
OUTPUT_FORMAT("elf32-i386", "elf32-i386", "elf32-i386")
OUTPUT_ARCH(i386)
SECTIONS
{
  . = 1M;

  .text : ALIGN(4096)
  {
    *(.text)
  }

  .rodata : ALIGN(4096)
  {
    *(.rodata)
  }

  .data : ALIGN(4096)
  {
    *(.data)
  }

  .bss : ALIGN(4096)
  {
    *(COMMON)
    *(.bss)
  }
  /*
  Place the assembly far out of the way. The c code will be placed in the text section and we don't want to
  cause alignment issues considering the c compiler's 16 byte boundaries.

  NOTE: We don't put the actual kernel entrypoint assembly here because it must be the first thing in memory
  (at the starting address).
  */
  .asm : ALIGN(4096)
  {
    *(.asm)
  }
}
