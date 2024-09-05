# In 32-bit protected mode, we don't have access to BIOS interrupts.
# In order to print, we can write directly to the VGA video memory,
# the mapping of which begins at address 0xB8000.
# The VGA has a color text mode across a 80x25 character grid where each
# cell is represented by two bytes: the first is the ASCII code of the char to be displayed;
# the second encodes char attributes, such as fg/bg color, blinking, etc.

# To calculate char position, we can use the following formula: 0xb8000 + 2 * (row * 80 + col)
.code32
.data
.equ      VIDEO_MEMORY, 0xB8000

.text
print_ln_32:
  call    print_string_32
  call    print_newline_32
  ret

print_string_32:
  pusha
  mov     $VIDEO_MEMORY, %edx

print_string_32_loop:
  movb    (%ebx), %al             # Set char at the string pointer in the low bits
  mov     $0x0F, %ah              # Set the attributes in the high bits (white fg, black bg)

  cmp     $0, %al                 # Null term check
  je      print_string_32_done

  mov     %ax, (%edx)             # Place our filled out %ax register into the VGA cell
  inc     %ebx                    # Next character (str++)
  add     $2, %edx                # Next VGA position
  jmp     print_string_32_loop

print_string_32_done:
  popa
  ret

print_newline_32:
  mov     $VIDEO_MEMORY, %edx
  movb    $0x0a, %al
  movb    $0x0d, %al
  mov     $0x0F, %ah              # Set the attributes in the high bits (white fg, black bg)
  mov     %ax, (%edx)
  add     $2, %edx

  ret
