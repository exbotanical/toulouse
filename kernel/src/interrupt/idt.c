#include "interrupt/idt.h"

#include "drivers/console/vga.h"
#include "interrupt/irq.h"
#include "lib/string.h"
#include "syscall/syscall.h"

gate_desc_t idt[IDT_NUM_ENTRIES];
descr_t     idtr = {sizeof(idt) - 1, (unsigned int)idt};

// Ignore `ISO C forbids initialization between function pointer and 'void *'`
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"

static void *irq_handlers[] = {
  &irq_0,
  &irq_1,
  &irq_2,
  &irq_3,
  &irq_4,
  &irq_5,
  &irq_6,
  &irq_7,
  &irq_8,
  &irq_9,
  &irq_10,
  &irq_11,
  &irq_12,
  &irq_13,
  &irq_14,
  &irq_15
};

static void *idt_exception_handlers[]
  = {&exception_0,  &exception_1,  &exception_2,  &exception_3,  &exception_4,  &exception_5,
     &exception_6,  &exception_7,  &exception_8,  &exception_9,  &exception_10, &exception_11,
     &exception_12, &exception_13, &exception_14, &exception_15, &exception_16, &exception_17,
     &exception_18, &exception_19, &exception_20, &exception_21, &exception_22, &exception_23,
     &exception_24, &exception_25, &exception_26, &exception_27, &exception_28, &exception_29,
     &exception_30, &exception_31};
#pragma GCC diagnostic pop

static void
idt_set_entry (int num, uint32_t handler, unsigned int flags) {
  idt[num].low_offset = handler & 0x0000FFFF;
  idt[num].selector   = KERNEL_CS;
  idt[num].flags      = flags << 8;
  idt[num].hi_offset  = handler >> 16;
}

void
idt_init (void) {
  int n;

  kmemset(idt, 0, sizeof(idt));

  for (n = 0; n < IDT_NUM_ENTRIES; n++) {
    if (n < 0x20) {
      idt_set_entry(n, (uint32_t)idt_exception_handlers[n], SD_32TRAPGATE | SD_PRESENT);
      continue;
    }
    if (n < 0x30) {
      idt_set_entry(n, (uint32_t)irq_handlers[n - 0x20], SD_32INTRGATE | SD_PRESENT);
      continue;
    }
    idt_set_entry(n, (uint32_t)&irq_unknown, SD_32INTRGATE | SD_PRESENT);
  }

  idt_set_entry(0x80, (uint32_t)&syscall, SD_32TRAPGATE | SD_DPL3 | SD_PRESENT);

  idt_load((unsigned int)&idtr);
}
