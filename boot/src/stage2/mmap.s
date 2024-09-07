.code16

.data
.equ      MMAP_ENTRIES, 0xA500
SMAP:
  .ascii  "PAMS"                  # Little endian

multiboot_info:
.flags:
  .long   0x0
.memlo:
  .long   0x0
.memhi:
  .long   0x0
.bootdev:                         # Boot device. Present if flags[1] is set.
  .long   0x0
.cmdline:                         # Kernel command line. Present if flags[2] is set.
  .long   0x0
.num_mods:                        # Number of modules loaded along with kernel. Present if flags[3] is set.
  .long   0x0
.mods_addr:
  .long   0x0
.syms0:                           # Symbol table info. Present if flags[4] or flags[5] is set.
  .long   0x0
.syms1:
  .long   0x0
.syms2:
  .long   0x0
.syms3:
  .long   0x0
.mmap_len:                        # Memory map. Present if flags[6] is set.
  .long   0x0
.mmap_addr:
  .long   0x0
.drives_len:                      # Physical addr of first drive structure. Flags[7].
  .long   0x0
.drives_addr:
  .long   0x0
.config_tbl:                      # ROM config table. Flags[8].
  .long   0x0
.bootloader_name:                 # Bootloader name. Flags[9].
  .long   0x0
.apm_tbl:                         # Advanced Power Management table. Flags[10].
  .long   0x0
.vbe_ctrl_info:                   # Video BIOS Extension. Flags[11].
  .long   0x0
.vbe_mode_info:
  .long   0x0
.vbe_mode_resw:
  .long   0x0
.vbe_interface_seg:
  .long   0x0
.vbe_interface_off:
  .long   0x0
.vbe_interface_len:
  .long   0x0

.text
mmap_init:                        # See: https://www.ctyme.com/intr/rb-1741.htm
  xorw     %ax, %ax
  movw     %ax, %es
  movw     %ax, %ds

  # es:di -> buffer of 24b entries
  mov     $0xa504, %di            # This is where the map entries will start
  xorl    %ebx, %ebx              # This is where we'll store continuation values
  xorw    %bp, %bp                # This is where we'll store the number of entries
  movl    SMAP, %edx
  movl    $0xE820, %eax           # ISR 0e820h
  movw    $1, %es:20(%di)         # Force a valid ACPI 3.x entry
  movl    $24, %ecx               # Each entry can be up to 24b
  int     $0x15
  jc      mmap_error              # carry on the first call means unsupported

  cmpl    SMAP, %eax              # On success, %eax should equal 'SMAP'
  jne     mmap_error

  testl   %ebx, %ebx              # If %ebx == 0, only 1 entry (or none)
  je      mmap_error

  jmp     mmap_start_entry

mmap_next_entry:
  movl    $0xe820, %eax
  movw    $1, %es:20(%di)
  movl    $24, %ecx
  int     $0x15
  jc      mmap_done               # Carry means we've reached the end of the list
  movl    SMAP, %edx

mmap_start_entry:
  jcxz    mmap_skip_entry         # If mem map entry is 0b, skip it
  cmp     $20, %cl                # got a 24 byte ACPI 3.X response?
  jbe     notext
  testw   $1, %es:20(%di)
  je      mmap_skip_entry

notext:
  movl    %es:8(%di), %ecx        # Low 32 bits of length
  orl     %es:12(%di), %ecx       # OR with high 32 bits of length (also sets ZF)
  jz      mmap_skip_entry
  incw    %bp
  addw    $24, %di

mmap_skip_entry:
  test    %ebx, %ebx              # If 0, we still have entries to read
  jne     mmap_next_entry

mmap_done:
	mov     %bp, 0xA500
  clc
  ret

mmap_error:
  mov     $READ_MMAP_MSG, %si
  call    print_ln_16
  stc
  ret

mmap_setup:
  call    mmap_init

  mov     %bp, .mmap_len
  mov     $0xA500, .mmap_addr

  ret
