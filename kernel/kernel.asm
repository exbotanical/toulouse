[BITS 32]

extern init
global _start

CODE_SEG equ 0x08
DATA_SEG equ 0x10

_start:
  mov ax, DATA_SEG
  mov ds, ax
  mov es, ax
  mov fs, ax
  mov gs, ax
  mov ss, ax
  ; Choose a memory address for a decent stack size
  mov ebp, 0x00200000
  mov esp, ebp

  ; Enable the A20 line
  in al, 0x02
  or al, 2
  out 0x92, al

  call init
  jmp $

times 512 - ($ - $$) db 0
