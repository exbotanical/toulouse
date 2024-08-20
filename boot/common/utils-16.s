# print_ln(%si = "str")
print_ln_16:
  call print_string
  call print_newline
  ret

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
  je print_string_done
  # The actual interrupt which runs the ISR
  int $0x10
  jmp print_char

print_string_done:
  ret

print_newline:
  pusha

  mov $0xE, %ah
  # Newline
  mov $0x0a, %al
  int $0x10
  # Carriage return
  mov $0x0d, %al
  int $0x10

  popa
  ret

# print_hex(%dx = 0x1234)
print_hex:
  pusha

  # Index
  mov $0, %cx

# Grab the last digit of %dx and convert it to ASCII
# Numerals: '0' -> 0x30 (ASCII code), '1' -> 0x31, ... '9' -> 0x39
hex_loop:
  # 4 times because we're dealing with %dx i.e. a 16-bit value
  cmp $4, %cx
  je print_hex_done

  # Convert the last digit
  # %ax is our working register
  mov %dx, %ax
  # Bitwise & using 0x000F will mask the first three values to zero,
  # isolating the value we want to convert to ASCII e.g. 0x1234 -> 0x0004.
  and $0x000F, %ax
  # Add 0x03 ('0' in ASCII) to the isolated number to get the ASCII value:
  # e.g. 0x30 + 0x0002 -> 0x32, or '2'
  add $0x30, %al
  # Check if the digit is greater than 9...
  cmp $0x39, %al
  # If not, we're ready to print it
  jle place_char
  # Else, we need to convert A-F by adding 7.
  # For example, if the value is 'A', we just got 0x30 + 'A', or 0x3A,
  # which is 7 lower than the ASCII A (0x41)
  add $7, %al

# Get the string position for the converted ASCII char
place_char:
  # %bx = base address
  # Get base address + string length - char index
  mov $HEX_OUT + 5, %bx
  sub %cx, %bx

  # Copy char in %al into %bx
  mov %al, (%bx)
  # Rotate bitx of %dx 4 positions to the right, placing
  # the next hex digit into the least significant position.
  ror $4, %dx
  # Increment the index
  inc %cx
  jmp hex_loop

print_hex_done:
  mov $HEX_OUT, %si
  call print_string

  popa
  ret

HEX_OUT:
  .asciz "0x0000"
