# dh = num_sectors_to_load
disk_load:
  push %dx

  # BIOS ISR for reading sector(s) from disk
  # https://www.ctyme.com/intr/rb-0607.htm
  mov $0x02, %ah
  # Read %dh sectors
  mov %dh, %al
  # Select cylinder 0
  mov $0x00, %ch
  # Start reading from the second sector (i.e. the one after the 512b bootloader)
  mov $0x02, %cl
  # Select head 0
  mov $0x00, %dh

  int $0x13
  # The BIOS will set the carry flag if there was a fault
  jc disk_error

  # Restore %dx - the high bits is the original num sectors arg
  pop %dx
  # Check the number of sectors actually read and compare to expected (%dh)
  cmp %al, %dh
  jne disk_error

  ret

disk_error:
  mov $DISK_ERR_MSG, %si
  call print_string
  jmp .

DISK_ERR_MSG: .asciz "Disk read error"
