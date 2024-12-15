.align 4
.global idt_load; idt_load:
  movl   0x4(%esp), %eax
  lidt   (%eax)
  ret
