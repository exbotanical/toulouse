.code16
.data
HEX_OUT:
  .asciz  "0x0000"

.text
# print_ln(%si = "str")
print_ln_16:
  call    print_string_16
  call    print_newline
  ret

# print_string_16(%si = "str")
print_string_16:
  mov     $0xE, %ah               # BIOS ISR (interrupt service routine) for teletype mode
                                  # https://www.ctyme.com/intr/rb-0106.htm

# while (*si) print(*si); si++
print_char:
  lodsb                           # loads the byte from the address in si into al and increments si
  cmp     $0, %al
  je      print_string_16_done

  int     $0x10                   # The actual interrupt which runs the ISR
  jmp     print_char

print_string_16_done:
  ret

print_newline:
  mov     $0xE, %ah

  mov     $0x0a, %al              # Newline
  int     $0x10

  mov     $0x0d, %al              # Carriage return
  int     $0x10

  ret

print_hex:                        # print_hex(%dx = 0x1234)
  pusha

  mov     $0, %cx                 # Index


# Grab the last digit of %dx and convert it to ASCII
# Numerals: '0' -> 0x30 (ASCII code), '1' -> 0x31, ... '9' -> 0x39
hex_loop:
  cmp     $4, %cx                 # 4 times because we're dealing with %dx i.e. a 16-bit value
  je      print_hex_done



  mov     %dx, %ax                # Convert the last digit
                                  # %ax is our working register


  and     $0x000F, %ax            # Bitwise & using 0x000F will mask the first three values to zero,
                                  # isolating the value we want to convert to ASCII e.g. 0x1234 -> 0x0004.


  add     $0x30, %al              # Add 0x03 ('0' in ASCII) to the isolated number to get the ASCII value:
                                  # e.g. 0x30 + 0x0002 -> 0x32, or '2'
  cmp     $0x39, %al              # Check if the digit is greater than 9...
  jle     place_char              # If not, we're ready to print it

  add     $7, %al                 # Else, we need to convert A-F by adding 7.
                                  # For example, if the value is 'A', we just got 0x30 + 'A', or 0x3A,
                                  # which is 7 lower than the ASCII A (0x41)

# Get the string position for the converted ASCII char
# %bx = base address
place_char:
  mov     $HEX_OUT + 5, %bx       # Get base address + string length - char index
  sub     %cx, %bx
  mov     %al, (%bx)              # Copy char in %al into %bx
  ror     $4, %dx                 # Rotate bitx of %dx 4 positions to the right, placing
                                  # the next hex digit into the least significant position.
  inc     %cx                     # Increment the index
  jmp     hex_loop

print_hex_done:
  mov     $HEX_OUT, %si
  call    print_string_16

  popa
  ret
