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
#include "kstat.h"
#include "mem/base.h"
#include "mem/paging.h"

unsigned int real_last_addr;
k_stat       kstat;

void
kmain (unsigned int magic, unsigned int mbi, unsigned int last_addr) {
  real_last_addr = last_addr - KERNEL_PAGE_OFFSET;
  vga_global_console_init();

  multiboot_init(magic, mbi);
  vgaprintf("[INIT] %s\n", "Multiboot data processed (if extant)");

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

  mem_init();
  vgaprintf("[INIT] %s\n", "Permanent page tables installed");

  int_enable();
  cpu_idle();
}
