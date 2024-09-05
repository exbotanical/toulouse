# The A20 line is a specific address line in the CPU's memory addressing system.
# The CPU uses address lines to access different memory locations.
# The A20 line is the 21st address line (starting from A0).
# As aforementioned, older CPUs like the 8086 could only address 1 MB of memory
# (20 address lines, A0 to A19).
# Even though they had 20-bit addresses, they could generate addresses beyond this
# limit thanks to segmentation.
# In these older CPUs, when an address exceeded 1 MB, it would "wrap around"
# to the beginning of memory. When newer CPUs like the 80286 were released,
# they could address more memory (over 1 MB) but they kept the wrap-around behavior
# for the sake of backward compatibility. The A20 line provides a means for disabling
# this behavior and allowing addressing above 1MB without wrapping. To do so, we
# must ensure the A20 bit is set to enabled.

# There's many ways to enable the A20 line, many of which depend on the chipset being used.
# A recommended way is:
# 1. First, test whether the BIOS has already enabled it (some do).
# 2. Try enabling via the BIOS in real mode.
# 3. Test BIOS enablement.
# 4. Try keyboard controller method.
# 5. Test if enabled in a loop with a time-out (keyboard ctrl method is often slow).
# 6. Try Fast A20.
# 7. Test if enabled in a loop with a time-out (I guess by "fast" they don't mean fast).
# 8. Failing all that, bork.
.code16
.data
# TODO: per error
EMSG_BIOS_A20:
  .asciz  "Failed to enable A20 line via BIOS"

.text
# Interrupt 15: https://www.ctyme.com/intr/int-15.htm
try_enable_a20_via_bios:
  pusha
  mov     $0x2403, %ax            # 1. Ask the BIOS if the A20 gate is supported
                                  # See: https://www.ctyme.com/intr/rb-1338.htm
  int     $0x15
  jb      fail                    # Failed if CF is set (CF cleared means success)

  cmp     $0x00, %ah              # Failed if %ah not set to zero
  jne     fail

  mov     $0x2402, %ax            # 2. Get A20 status
                                  # See: https://www.ctyme.com/intr/rb-1337.htm
  int     $0x15
  jb      fail

  cmp     $0x00, %ah
  jne     fail

  cmp     $0x01, %al              # AL = current state (00h disabled, 01h enabled)
  je      done                    # If eq, A20 already enabled



  mov     $0x2401, %ax            # 3. Try to enable the A20 line
                                  # See: https://www.ctyme.com/intr/rb-1336.htm
  # TODO: abstract this repeat logic into a fn
  int     $0x15
  jb      fail
  cmp     $0x00, %ah
  je      done

fail:
  mov     $EMSG_BIOS_A20, %si
  call    print_ln_16

done:
  popa
  ret

.code32

.text

# Return register: %eax
# Retval: 0 if disabled, 1 if enabled
# TODO: (push/pop)al
is_a20_enabled:
  mov     $0x112345, %edi         # Odd mb address
  mov     $0x012345, %esi         # Even mb address
  mov     %esi, (%esi)            # Compare - if the contents of both addresses actually
                                  # point to the same address, then we're wrapping (A20 disabled).
  mov     %edi, (%edi)
  cmpsl
  jne     enabled

disabled:
  mov     $0, %eax

enabled:
  mov     $1, %eax
  ret

# Tries to enable the A20 line via the keyboard controller.
# Intel's 8042 keyboard controller had a spare pin through which
# the designers decided to route the A20 line.
enable_a20_via_keyctrl:
  pushal
  cli

  call    keyctrl_until_writable
  mov     $0xAD, %al
  out     %al, $0x64

  call    keyctrl_until_writable
  mov     $0xD0, %al
  out     %al, $0x64

  call    keyctrl_until_readable
  in      $0x60, %al
  push    %eax

  call    keyctrl_until_writable
  mov     $0xD1, %al
  out     %al, $0x64

  call    keyctrl_until_writable
  pop     %eax
  or      $0x02, %al
  out     %al, $0x60

  call    keyctrl_until_writable
  mov     $0xAE, %al
  out     %al, $0x64

  call    keyctrl_until_writable

  sti
  popal
  ret

# "the chip will catch accesses to ports 0x64 and 0x60 and simulate the expected behaviour,
# also when no keyboard controller is present"
keyctrl_until_writable:
  in      $0x64, %al              # See: https://www.win.tue.nl/~aeb/linux/kbd/A20.html
  test    $0x02, %al              # Input buffer status: 0 -> empty and ready, 1 -> full/busy
  jnz     keyctrl_until_writable

  ret

keyctrl_until_readable:
  in      $0x64, %al
  test    $0x01, %al              # Output buffer status: 0 -> full/ready, 1 -> empty
  jz      keyctrl_until_readable
  ret

enable_a20_via_fast:
  pushal
  in      $0x92, %al
  or      $0x02, %al
  out     %al, $0x92

  popal
  ret
