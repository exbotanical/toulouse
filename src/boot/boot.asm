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

gdt_data:                                       ; linked to DS, SS, ES, FS, GS
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
  mov eax, 1                                    ; eax register to represent starting sector
  mov ecx, 100                                  ; total sectors to load - value  == /dev/zero sectors written (see Makefile)
  mov edi,  0x0100000                           ; 1MB
  call ata_lba_read 
  jmp CODE_SEG:0x0100000

ata_lba_read:
  mov ebx, eax                                  ; backup the LBA
  shr eax, 24                                   ; send the highest 8 bits of the LBA to the harddisk ctrlr
  or eax, 0xE0                                  ; select the master drive
  mov dx, 0x1F6
  out dx, al
                                                ; sent the total sectors to read
  mov eax, ecx
  mov dx, 0x1F2
  out dx, al
                                                ; send yet more bits to the LBA
  mov eax, ebx                                  ; restore backup LBA
  mov dx, 0x1F3
  out dx, al

  mov dx, 0x1F4
  mov eax, ebx                                  ; restore backup LBA
  shr eax, 8
  out dx, al
                                                ; send upper 16 bits of the LBA
  mov dx, 0x1F5
  mov eax, ebx                                  ; restore backup LBA
  shr eax, 16
  out dx, al

  mov dx, 0x1f7
  mov al, 0x20
  out dx, al

.next_sector:                                   ; read all sectors into memory
  push ecx

.try:                                           ; check if we need to read
  mov dx, 0x1f7
  in al, dx
  test al, 8
  jz .try

  mov ecx, 256                                  ; move 256 words at a time
  mov dx, 0x1F0
  rep insw
  pop ecx                                       
  loop .next_sector
  ret

times 510-($ - $$) db 0                         ; fill 510 bytes 
dw 0xAA55
