#ifndef INTERRUPT_IDT_H
#define INTERRUPT_IDT_H

#include "interrupt/irq.h"
#include "mem/segments.h"

#define IDT_NUM_ENTRIES 256

void idt_init(void);
void idt_load(unsigned int idtr);

#endif /* INTERRUPT_IDT_H */
