#include "proc/lock.h"

#include "arch/eflags.h"
#include "arch/interrupt.h"
#include "proc/sleep.h"

/**
 * A critical section lock
 */
unsigned int area = 0;

void
lock_resource (resource_t *resource) {
  unsigned int flags;

  while (true) {
#ifndef UNIT_TEST
    flags = eflags_get();
    int_disable();
#endif
    if (resource->locked) {
      resource->wanted = 1;
      INTERRUPTS_ON();
      sleep(resource, PROC_UNINTERRUPTIBLE);
    } else {
      break;
    }
  }

  resource->locked = true;
  INTERRUPTS_ON();
}

void
unlock_resource (resource_t *resource) {
  INTERRUPTS_OFF();

  resource->locked = false;
  if (resource->wanted) {
    resource->wanted = false;
    wakeup(resource);
  }

  INTERRUPTS_ON();
}

retval_t
lock_area (unsigned int type) {
  INTERRUPTS_OFF();

  retval_t ret  = area & type;
  area         |= type;

  INTERRUPTS_ON();

  return ret;
}

retval_t
unlock_area (unsigned int type) {
  INTERRUPTS_OFF();

  retval_t ret  = area & type;
  area         &= ~type;

  INTERRUPTS_ON();

  return ret;
}
