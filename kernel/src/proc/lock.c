#include "proc/lock.h"

/**
 * A critical section lock
 */
static unsigned int area = 0;

void
lock_resource (resource_t *resource) {
  while (true) {
    INTERRUPTS_OFF();
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
