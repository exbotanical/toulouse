# The BIOS loads us into 16-bit real mode for backward compatibility purposes (i.e. old 8086 processors).
# That is, in this mode memory addresses are calculated using 16-bit values;
# we can directly address up to 64kb of memory, starting from address 0x00000 to 0xFFFFF.
# Therefore, we instruct the assembler to generate 16-bit machine code here,
# affecting the size of registers, instructions, and memory addresses.
# Note, we can use the segment registers to reference addresses up to 1Mb.
.code16

# This is our entry-point at 0x7C00
.global   _start
.extern   stage2_start

.data

# TODO: logging/debug mode
INIT_REAL_MODE_MSG:
  .asciz  "Stage 1 bootloader loaded in 16-bit real mode"

.text
# https://wiki.osdev.org/FAT#BPB_(BIOS_Parameter_Block)
# Prevent BIOS from overwriting the next ~30 bytes of our code on some devices e.g. USB
# In short, the BIOS *may* fill in default values for the BPB.
# We fill in 33 null bytes so if the BIOS does this, we're good.
_start:
  jmp     stage1

.space    33, 0

# Ensures the segment register %cs becomes 0x7c00 (0x00 + the 0x7c00 offset)
# i.e. far jump changes the code segment
stage1:
  jmp     $0x00, $load_rm

# Must be the first code; all addresses are relative to this offset
load_rm:
  cli
  xor     %ax, %ax                # Make sure the segments are pointed at the origin.
  mov     %ax, %ds                # Normally, we'd point %ds at 0x7C0, which will point the data segment at 0x7C00.
  mov     %ax, %es                # e.g. 0x7C0 * 16 -> 0x7C00
  mov     %ax, %ss                # But here, we point them at 0x00 because we're using a linker directive
  mov     %ax, %fs                # to essentially do ORG 0x7C00 (see linker.ld).
  mov     %ax, %gs                # Essentially, we cannot assume the BIOS has done this for us.

  mov     $0x7C00, %sp            # Set the stack pointer explicitly
  sti

  cld                             # The initial state of the direction flag isn't guaranteed
                                  # right now, so we'll reset it. The direction flag controls
                                  # the direction in which memory is accessed e.g. via lodsb.
  mov     %dl, DISK_DRIVE_NO      # The BIOS loads the drive number of the booted device into %dl on load.
                                  # We'll save it in DISK_DRIVE_NO for posterity.
  mov     $INIT_REAL_MODE_MSG, %si
  call    print_ln_16

load_stage2:
  lea     stage2_sector, %ax      # Start sector
  mov     $num_sectors, %cx       # Num 512b sectors to read
  xor     %dx, %dx                # Clear %dx - this will be the buffer base
  lea     stage2_start, %bx       # Buffer offset i.e. where we'll jump to
  call    disk_load_16            # Clear the buffer segment

  jmp     stage2_start

.include  "src/common/utils-16.s"
.include  "src/stage1/disk-16.s"
