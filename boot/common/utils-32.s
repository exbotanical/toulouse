# In 32-bit protected mode, we don't have access to BIOS interrupts.
# In order to print, we can write directly to the VGA video memory,
# the mapping of which begins at address 0xB8000.
# The VGA has a color text mode across a 80x25 character grid where each
# cell is represented by two bytes: the first is the ASCII code of the char to be displayed;
# the second encodes char attributes, such as fg/bg color, blinking, etc.

# To calculate char position, we can use the following formula: 0xb8000 + 2 * (row * 80 + col)
.code32
.equ VIDEO_MEMORY, 0xB8000

print_string_32:
  pusha
  mov $VIDEO_MEMORY, %edx

print_string_32_loop:
  # Set char at the string pointer in the low bits
  movb (%ebx), %al
  # Set the attributes in the high bits (white fg, black bg)
  mov $0x0F, %ah

  # Null term check
  cmp $0, %al
  je print_string_32_done

  # Place our filled out %ax register into the VGA cell
  mov %ax, (%edx)
  # Next character (str++)
  inc %ebx
  # Next VGA position
  add $2, %edx
  jmp print_string_32_loop

print_string_32_done:
  popa
  ret
