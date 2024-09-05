.code16
.data
DISK_DRIVE_NO:                    # This is where we'll store the drive number of the booted disk.
  .byte   0x80                    # We'll default to 0x80 (first HD).
DISK_ERR_MSG:
  .asciz  "Disk read error"

# disk address packet
# See: https://www.ctyme.com/intr/rb-0708.htm#Table272
packet:
  .byte   0x10                    # Size of packet (16 bytes)
  .byte   0x0                     # Reserved (always 0)
.num_sectors:                     # Maximum number of sectors (blocks) that can be loaded at once
  .word   127                     # (some BIOS can load more but we'll prefer a lower-bound to be safe)
.buf_off:                         # The buffer defines where the data will be loaded to
  .word   0x0                     # 16-bit offset in the segment
.buf_seg:                         # 16-bit segment where the data will be loaded
  .word   0x0
.lba_lower:                       # Specifies where the data we're loading resides on disk
  .long   0x0                     # Lower 32 bits of 48-bit starting LBA
.lba_upper:
  .long   0x0                     # Upper 16 bits of 48-bit starting LBA

.text
# ax=start sector
# cx=num 512b sectors to read
# bx=buffer offset
# dx=buffer segment
disk_load_16:

disk_load_16_start:
  cmp     $127, %cx

  jbe     disk_load_16_load       # If we're not requesting more than 127, go ahead and load.
                                  # Otherwise, we'll need to paginate.
  pusha
  mov     $127, %cx
  call    disk_load_16
  popa

  add     $127, %ax
  add     $4064, %dx              # 127 * 512 / 16

  sub     $127, %cx
  jmp     disk_load_16_start

# TODO: check if extensions supported
disk_load_16_load:
  mov     %ax, .lba_lower         # Fill out the disk address packet
  mov     %cx, .num_sectors
  mov     %dx, .buf_seg
  mov     %bx, .buf_off
  mov     $packet, %si

  mov     DISK_DRIVE_NO, %dl      # Restore the drive number.
                                  # This means we must have stored %dl in DISK_DRIVE_NO earlier.
  mov     $0x42, %ah              # Extended Read
                                  # See: https://www.ctyme.com/intr/rb-0708.htm
  int     $0x13
  jc      disk_load_16_error

  cmp     .num_sectors, %cx       # Check the number of sectors actually read and compare to expected
                                  # See: "disk address packet's block count field set to number of blocks
                                  # successfully transferred"
  jne     disk_load_16_error

  ret

disk_load_16_error:
  mov     $DISK_ERR_MSG, %si
  call    print_ln_16
  jmp     .
