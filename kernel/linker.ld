KERNEL_PHYSICAL_BASE = 0x00100000;
KERNEL_VIRTUAL_BASE = 0xC0000000;

/* TODO:? https://stackoverflow.com/questions/28837199/can-i-use-preprocessor-directives-in-ld-file */
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

	.multiboot_header : {
		*(.multiboot_header)
	}

	.entry : {
		*(.entry)
	}

	. += KERNEL_VIRTUAL_BASE;

	.text ALIGN (4K) : AT (ADDR (.text) - KERNEL_VIRTUAL_BASE)
	{
		*(.text)
	}

	.rodata ALIGN (4K) : AT (ADDR (.rodata) - KERNEL_VIRTUAL_BASE)
	{
		*(.rodata)
	}

	.data ALIGN (4K) : AT (ADDR (.data) - KERNEL_VIRTUAL_BASE)
	{
		*(.data)
	}

	.bss : AT (ADDR (.bss) - KERNEL_VIRTUAL_BASE)
	{
		*(COMMON)
		*(.bss)
	}

	image_end = . - KERNEL_VIRTUAL_BASE;

	/DISCARD/ : {
		*(.comment)
		*(.note.gnu.build-id)
	}
}
