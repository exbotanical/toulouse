#ifndef IDT_H
#define IDT_H

#include "common/compiler.h"
#include "common/types.h"

typedef struct {
  uint16_t size;
  uint32_t offset;
} packed idtr_t;

typedef struct {
  // Lower 16 bits of ISR address
  uint16_t isr_lo;

} packed idt_entry_t;

#endif /* IDT_H */
