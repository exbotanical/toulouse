#define ASM_SOURCE 1
#include "mem/base.h"

/* TODO:? https://stackoverflow.com/questions/28837199/can-i-use-preprocessor-directives-in-ld-file */
/* TODO: preprocessor first */
OUTPUT_FORMAT("elf32-i386")
OUTPUT_ARCH(i386)
ENTRY(_start)
SECTIONS
{
	. = KERNEL_PHYSICAL_BASE;

	/* The kernel will live at 3GB + 1MB in the virtual address space, */
	/* which will be mapped to 1MB in the physical address space. */
	/* Note that we page-align the sections. */
	image_start = .;

	.setup ALIGN(4K) :
	{
		*(.setup)
	}

	. += KERNEL_PAGE_OFFSET;

	.text : AT (ADDR(.text) - KERNEL_PAGE_OFFSET)
	{
		*(.text)
	}
	text_end = .;

	.data ALIGN (4K) : AT (ADDR(.data) - KERNEL_PAGE_OFFSET)
	{
		*(.data)
		*(.rodata)
	}
	data_end = .;

	.bss : AT (ADDR (.bss) - KERNEL_PAGE_OFFSET)
	{
		*(COMMON)
		*(.bss)
	}
	image_end = .;

	/DISCARD/ : {
		*(.eh_frame)
	}
}
