; BIOS has loaded us here
ORG 0x7c00

; We're in 16 bit real mode
BITS 16

CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

_init:
  jmp short init
  nop
; https://wiki.osdev.org/FAT#BPB_.28BIOS_Parameter_Block.29
; Prevent BPB overwriting the next ~30 bytes of our code on some devices e.g. USB
times 33 - ($ - _init) db 0

handle_zero:
  mov ah, 0x0e
  mov al, 'A'
  mov bx, 0x00
  int 0x10
  iret

init:
  ; Ensure we're at 0x7c00 in our code
  jmp 0:start

start:
  ; Clear interrupts - we don't want to screw up what we're about to do
  cli
  ; Setup the segment registers; we cannot rely on the BIOS to do this correctly
  mov ax, 0x00
  ; Set the data segment to where the BIOS has loaded us
  mov ds, ax
  mov es, ax
  ; Set stack segment to 0
  mov ss, ax
  mov sp, 0x7c00

  ; Re-enable interrupts
  sti

  ; Move the message into si at DS:SI
  mov si, load_msg
  call print

.load_protected:
  cli
  ; Load the GDT descriptor
  lgdt[gdt_descriptor]

  ; Set protected mode bit on control register 0vcfdsxawq
  mov eax, cr0
  or eax, 0x1
  mov cr0, eax

  jmp CODE_SEG:load32

  sti

gdt_start:

gdt_null:
  dd 0x0
  dd 0x0

; offset 0x8 - code segment
gdt_code:
  dw 0xffff ; Segment limit first 0-15 bits; this code can access full 4gb addressable memory
  dw 0 ; base first 0-15 bits - starts at address 0
  db 0 ; 16-23 bits
  db 0x9a ; access bytes - x/r
  db 11001111b ; high 4 bits and low 4 bits flags
  db 0 ; 24 -31 bits

; offset 0x10 - data segment
gdt_data: ; DS, SS
  dw 0xffff ; Segment limit first 0-15 bits
  dw 0 ; base first 0-15 bits
  db 0 ; 16-23 bits
  db 0x92 ; access bytes
  db 11001111b ; high 4 bits and low 4 bits flags
  db 0 ; 24 -31 bits

gdt_end:
gdt_descriptor:
  dw gdt_end - gdt_start - 1
  dd gdt_start ; offset

print:
.loop:
  ; Write char + attr
  ; http://www.ctyme.com/intr/rb-0099.htm
  mov ah, 0x09
  ; Page number for the BIOS video teletype instruction
  ; Keep this to zero - the visible page
  mov bh, 0x00
  ; Set fg color bits
  mov bl, 0x05
  ; Set num times to write char
  mov cx, 0x01
  ; Load a byte into the al register and increment si
  lodsb
  ; Cmp to char
  cmp al, 0
  ; If null byte, we're done
  je .done
  ; Call the BIOS interrupt
  int 0x10

  ; Now we update the cursor. If we don't do this,
  ; we'll just keep overwriting the same cell in the
  ; VGA buffer and output only the last character to the screen.

  ; Set page num again
  mov bh, 0x00
  ; Get cursor pos and size
  ; http://www.ctyme.com/intr/rb-0088.htm
  mov ah, 0x03
  int 0x10

  ; Set cursor position
  ; http://www.ctyme.com/intr/rb-0087.htm
  mov ah, 0x02
  ; Set page num again
  mov bh, 0x00
  ; Increment the column set by 03h
  inc dl
  int 0x10

  jmp .loop
.done:
  ret

load_msg:
  db 'Hello, world!', 0

[BITS 32]
load32:
  mov ax, DATA_SEG
  mov ds, ax
  mov es, ax
  mov fs, ax
  mov gs, ax
  mov ss, ax
  ; Choose a memory address for a decent stack size
  mov ebp, 0x00200000
  mov esp, ebp
  jmp $


; $ - current address in section
; $$ - start address of section
; db 0 - define byte n
; Thus 510 - (current_addr - start_addr) times zero
; Pad remaining bytes (up to 510) with zeros...
times 510 - ($ - $$) db 0
; So we can place the MBR signature in bytes 511 and 512
; ...in little endian because of Intel
dw 0xaa55
