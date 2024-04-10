#ifndef IDT_H
#define IDT_H

#include <stdint.h>

typedef struct {
  uint16_t offset_1;   // Offset bits 0 - 15
  uint16_t selector;   // Selector in GDT
  uint16_t zero;       // No-op; needed for alignment
  uint16_t type_attr;  // Descriptor type and attrs
  uint16_t offset_2;   // Offset bits 16 - 31
} idt_desc __attribute__((packed));

typedef struct {
  uint16_t limit;  // Size of desc table - 1
  uint16_t base;   // Base addr of the start of the idt
} idtr_desc __attribute__((packed));

#endif             /* IDT_H */
