ENTRY(_start)
OUTPUT_FORMAT(binary)
SECTIONS
{
  /* The BIOS drops us here (or rather looks for this address and jumps to it).
  We tell the linker to start here, which makes all addresses relative to this starting point.
  Without this, the assembler would assume the start address is 0x0000 and the code
  would be generated accordingly. If the BIOS loads this code at 0x7c00,
  then any jump, call, or data reference would be incorrect because the code
  was assembled for the wrong starting address.

  The reason the BIOS loads us here is the interrupts table and other BIOS things
  exist at the start of memory. 0x7c00 is deemed "safe" and means we won't clobber the BIOS stuff.
  It's kinda like this: */

  /*
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

  src: image from https://www.cs.bham.ac.uk/~exr/lectures/opsys/10_11/lectures/os-dev.pdf
  */
  . = 0x7C00;
  .stage1 : AT(0x7C00) SUBALIGN(4)
  {
    build/stage1.o(.*)
  }

  /* When the machine boots, the BIOS doesn't know how to load the OS;
  it delegates this responsibility to the boot sector.
  The boot sector is expected to be 512 bytes found in the first sector of the disk.
  The BIOS checks that bytes 511 and 512 of the ostensible boot sector equal 0xAA55
  in order to verify it is indeed bootable. */

  /* Fill the remainder of the output so it totals 512 bytes */
  . = 0x7DFE;
  /* Magic boot sector word: */
  .bootsig : {
    SHORT(0xaa55);
  }

  . = 0x7E00;
  .stage2 : AT(0x7E00) SUBALIGN(4)
  {
    build/stage2.o(.*)
  }
  . = ALIGN(512);

  . = 0x9000;
  .load_kernel : SUBALIGN(4)
  {
    *(.text*)
    *(.rodata*)
    *(.data*)
  }
  . = ALIGN(512);

  .bss : SUBALIGN(4) {
    __bss_start = .;
    *(COMMON);
    *(.bss*);
  }
  . = ALIGN(4);
  __bss_end = .;
  kernel_end = .;

  stage2_start = LOADADDR(.stage2);
  /* Get the LBA sector that we'll start loading from in stage 1.
  For example, if _start = 0x7c00 and stage2_start (512 bytes away) is 0x7e00,
  then the diff is 0x0200; divided by 512, that's 1. We start at the second sector. */
  stage2_sector = (stage2_start - _start)/512;
  num_sectors = (kernel_end - stage2_start)/512;

  /DISCARD/ : {
    *(.eh_frame);
    *(.comment);
    *(.note.gnu.*);
  }
}
