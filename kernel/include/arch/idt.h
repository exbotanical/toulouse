#ifndef IDT_H
#define IDT_H

#include "common/types.h"

typedef struct {
  uint16_t size;
  uint32_t offset;
} __attribute__((__packed__)) idtr_t;

typedef struct {
  // Lower 16 bits of ISR address
  uint16_t isr_lo;

} __attribute__((__packed__)) idt_entry_t;

#endif /* IDT_H */
