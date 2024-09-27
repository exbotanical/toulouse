#include "arch/idt.h"

#include "arch/x86.h"
#include "drivers/console/vga.h"
#include "lib/string.h"
#include "mem/base.h"

idt_desc_t  idt_descriptors[NUM_INTERRUPTS];
idtr_desc_t idtr_descriptor;

extern void load_idt(idtr_desc_t* idtr);

static void
page_fault_handler (uint32_t ec) {
  vga_printf("%s", "PAGE FAULT!");

  while (true) {
    idle();
  }
}

static void
idt_set (uint32_t interrupt_no, uint32_t addr) {
  idt_descriptors[interrupt_no].lo_offset = addr & 0x0000ffff;
  idt_descriptors[interrupt_no].selector  = CODE_SEG;
  idt_descriptors[interrupt_no].zero      = 0x00;
  idt_descriptors[interrupt_no].type_attr = 0xEE;
  idt_descriptors[interrupt_no].hi_offset = addr >> 16;
}

void
idt_init (void) {
  k_memset(idt_descriptors, 0, sizeof(idt_descriptors));
  idtr_descriptor.limit = sizeof(idt_descriptors) - 1;
  idtr_descriptor.base  = (uint32_t)idt_descriptors;

  idt_set(14, (uint32_t)page_fault_handler);
  load_idt(&idtr_descriptor);
}
