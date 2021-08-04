ORG 0x7c00                                      ; specify assembly origin addr - later we will set to 0x7c00 so the assembler knows how to offset our data
BITS 16                                         ; assemble instructions as 16-bit

CODE_SEG equ gdt_code - gdt_init
DATA_SEG equ gdt_data - gdt_init

_init:
  jmp short init                                ; jump to the init routine
  nop
  
 times 33 db 0                                  ; config parameter block to prevent usb emulation from filling empty bits

init:
  jmp 0:mount                                   ; change code segment 

mount:
  cli                                           ; clear interrupts 
  mov ax, 0x00                                  ; set registers manually as we do not know what their state will be
  mov ds, ax
  mov es, ax
  mov ss, ax
  mov sp, 0x7c00                                ; move stack pointer
  sti                                           ; enable interrupts

  jmp $ 

.load_protected:
  cli                                           ; clear interrupts
  lgdt[gdt_descriptor]                          ; load gdt
  mov eax, cr0
  or eax, 0x1
  mov cr0, eax
  jmp CODE_SEG:load32                           ; switch to code selector and jump to load32 addr

gdt_init: 
gdt_null:                                       ; gdt
  dd 0x0                                        ; null descriptor
  dd 0x0
                                                ; offset 0x8
gdt_code:                                       ; - CS should point to this
  dw 0xffff                                     ; segment limit first 0-15 bits
  dw 0                                          ; base first 0-15 bits
  db 0                                          ; base 16-23 bits
  db 0x9a                                       ; access byte bitmask
  db 11001111b                                  ; high 4 bit flags and low 4 bit flags
  db 0                                          ; base 24-31 bits

                                                ; offset 0x10
gdt_data:                                       ; linked to DS, SS, ES, FS, GS
  dw 0xffff
  dw 0xffff  
  dw 0  
  db 0  
  db 0x92                                       ; amend byte bitmask
  db 11001111b  
  db 0

gdt_end:

gdt_descriptor:
  dw gdt_end - gdt_init-1
  dd gdt_init

[BITS 32]
load32:
  mov ax, DATA_SEG
  mov ds, ax
  mov es, ax
  mov fs, ax
  mov gs, ax
  mov ss, ax
  mov ebp, 0x00200000
  mov esp, ebp                                  ; set stack pointer

                                                ; enable the A20 line
  in al, 0x92
  or al, 2
  out 0x92, al
  
  jmp $

times 510-($ - $$) db 0                         ; fill 510 bytes 
dw 0xAA55
