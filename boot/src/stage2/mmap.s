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
  xor     %ax, %ax
  mov     %ax, %es

  # es:di -> buffer of 24b entries
  mov     $MMAP_ENTRIES+4, %di    # This is where the map entries will start
  xor     %ebx, %ebx              # This is where we'll store continuation values
  xor     %bp, %bp                # This is where we'll store the number of entries
  mov     SMAP, %edx
  mov     $0xE820, %eax           # ISR 0e820h
  mov     $1, %es:20(%di)         # Force a valid ACPI 3.x entry
  mov     $24, %ecx               # Each entry can be up to 24b
  int     $0x15
  jc      mmap_error              # carry on the first call means unsupported

  mov     SMAP, %edx
  cmp     %edx, %eax              # On success, %eax should equal 'SMAP'
  jne     mmap_error

  test    %ebx, %ebx              # If %ebx == 0, only 1 entry (or none)
  je      mmap_error

  jmp     mmap_start_entry

mmap_next_entry:
  mov     $0xe820, %eax
  mov     $1, %es:20(%di)
  mov     $24, %ecx
  int     $0x15
  jc      mmap_done               # Carry means we've reached the end of the list
  mov     SMAP, %edx

mmap_start_entry:
  jcxz    mmap_skip_entry         # If mem map entry is 0b, skip it
  cmp     $20, %cl                # got a 24 byte ACPI 3.X response?
  jbe     notext
  test    $1, %es:20(%di)
  je      mmap_skip_entry

notext:
  mov     %es:8(%di), %ecx        # Low 32 bits of length
  or      %es:12(%di), %ecx       # OR with high 32 bits of length (also sets ZF)
  jz      mmap_skip_entry
  inc     %bp
  add     $24, %di

mmap_skip_entry:
  test    %ebx, %ebx              # If 0, we still have entries to read
  jz      mmap_done
  jmp     mmap_next_entry

mmap_done:
  mov     %es:MMAP_ENTRIES, %bp   # Store number of mmap entries when done
  clc
  ret

mmap_error:
  mov     $READ_MMAP_MSG, %si
  call    print_ln_16
  stc

get_memory_sz:
  push    %cx
  push    %dx
  xor     %cx, %cx
  xor     %dx, %dx

  mov     $0xE801, %ax
  int     $0x15
  jc      get_memory_sz_error
  cmp     $0x86, %ah              # Not supported :(
  je      get_memory_sz_error
  cmp     $0x80, %ah              # Invalid command
  je      get_memory_sz_error
  jcxz    get_memory_sz_use_ax
  mov     %cx, %ax
  mov     %dx, %bx

get_memory_sz_use_ax:
  pop     %dx
  pop     %cx
  ret

get_memory_sz_error:
  mov     $-1, %ax
  mov     $0, %bx

  jmp     get_memory_sz_use_ax

mmap_setup:
  xor     %eax, %eax
  xor     %ebx, %ebx
  call    get_memory_sz
  mov     %bx, .memhi
  mov     %ax, .memlo
  mov     $0x0, .bootdev

  call    mmap_init

  mov     %ax, .mmap_len
  mov     $MMAP_ENTRIES+4, .mmap_addr

  ret
