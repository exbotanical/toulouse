#ifndef INTERRUPT_TIMER_H
#define INTERRUPT_TIMER_H

#define TIMER_IRQ 0
#define HZ        100 /* kernel's Hertz rate (100 = 10ms) */

typedef struct timer_task timer_task_t;

struct timer_task {
  int expires_at;
  void (*fn)(unsigned int);
  unsigned int  arg;
  timer_task_t* next;
};

typedef struct {
  void (*fn)(unsigned int);
  unsigned int arg;
} timer_task_request_t;

void timer_init(void);
void timer_task_add(timer_task_request_t* req, unsigned int ticks);
void timer_task_remove(timer_task_request_t* req);

#endif /* INTERRUPT_TIMER_H */
