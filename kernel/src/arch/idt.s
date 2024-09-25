.align 4
.globl load_idt; load_idt:
	movl	0x4(%esp), %eax
	lidt	(%eax)
	ret
