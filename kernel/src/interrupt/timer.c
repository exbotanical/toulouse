#include "interrupt/timer.h"

#include "arch/eflags.h"
#include "drivers/console/tmpcon.h"
#include "interrupt/const.h"
#include "interrupt/irq.h"
#include "interrupt/pit.h"
#include "interrupt/signal.h"
#include "kconfig.h"
#include "kernel.h"
#include "kstat.h"
#include "sync/simplelock.h"

static void timer_irq(int num, sig_context_t* sc);
static void timer_irq_bh(sig_context_t* sc);
static void timer_task_bh(sig_context_t* sc);

timer_task_t  tt_pool[NUM_TIMER_TASKS];
timer_task_t* tt_pool_head;
timer_task_t* tt_head;

static interrupt_bh_t timer_bh         = {0, &timer_irq_bh, NULL};
static interrupt_bh_t tt_bh            = {0, &timer_task_bh, NULL};
static interrupt_t    timer_irq_config = {0, "timer", &timer_irq, NULL};

static void
timer_irq (int num, sig_context_t* sc) {
  if ((++kstat.ticks % HZ) == 0) {
    kstat.system_time++;
    kstat.uptime++;
  }

  timer_bh.flags |= IRQ_BH_ACTIVE;
}

static void
timer_irq_bh (sig_context_t* sc) {
  if (tt_head) {
    if (tt_head->expires_at > 0) {
      tt_head->expires_at--;
      if (!tt_head->expires_at) {
        tt_bh.flags |= IRQ_BH_ACTIVE;
      }
    } else {
      kprintf("%s(): callout losing ticks.\n", __func__);
      tt_bh.flags |= IRQ_BH_ACTIVE;
    }
  }
}

static void
timer_task_init (timer_task_t* tt, timer_task_request_t* req, unsigned int ticks) {
  kmemset(tt, 0, sizeof(timer_task_t));
  tt->fn         = req->fn;
  tt->arg        = req->arg;
  tt->expires_at = ticks;
}

static void
timer_task_free (timer_task_t* old) {
  old->next    = tt_pool_head;
  tt_pool_head = old;
}

static timer_task_t*
timer_task_get_free (void) {
  timer_task_t* next_free = NULL;
  if (tt_pool_head) {
    next_free       = tt_pool_head;
    tt_pool_head    = next_free->next;
    next_free->next = NULL;
  }
  return next_free;
}

static void
timer_task_insert (timer_task_t* tt) {
  if (!tt_head) {
    tt_head = tt;
  } else {
    timer_task_t** tmp = &tt_head;
    // For each timer task...
    while (*tmp) {
      // Try to find the first task with an expiry greater than that of the new task.
      if ((*tmp)->expires_at > tt->expires_at) {
        // Found - subtract the new expiry from the current one and insert the new task before it.
        (*tmp)->expires_at -= tt->expires_at;
        tt->next            = *tmp;
        break;
      }
      // Otherwise, deduct all tt expiries from ours.
      tt->expires_at -= (*tmp)->expires_at;
      tmp             = &(*tmp)->next;
    }
    *tmp = tt;
  }
}

static void
timer_task_bh (sig_context_t* sc) {
  void (*fn)(unsigned int);
  unsigned int arg;

  timer_task_t* tt;
  while (tt_head) {
    if (tt_head->expires_at) {
      break;
    }
    if (simplelock_lock(LOCK_TT)) {
      break;
    }

    fn      = tt_head->fn;
    arg     = tt_head->arg;
    tt      = tt_head;
    tt_head = tt_head->next;

    timer_task_free(tt);
    simplelock_unlock(LOCK_TT);

    fn(arg);
  }
}

void
timer_task_add (timer_task_request_t* req, unsigned int ticks) {
  unsigned int flags = eflags_get();
  timer_task_remove(req);

  timer_task_t* tt;
  if (!(tt = timer_task_get_free())) {
    klogf_warn("%s(): no more timer_task slots\n", __func__);
    goto done;
  }

  timer_task_init(tt, req, ticks);
  timer_task_insert(tt);

done:
  eflags_set(flags);
}

void
timer_task_remove (timer_task_request_t* req) {
  unsigned int flags = eflags_get();

  timer_task_t* tt   = tt_head;
  while (tt) {
    if (tt->fn == req->fn && tt->arg == req->arg) {
      if (tt_head) {
        timer_task_t** tmp = &tt_head;
        while (*tmp) {
          if ((*tmp) == tt) {
            if ((*tmp)->next != NULL) {
              *tmp                = (*tmp)->next;
              // Update the expires_at of the next tt to preserve the relative delay of all tts.
              (*tmp)->expires_at += tt->expires_at;
            } else {
              *tmp = NULL;
            }
            timer_task_free(tt);
            break;
          }
          tmp = &(*tmp)->next;
        }
      }
      break;
    }
    tt = tt->next;
  }

  eflags_set(flags);
}

void
timer_init (void) {
  irq_bottom_half_register(&timer_bh);
  irq_bottom_half_register(&tt_bh);

  pit_init(HZ);

  kmemset(tt_pool, 0, sizeof(tt_pool));

  tt_pool_head   = NULL;
  unsigned int n = NUM_TIMER_TASKS;
  while (--n) {
    timer_task_t* tt = &tt_pool[n];
    timer_task_free(tt);
  }
  tt_head = NULL;

  if (!irq_register(TIMER_IRQ, &timer_irq_config)) {
    irq_enable(TIMER_IRQ);
  }
}
