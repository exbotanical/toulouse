#ifndef MOCKS_H
#define MOCKS_H

#include "proc/proc.h"

#define FAKE_EFLAGS_VAL 0xDEADBEEF

static bool         interrupts_enabled = true;
static bool         slept              = false;
static void        *slept_on           = NULL;
static void        *woken_up           = NULL;
static unsigned int mock_eflags        = FAKE_EFLAGS_VAL;
extern unsigned int area;

unsigned int
eflags_get (void) {
  return mock_eflags;
}

void
int_disable (void) {
  interrupts_enabled = false;
  mock_eflags        = 0x1;
}

void
eflags_set (uint32_t eflags) {
  interrupts_enabled = true;
  mock_eflags        = FAKE_EFLAGS_VAL;
}

int
sleep (void *addr, proc_inttype state) {
  slept    = true;
  slept_on = addr;
}

void
wakeup (void *addr) {
  woken_up = addr;
}

// Reset all mocks between tests
static void
reset_mocks (void) {
  interrupts_enabled = true;
  slept              = false;
  slept_on           = NULL;
  woken_up           = NULL;
  mock_eflags        = FAKE_EFLAGS_VAL;
}

#include <pthread.h>  // Must come later to avoid typedef collissions

#endif                /* MOCKS_H */
