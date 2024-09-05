#ifndef BOOT_H
#define BOOT_H

/**
 * Loads the kernel ELF file and jumps to the kernel entrypoint. This also stands as the demarcation
 * between using the local bootloader and some third party bootloader such as GRUB.
 */
void load_kernel(void);

#endif /* BOOT_H */
