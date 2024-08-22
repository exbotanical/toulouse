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
  xor %ax, %ax
  mov %ax, %ds
  mov %ax, %es
  mov %ax, %ss
  mov %ax, %fs
  mov %ax, %gs
  # Set the stack pointer explicitly
  mov $0x7C00, %sp
  sti
  # The initial state of the direction flag isn't guaranteed
  # right now, so we'll reset it. The direction flag controls
  # the direction in which memory is accessed e.g. via lodsb.
  cld

  # The BIOS loads the drive number of the booted device into %dl on load.
  # We'll save it in DISK_DRIVE_NO for posterity.
  mov %dl, DISK_DRIVE_NO

  mov $INIT_REAL_MODE_MSG, %si
  call print_ln_16

load_stage2:
  # Start sector
  # For example, if _start = 0x7c00 and stage2_start (512 bytes away) is 0x7e00,
  # then the diff is 0x0200; divided by 512, that's 1. We start at the second sector.
  lea (stage2_start - _start)/512, %ax
  # Num 512b sectors to read
  lea (kernel_end - stage2_start)/512, %cx

  # Clear %dx - this will be the buffer base
  xor %dx, %dx
  # Buffer offset i.e. where we'll jump to
  lea stage2_start, %bx

  # Clear the buffer segment
  call disk_load_16

  jmp stage2_start

# TODO: logging/debug mode
INIT_REAL_MODE_MSG: .asciz "Stage 1 bootloader loaded in 16-bit real mode"

.include "boot/common/utils-16.s"
.include "boot/common/disk-16.s"
