#include "kmain.h"

#include "arch/cpu.h"
#include "arch/interrupt.h"
#include "arch/x86.h"
#include "drivers/console/vga.h"
#include "fs/device.h"
#include "init/multiboot.h"
#include "interrupt/idt.h"
#include "interrupt/irq.h"
#include "interrupt/pic.h"
#include "kernel.h"

int k_param_memsize;
int k_param_extmemsize;

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

  // Allocate devices tables
  devices_init();
  vgaprintf("[INIT] %s\n", "Devices table allocated");

  vgaprintf("MAGICK: 0x%X\n", magic);

  int_enable();
  cpu_idle();
}
