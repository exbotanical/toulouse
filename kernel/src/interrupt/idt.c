#include "interrupt/idt.h"

#include "drivers/dev/char/tmpcon.h"
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

/**
 * Sets a single entry in the Interrupt Descriptor Table (IDT). Each entry tells the CPU where to
 * jump (i.e., which handler function to call) when a specific interrupt vector is triggered.
 *
 * @param num The interrupt vector number (0–255).
 * @param handler The 32-bit address of the interrupt handler function.
 * @param flags Attributes that describe the type of gate, privilege level, and whether it's
 * present.
 */
static void
idt_set_entry (int num, uint32_t handler, unsigned int flags) {
  // Extracts the lower 16 bits of the 32-bit handler address.
  // The IDT stores 32-bit addresses split into two 16-bit parts: low and high.
  idt[num].low_offset = handler & 0x0000FFFF;

  // This sets the code segment selector that the CPU will use when jumping to the handler.
  // KERNEL_CS refers to the kernel's code segment in the GDT (usually 0x08).
  idt[num].selector   = KERNEL_CS;

  // The flags are shifted left by 8 bits because the actual IDT descriptor layout expects the flags
  // in the high byte of this 16-bit field.
  // These flags typically include:
  // * Gate type(interrupt / trap gate)
  // * Descriptor privilege level(DPL)
  // * Present bit(P)
  idt[num].flags      = flags << 8;

  // Extracts the upper 16 bits of the handler address.
  // Together with `low_offset`, this forms the full 32-bit address to jump to.
  idt[num].hi_offset  = handler >> 16;
}

void
idt_init (void) {
  int n;

  kmemset(idt, 0, sizeof(idt));

  for (n = 0; n < IDT_NUM_ENTRIES; n++) {
    // Exception vectors (0 - 31). These are CPU exceptions (like divide-by-zero, page fault, etc.).
    // They use trap gates, which don't disable further interrupts automatically.
    if (n < 0x20) {
      idt_set_entry(n, (uint32_t)idt_exception_handlers[n], SD_32TRAPGATE | SD_PRESENT);
      continue;
    }
    // These are hardware interrupts, remapped by the PIC to start at vector 0x20.
    // irq_handlers[n - 0x20] gives the actual function for IRQ number n - 0x20.
    // They use interrupt gates, which automatically disable interrupts while running.
    if (n < 0x30) {
      idt_set_entry(n, (uint32_t)irq_handlers[n - 0x20], SD_32INTRGATE | SD_PRESENT);
      continue;
    }
    // For any interrupt not already handled (i.e., vectors 48–255), assign a default irq_unknown
    // handler.
    idt_set_entry(n, (uint32_t)&irq_unknown, SD_32INTRGATE | SD_PRESENT);
  }

  // Vector 0x80 is traditionally used for system calls.
  // SD_DPL3 sets the descriptor privilege level to 3, so user-mode (ring 3) code can invoke it.
  // Uses a trap gate so that interrupts stay enabled during the syscall.
  idt_set_entry(0x80, (uint32_t)&syscall, SD_32TRAPGATE | SD_DPL3 | SD_PRESENT);

  idt_load((unsigned int)&idtr);
}
