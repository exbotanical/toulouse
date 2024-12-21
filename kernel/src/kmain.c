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
  real_last_addr = last_addr - KERNEL_PAGE_OFFSET;  // 3221225472
  vga_global_console_init();
  // addr = ((unsigned int)image_end & PAGE_MASK) + PAGE_SZ;
  // 0xFFFC (65532) vs 0xC01080000 (51556909056 -> 48335683584) | 0xC0105A8D (3222297229 -> 1071757)
  vgaprintf("\n>>> 0x%X\n", last_addr);
  // image_end=0xc0107ea4 (3222306468)
  // -1072660480
  // 80108000
  // 0xc0108284

  // real_last_addr
  // dev: 0x109000 (1085440)
  // grub: 0x105A8D (1071757)

  // last_addr
  // dev: 0xc01090000 (51556974592)
  // grub: 0xc0105a8d (3222297229)
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

  // mem_init();
  vgaprintf("[INIT] %s\n", "Permanent page tables installed");

  int_enable();
  cpu_idle();
}
