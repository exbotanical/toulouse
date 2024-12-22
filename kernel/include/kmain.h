#ifndef KMAIN_H
#define KMAIN_H

/**
 * Kernel entry-point, invoked by the bootloader.
 *
 * @param magic The multiboot magic number, if multiboot is being used.
 * @param mbi A pointer to the multiboot info structure, if multiboot is being used.
 * @param last_addr The address of the aligned page boundary a single page beyond the end of static
 * kernel memory. This is used so we can allocate resources just beyond the higher-half kernel.
 */
void kmain(unsigned int magic, unsigned int mbi, unsigned int last_addr);

#endif /* KMAIN_H */
