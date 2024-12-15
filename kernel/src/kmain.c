#include "kmain.h"

#include "arch/x86.h"
#include "drivers/console/vga.h"
#include "init/multiboot.h"
#include "interrupt/idt.h"
#include "interrupt/irq.h"
#include "interrupt/pic.h"

void
kmain (unsigned int magic, unsigned int mbi) {
  vga_global_console_init();
  multiboot_init(magic, mbi);

  // Remap the PIC and mask all interrupts
  pic_init();
  vgaprintf("[INIT] %s\n", "PIC remapped");
  // Allocate the IRQ table
  irq_init();
  vgaprintf("[INIT] %s\n", "IRQ table allocated");
  // Register interrupts and exception handlers
  idt_init();
  vgaprintf("[INIT] %s\n", "IDT initialized");

  asm volatile("int $0");

  vgaprintf("MAGICK: 0x%X\n", magic);
}
