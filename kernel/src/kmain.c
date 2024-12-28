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
#include "interrupt/timer.h"
#include "kernel.h"
#include "kstat.h"
#include "mem/base.h"
#include "mem/paging.h"
#include "proc/process.h"

unsigned int real_last_addr;
kstat_t      kstat;

void
kmain (unsigned int magic, unsigned int mbi, unsigned int last_addr) {
  real_last_addr = last_addr - KERNEL_PAGE_OFFSET;
  vga_global_console_init();

  multiboot_init(magic, mbi);
  klog_info("Multiboot data processed (if extant)");

  // Remap the PIC and mask all interrupts
  pic_init();
  klog_info("PIC remapped");

  // Allocate the IRQ table
  irq_init();
  klog_info("IRQ table allocated");

  // Register interrupts and exception handlers
  idt_init();
  klog_info("IDT initialized");

  // Allocate devices tables
  devices_init();
  klog_info("Devices table allocated");

  mem_init();
  klog_info("Permanent page tables installed");

  timer_init();
  klog_info("Timer initialized");

  proc_init();
  klog_info("Process table initialized");

  int_enable();
  cpu_idle();
}
