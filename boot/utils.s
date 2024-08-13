# print_string(%si = "str")
print_string:
  # BIOS ISR (interrupt service routine) for teletype mode
  # https://www.ctyme.com/intr/rb-0106.htm
  mov $0xE, %ah

# while (*si) print(*si); si++
print_char:
  # loads the byte from the address in si into al and increments si
  lodsb
  cmp $0, %al
  je print_done
  # The actual interrupt which runs the ISR
  int $0x10
  jmp print_char

print_done:
  ret
