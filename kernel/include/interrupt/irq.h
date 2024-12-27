#ifndef INTERRUPT_IRQ_H
#define INTERRUPT_IRQ_H

#include "common/types.h"
#include "interrupt/signal.h"

#define NUM_IRQS      16
#define IRQ_BH_ACTIVE 0x01

typedef struct interrupt interrupt_t;

struct interrupt {
  unsigned int ticks;
  char        *name;
  void (*handler)(int, sig_context_t *);
  interrupt_t *next;
};

typedef struct interrupt_bh interrupt_bh_t;

struct interrupt_bh {
  int flags;
  void (*fn)(sig_context_t *);
  interrupt_bh_t *next;
};

extern interrupt_t *irq_table[NUM_IRQS];

extern void exception_0(void);
extern void exception_1(void);
extern void exception_2(void);
extern void exception_3(void);
extern void exception_4(void);
extern void exception_5(void);
extern void exception_6(void);
extern void exception_7(void);
extern void exception_8(void);
extern void exception_9(void);
extern void exception_10(void);
extern void exception_11(void);
extern void exception_12(void);
extern void exception_13(void);
extern void exception_14(void);
extern void exception_15(void);
extern void exception_16(void);
extern void exception_17(void);
extern void exception_18(void);
extern void exception_19(void);
extern void exception_20(void);
extern void exception_21(void);
extern void exception_22(void);
extern void exception_23(void);
extern void exception_24(void);
extern void exception_25(void);
extern void exception_26(void);
extern void exception_27(void);
extern void exception_28(void);
extern void exception_29(void);
extern void exception_30(void);
extern void exception_31(void);

extern void irq_0(void);
extern void irq_1(void);
extern void irq_2(void);
extern void irq_3(void);
extern void irq_4(void);
extern void irq_5(void);
extern void irq_6(void);
extern void irq_7(void);
extern void irq_8(void);
extern void irq_9(void);
extern void irq_10(void);
extern void irq_11(void);
extern void irq_12(void);
extern void irq_13(void);
extern void irq_14(void);
extern void irq_15(void);
extern void irq_unknown(void);

/**
 * Sets up the IRQ table.
 */
void irq_init(void);
void irq_bottom_half_register(interrupt_bh_t *new);
void irq_bottom_half_exec(sig_context_t *sc);
void irq_enable(int irq_num);
void irq_disable(int irq_num);
bool irq_register(int irq_num, interrupt_t *interrupt);
void irq_handler(int irq_num, sig_context_t sc);
void irq_unknown_handler(void);
void irq_spurious_interrupt_handler(int irq_num);

#endif /* INTERRUPT_IRQ_H */
