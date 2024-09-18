#include "sync/spinlock.h"

#include "../tests.h"

void
spinlock_lock_test (void) {
  spinlock_t lock = {
    .holder  = 0,
    .tickets = {.owner = 0, .next = 0}
  };

  spinlock_lock(&lock);
  ok(lock.tickets.owner == 0, "lock does not modify the owner");
  ok(lock.tickets.next == 1, "lock increments the next ticket");

  spinlock_unlock(&lock);
  ok(lock.tickets.owner == 1, "unlock increments the owner");
  ok(lock.tickets.next == 1, "unlock does not increment the next ticket");

  spinlock_lock(&lock);
  ok(lock.tickets.owner == 1, "lock does not modify the owner");
  ok(lock.tickets.next == 2, "lock increments the next ticket");

  spinlock_unlock(&lock);
  ok(lock.tickets.owner == 2, "unlock increments the owner");
  ok(lock.tickets.next == 2, "unlock does not increment the next ticket");
}

void
run_spinlock_tests (void) {
  spinlock_lock_test();
}
