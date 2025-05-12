#ifndef SYNC_SPINLOCK_H
#define SYNC_SPINLOCK_H

#include "lib/compiler.h"
#include "lib/list.h"
#include "lib/types.h"

typedef uint16_t ticket_t;

/**
 * A ticket pair used for spinlocks.
 * Must be strictly in this order, or else head will overflow into tail.
 * This ordering will arrange a DWORD in memory like (0xTTTTHHHH).
 */
typedef struct {
  ticket_t owner;
  ticket_t next;
} packed ticket_pair_t;

/**
 * Represents a single spinlock with ticket pair.
 * Linked-list capable.
 */
typedef struct {
  ticket_pair_t tickets;
  int           holder;
  list_head_t   list;
} spinlock_t;  // TODO: inner packed

/**
 * Creates and returns an initialized spinlock. Wrapper for `spinlock_init`.
 *
 * @return spinlock_t*
 */
static inline spinlock_t *
spinlock_create (void) {
  spinlock_t *sl = &(spinlock_t){
    .holder  = 0xFFFF,
    .tickets = {.next = 0, .owner = 0},
  };

  list_init(&sl->list);

  return sl;
}

/**
 * Initializes a spinlock.
 *
 * @param sl A pointer to the spinlock to be initialized.
 */
static inline void
spinlock_init (spinlock_t *sl) {
  *sl = (spinlock_t){
    .holder  = 0xFFFF,
    .tickets = {.next = 0, .owner = 0},
  };

  list_init(&sl->list);
}

/**
 * Acquires the given spinlock.
 *
 * @param lock
 */
void spinlock_lock(volatile spinlock_t *lock);

/**
 * Relinquishes the given spinlock.
 *
 * @param lock
 */
void spinlock_unlock(volatile spinlock_t *lock);

#endif /* SYNC_SPINLOCK_H */
