#ifndef MEM_LAYOUT_H
#define MEM_LAYOUT_H

/**
 * Initializes a temporary page directory so we can setup the higher-half kernel.
 *
 * @param magic The multiboot magic number, if extant.
 * @param mbi_ptr A pointer to the multiboot info structure, if extant.
 * @return unsigned int The address of the page directory.
 */
unsigned int mem_init_temporary(unsigned int magic, unsigned int mbi_ptr);

/**
 * Initializes the memory manager and permanent page directory. Also allocates kernel resources
 * beyond the higher-half static kernel memory boundary.
 */
void mem_init(void);

#endif /* MEM_LAYOUT_H */
