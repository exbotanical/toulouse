# This is where we'll store the drive number of the booted disk.
# We'll default to 0x80 (first HD).
DISK_DRIVE_NO: .byte 0x80

# See: https://www.ctyme.com/intr/rb-0708.htm#Table272
packet:
  # Size of packet (16 bytes)
  .byte 0x10
  # Reserved (always 0)
  .byte 0x0

.num_sectors:
  # Maximum number of sectors (blocks) that can be loaded at once
  # (some BIOS can load more but we'll prefer a lower-bound to be safe)
  .word 127

# The buffer defines where the data will be loaded to
.buf_off:
  # 16-bit offset in the segment
  .word 0x0
.buf_seg:
  # 16-bit segment where the data will be loaded
  .word 0x0

# Specifies where the data we're loading resides on disk
.lba_lower:
  # Lower 32 bits of 48-bit starting LBA
  .long 0x0
.lba_upper:
  # Upper 16 bits of 48-bit starting LBA
  .long 0x0

# ax=start sector
# cx=num 512b sectors to read
# bx=buffer offset
# dx=buffer segment
disk_load_16:

.start:
  cmp $127, %cx
  # If we're not requesting more than 127, go ahead and load. Otherwise, we'll need to paginate.
  jbe .load

  pusha
  mov $127, %cx
  call disk_load_16
  popa

  add $127, %ax
  # 127 * 512 / 16
  add $4064, %dx

  sub $127, %cx
  jmp .start

# TODO: check if extensions supported
.load:
  # Fill out the disk address packet
  mov %ax, .lba_lower
  mov %cx, .num_sectors
  mov %dx, .buf_seg
  mov %bx, .buf_off
  mov $packet, %si

  # Restore the drive number. This means we must have stored %dl in DISK_DRIVE_NO earlier.
  mov DISK_DRIVE_NO, %dl

  # Extended Read
  # See: https://www.ctyme.com/intr/rb-0708.htm
  mov $0x42, %ah
  int $0x13
  jc .disk_error

  # Check the number of sectors actually read and compare to expected
  # See: "disk address packet's block count field set to number of blocks
  # successfully transferred"
  cmp .num_sectors, %cx
  jne .disk_error

  ret

.disk_error:
  mov $DISK_ERR_MSG, %si
  call print_string_16
  jmp .

DISK_ERR_MSG: .asciz "Disk read error"
