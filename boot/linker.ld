ENTRY(_start)
OUTPUT_FORMAT(binary)
SECTIONS
{
  /*
    The BIOS drops us here (or rather looks for this address and jumps to it).
    We tell the linker to start here, which makes all addresses relative to this starting point.
    Without this, the assembler would assume the start address is 0x0000 and the code
    would be generated accordingly. If the BIOS loads this code at 0x7c00,
    then any jump, call, or data reference would be incorrect because the code
    was assembled for the wrong starting address.

    The reason the BIOS loads us here is the interrupts table and other BIOS things
    exist at the start of memory. 0x7c00 is deemed "safe" and means we won't clobber the BIOS stuff.
    It's kinda like this (image from https://www.cs.bham.ac.uk/~exr/lectures/opsys/10_11/lectures/os-dev.pdf):

    |-------------------|
    | Free              |
    |------0x1000000----|
    | BIOS (256kb)      |
    |------0xC0000------|
    | Video Mem (128kb) |
    |------0xA0000------|
    | Ext. BIOS Data    |
    | (639kb)           |
    |------0x9FC00------|
    | Free (638kb)      |
    |------0x7E00-------|
    | Boot Sector (Us!) |
    |------0x7C00-------|
    |-------0x500-------|
    | BIOS Data (256b)  |
    |-------0x400-------|
    | Interrupt Vec Tbl |
    |--------0x0--------|
  */
  . = 0x7C00;

  .text :
  {
    *(.text)
  }

  .rodata :
  {
    *(.rodata)
  }

  .data :
  {
    *(.data)
  }

  .bss :
  {
    *(COMMON)
    *(.bss)
  }
}
