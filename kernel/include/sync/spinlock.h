#ifndef SPINLOCK_H
#define SPINLOCK_H

#include "common/compiler.h"
#include "common/types.h"
#include "lib/list.h"

typedef uint16_t ticket_t;

// Must be strictly in this order, or else head will overflow into tail
// This ordering will arrange a DWORD in memory like (0xTTTTHHHH)
typedef struct {
  ticket_t owner;
  ticket_t next;
} packed ticket_pair_t;

typedef struct {
  ticket_pair_t tickets;
  int           holder;
  list_head_t   list;
} spinlock_t;  // TODO: inner packed

static inline spinlock_t *
spinlock_create (void) {
  spinlock_t *sl = &(spinlock_t){
    .holder  = 0xFFFF,
    .tickets = {.next = 0, .owner = 0},
  };

  list_init(&sl->list);

  return sl;
}

static inline void
spinlock_init (spinlock_t *sl) {
  *sl = (spinlock_t){
    .holder  = 0xFFFF,
    .tickets = {.next = 0, .owner = 0},
  };

  list_init(&sl->list);
}

bool spinlock_trylock(volatile spinlock_t *lock);
void spinlock_lock(volatile spinlock_t *lock);
void spinlock_unlock(volatile spinlock_t *lock);

#endif /* SPINLOCK_H */
