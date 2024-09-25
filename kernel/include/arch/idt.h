#ifndef IDT_H
#define IDT_H

#include "common/compiler.h"
#include "common/types.h"

#define NUM_INTERRUPTS 512
#define CODE_SEG       0x08  // TODO: shared
#define DATA_SEG       0x10  // TODO: shared

typedef struct {
  uint16_t lo_offset;
  uint16_t selector;
  uint8_t  zero;
  uint8_t  type_attr;
  uint16_t hi_offset;
} packed idt_desc_t;

typedef struct {
  uint16_t limit;
  uint32_t base;
} packed idtr_desc_t;

void idt_init(void);

#endif /* IDT_H */
