#ifndef INTERRUPT_IDT_H
#define INTERRUPT_IDT_H

#include "interrupt/irq.h"
#include "mem/segments.h"

#define IDT_NUM_ENTRIES 256

/**
 * Sets all interrupt handlers and loads the IDT.
 */
void idt_init(void);

/**
 * Loads the IDT pointed at by `idtr`.
 *
 * @param idtr
 */
void idt_load(unsigned int idtr);

#endif /* INTERRUPT_IDT_H */
