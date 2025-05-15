#include "proc/lock.h"

#include "../mocks.h"
#include "../tests.h"

static resource_t contentious_res = {};

static void *
unlocker (void *) {
  while (!contentious_res.wanted);
  unlock_resource(&contentious_res);
  return NULL;
}

static void
lock_resource_uncontended_test (void) {
  resource_t res = {};

  lock_resource(&res);

  ok(res.locked, "Resource should be locked");
  ok(!res.wanted, "Resource should not be marked as wanted");
  ok(!slept, "Should not sleep if resource is not locked");
  eq_num(interrupts_enabled, true, "Interrupts should be re-enabled after lock");
}

static void
lock_resource_contended_test (void) {
  contentious_res.locked = true;
  pthread_t t;

  // Using this thread allows us to prove `lock_resource` sets the wanted flag; otherwise it would
  // hang forever
  pthread_create(&t, NULL, unlocker, NULL);
  lock_resource(&contentious_res);

  ok(contentious_res.locked, "Resource should be locked after loop");

  ok(slept, "Should sleep if resource is locked");
  ok(slept_on == &contentious_res, "Slept on correct resource");
  eq_num(interrupts_enabled, true, "Interrupts should be re-enabled");
  eq_num(mock_eflags, FAKE_EFLAGS_VAL, "eflags_get should be called");

  pthread_join(t, NULL);
}

static void
unlock_resource_no_waiters_test (void) {
  resource_t res = {.locked = true};

  unlock_resource(&res);

  ok(!res.locked, "Resource should be unlocked");
  ok(!res.wanted, "Resource wanted should stay false");
  ok(!woken_up, "No wakeup if no waiters");
  eq_num(interrupts_enabled, true, "Interrupts re-enabled after unlock");
}

static void
unlock_resource_with_waiters_test (void) {
  resource_t res = {.locked = true, .wanted = true};

  unlock_resource(&res);

  ok(!res.locked, "Resource should be unlocked");
  ok(!res.wanted, "Wanted flag should be cleared");
  ok(woken_up == &res, "Wakeup called with correct resource");
  eq_num(interrupts_enabled, true, "Interrupts re-enabled after unlock");
}

static void
lock_area_basic_test (void) {
  area         = 0b0000;

  retval_t ret = lock_area(0b0010);

  eq_num(ret, RET_FAIL, "Lock was not already held");
  eq_num(area, 0b0010, "Area now contains lock");
  eq_num(interrupts_enabled, true, "Interrupts re-enabled");
}

static void
lock_area_existing_test (void) {
  extern unsigned int area;
  area         = 0b0010;

  retval_t ret = lock_area(0b0010);

  eq_num(ret, 0b0010, "Lock was already held");
  eq_num(area, 0b0010, "Area remains unchanged");
  eq_num(interrupts_enabled, true, "Interrupts re-enabled");
}

static void
unlock_area_basic_test (void) {
  extern unsigned int area;
  area         = 0b1111;

  retval_t ret = unlock_area(0b0100);

  eq_num(ret, 0b0100, "Lock was previously held");
  eq_num(area, 0b1011, "Lock bit cleared");
  eq_num(interrupts_enabled, true, "Interrupts re-enabled");
}

static void
unlock_area_not_held_test (void) {
  extern unsigned int area;
  area         = 0b0011;

  retval_t ret = unlock_area(0b1000);

  eq_num(ret, 0b0000, "Lock was not held");
  eq_num(area, 0b0011, "Area unchanged");
  eq_num(interrupts_enabled, true, "Interrupts re-enabled");
}

void
run_proc_lock_tests (void) {
  lock_resource_uncontended_test();
  reset_mocks();

  lock_resource_contended_test();
  reset_mocks();

  unlock_resource_no_waiters_test();
  reset_mocks();

  unlock_resource_with_waiters_test();
  reset_mocks();

  lock_area_basic_test();
  lock_area_existing_test();
  unlock_area_basic_test();
  unlock_area_not_held_test();
}
