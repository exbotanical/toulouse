#include "idt.h"

#include "config.h"
#include "termio.h"

idt_desc  idt_descriptors[TOULOUSE_TOTAL_INTERRUPTS];
idtr_desc idtr_descriptor;


void idt_zero() {
  term_print ("divide by zero error\n");
}

void
idt_set (int ino, void* addr)
{
  idt_desc* desc = &idt_descriptors[ino];
  desc->offset_1 = (uint32_t)addr & 0x0000ffff;
  desc->selector = TOULOUSE_CODE_SELECTOR;
  desc->zero = 0x00;
  desc->type_attr = 0xEE;
  desc->offset_2 = (uint32_t)addr >> 16;
}

void
idt_init (void)
{
  memset(idt_descriptors, 0, sizeof(idt_descriptors));
  idtr_descriptor.limit = sizeof(idt_descriptors) - 1;
  idtr_descriptor.base  = idt_descriptors;

  idt_set(0, idt_zero);
}
