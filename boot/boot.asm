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
  call print16

.load_protected:
  cli
  ; Load the GDT descriptor
  lgdt[gdt_descriptor]

  ; Set protected mode bit on control register 0vcfdsxawq
  mov eax, cr0
  or eax, 0x1
  mov cr0, eax

  jmp CODE_SEG:load32

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

print16:
.loop:
  ; Write char + attr
  ; http://www.ctyme.com/intr/rb-0099.htm
  mov ah, 0x09
  ; Page number for the BIOS video teletype instruction
  ; Keep this to zero - the visible page
  mov bh, 0x00
  ; Set fg color bits
  mov bl, 0x03
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
  db 'Preparing to load kernel...', 0

; In protected mode now. Time to jump to the kernel in memory
[BITS 32]
load32:
  ; Starting sector to load from (0 is the boot sector, of course)
  mov eax, 1
  ; Total number of sectors to load
  ; However many sectors we padded with via dd
  mov ecx, 100
  ; 1MG - the address we want to load into
  mov edi, 0x0100000 ; TODO: const
  call ata_lba_read
  jmp CODE_SEG:0x0100000

; Because we're in protected mode now, we don't have interrupts like 13h.
; Instead, we'll have to perform direct disk access via I/O ports.
; This is gonna suck...
; https://wiki.osdev.org/ATA_read/write_sectors
ata_lba_read:
  ; Save the passed LBA for later
  mov ebx, eax

  ; Send the highest 8 bits of the LBA to the hard disk controller
  ; by shifting eax 24 bits to the right (32 - 24 = 8)
  shr eax, 24
  ; Select master drive
  or eax, 0xe0
  ; Grab the port number for the device
  mov dx, 0x1f6
  ; Write isolated 8 bits of eax to the I/O port (the rest is zeros after our >>)
  out dx, al

  ; Send the total num sectors to read
  mov eax, ecx
  mov dx, 0x1f2
  out dx, al

  ; Send LBA bits 0 - 7
  ; Restore the backup LBA into eax
  mov eax, ebx
  mov dx, 0x1f3
  out dx, al

  ; Send LBA bits 8 - 15
  mov dx, 0x1f4
  ; Restore the backup again just in case we somehow clobbered eax since the last time
  mov eax, ebx
  shr eax, 8
  out dx, al

  ; Send LBA bits 16 - 23
  mov dx, 0x1f5
  mov eax, ebx
  shr eax, 16
  out dx, al

  ; Use the command port and read with retry
  mov dx, 0x1f7
  mov al, 0x20
  out dx, al

; Try to read all sectors into memory
.next_sector:
  push ecx

; The controller is slow. Keep checking until it's ready
; for us to read.
.still_going:
  mov dx, 0x1f7
  in al, dx
  test al, 8
  jz .still_going

; Reading 256 words at a time
  mov ecx, 256
  mov dx, 0x1f0
  ; Read word from port and store in the address we want to load into
  ; (above specified as 1MB, or address 0x0100000)
  ; We rep by ecx times (256 times)
  rep insw
  ; Grab the number of sectors we wanted to read
  pop ecx
  ; Loop decrements ecx
  loop .next_sector
  ret

; $ - current address in section
; $$ - start address of section
; db 0 - define byte n
; Thus 510 - (current_addr - start_addr) times zero
; Pad remaining bytes (up to 510) with zeros...
times 510 - ($ - $$) db 0
; So we can place the MBR signature in bytes 511 and 512
; ...in little endian because of Intel
dw 0xaa55
