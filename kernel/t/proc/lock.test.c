#include "proc/lock.h"

#include "../tests.h"

// Mocks for external functions and macros
static int          interrupts_enabled = 1;
static int          slept              = 0;
static void        *slept_on           = NULL;
static void        *woken_up           = NULL;
static unsigned int mock_eflags        = 0xDEADBEEF;
extern unsigned int area;  // from original file

unsigned int
eflags_get (void) {
  return mock_eflags;
}

void
int_disable (void) {
  interrupts_enabled = 0;
}

#define INTERRUPTS_ON()  (interrupts_enabled = 1)
#define INTERRUPTS_OFF() (interrupts_enabled = 0)

// Reset all mocks between tests
static void
reset_mocks (void) {
  interrupts_enabled = 1;
  slept              = 0;
  slept_on           = NULL;
  woken_up           = NULL;
  mock_eflags        = 0xDEADBEEF;
}

static void
lock_resource_uncontended_test (void) {
  resource_t res = {0};

  lock_resource(&res);

  ok(res.locked, "Resource should be locked");
  ok(!res.wanted, "Resource should not be marked as wanted");
  ok(!slept, "Should not sleep if resource is not locked");
  eq_num(interrupts_enabled, 1, "Interrupts should be re-enabled after lock");
}

// static void
// lock_resource_contended_test (void) {
//   resource_t res = {.locked = 1};

//   // Simulate second attempt to lock
//   lock_resource(&res);
//   ok(res.locked, "Resource should be locked after loop");
//   ok(res.wanted, "Resource should be marked as wanted");
//   ok(slept, "Should sleep if resource is locked");
//   ok(slept_on == &res, "Slept on correct resource");
//   eq_num(interrupts_enabled, 1, "Interrupts should be re-enabled");
//   eq_num(mock_eflags, 0xDEADBEEF, "eflags_get should be called");
// }

// static void
// unlock_resource_no_waiters_test (void) {
//   resource_t res = {.locked = 1};

//   unlock_resource(&res);

//   ok(!res.locked, "Resource should be unlocked");
//   ok(!res.wanted, "Resource wanted should stay false");
//   ok(!woken_up, "No wakeup if no waiters");
//   eq_num(interrupts_enabled, 1, "Interrupts re-enabled after unlock");
// }

static void
unlock_resource_with_waiters_test (void) {
  resource_t res = {.locked = 1, .wanted = 1};

  unlock_resource(&res);

  ok(!res.locked, "Resource should be unlocked");
  ok(!res.wanted, "Wanted flag should be cleared");
  ok(woken_up == &res, "Wakeup called with correct resource");
  eq_num(interrupts_enabled, 1, "Interrupts re-enabled after unlock");
}

static void
lock_area_basic_test (void) {
  area         = 0b0000;

  retval_t ret = lock_area(0b0010);

  eq_num(ret, 0, "Lock was not already held");
  eq_num(area, 0b0010, "Area now contains lock");
  // eq_num(interrupts_enabled, 1, "Interrupts re-enabled");
}

static void
lock_area_existing_test (void) {
  extern unsigned int area;
  area         = 0b0010;

  retval_t ret = lock_area(0b0010);

  eq_num(ret, 0b0010, "Lock was already held");
  eq_num(area, 0b0010, "Area remains unchanged");
  eq_num(interrupts_enabled, 1, "Interrupts re-enabled");
}

static void
unlock_area_basic_test (void) {
  extern unsigned int area;
  area         = 0b1111;

  retval_t ret = unlock_area(0b0100);

  eq_num(ret, 0b0100, "Lock was previously held");
  eq_num(area, 0b1011, "Lock bit cleared");
  eq_num(interrupts_enabled, 1, "Interrupts re-enabled");
}

static void
unlock_area_not_held_test (void) {
  extern unsigned int area;
  area         = 0b0011;

  retval_t ret = unlock_area(0b1000);

  eq_num(ret, 0b0000, "Lock was not held");
  eq_num(area, 0b0011, "Area unchanged");
  eq_num(interrupts_enabled, 1, "Interrupts re-enabled");
}

void
run_proc_lock_tests (void) {
  lock_resource_uncontended_test();
  reset_mocks();

  // lock_resource_contended_test();
  // reset_mocks();

  // unlock_resource_no_waiters_test();
  // reset_mocks();

  // unlock_resource_with_waiters_test();
  // reset_mocks();

  lock_area_basic_test();
  lock_area_existing_test();
  unlock_area_basic_test();
  unlock_area_not_held_test();
}
