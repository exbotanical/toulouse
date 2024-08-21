.code32

.global _start
.extern kernel_start

_start:
  call kernel_start
  jmp .

# Make sure the kernel asm is aligned
.fill 512 - (. - _start), 1, 0
