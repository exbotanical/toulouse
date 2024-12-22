#include "sync/spinlock.h"

#include "arch/atomic.h"
#include "arch/interrupt.h"
#include "arch/x86.h"
#include "common/compiler.h"

void
spinlock_lock (volatile spinlock_t *lock) {
  // TODO: assert interrupts disabled

  // 1. Create a local ticket pair (this is what we'll increment with).
  // This represents the CPU's "desired ticket".
  register ticket_pair_t my_ticket = {.next = 1};

  // 2. Add my_ticket to tickets, and return the original tickets value in my_ticket.
  // This is the CPU's assigned ticket.
  my_ticket                        = atomic_xchg(add, &lock->tickets, my_ticket);

  // 3. If the  current owner of the lock equals the newly acquired ticket, the CPU is the next in
  // line.
  if (likely(my_ticket.owner == my_ticket.next)) {
    goto lock_out;
  }

  // 4. Otherwise, wait until our turn
  while (true) {
    if (access_once(lock->tickets.owner) == my_ticket.next) {
      goto lock_out;
    }
    idle();
  }

lock_out:
  barrier();
}

void
spinlock_unlock (volatile spinlock_t *lock) {
  barrier();
  atomic_add(&lock->tickets.owner, 1);
  barrier();
}
