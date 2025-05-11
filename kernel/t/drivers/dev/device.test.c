#include "drivers/dev/device.h"

#include "../../tests.h"

static void
device_getters_test (void) {
  deviceno_t dev = DEVICE_MKDEV(12, 34);
  eq_num(dev, 0x0C22, "DEVICE_MKDEV(12, 34) == 0x0C22");
  eq_num(DEVICE_MAJOR(dev), 12, "DEVICE_MAJOR returns correct major");
  eq_num(DEVICE_MINOR(dev), 34, "DEVICE_MINOR returns correct minor");

  dev = DEVICE_MKDEV(0xAB, 0xCD);
  eq_num(DEVICE_MAJOR(dev), 0xAB, "DEVICE_MAJOR for 0xABCD == 0xAB");
  eq_num(DEVICE_MINOR(dev), 0xCD, "DEVICE_MINOR for 0xABCD == 0xCD");
}

static void
device_setters_test (void) {
  unsigned int minors[8] = {0};  // 256 bits

  // Initially all bits are clear
  for (int i = 0; i < 256; i++) {
    eq_num(DEVICE_TEST_MINOR(minors, i), 0, "Initial minor bit %d is clear", i);
  }

  // Set some bits
  DEVICE_SET_MINOR(minors, 0);
  DEVICE_SET_MINOR(minors, 31);
  DEVICE_SET_MINOR(minors, 32);
  DEVICE_SET_MINOR(minors, 255);

  ok(DEVICE_TEST_MINOR(minors, 0), "Bit 0 set correctly");
  ok(DEVICE_TEST_MINOR(minors, 31), "Bit 31 set correctly");
  ok(DEVICE_TEST_MINOR(minors, 32), "Bit 32 set correctly");
  ok(DEVICE_TEST_MINOR(minors, 255), "Bit 255 set correctly");

  // Clear one and test again
  DEVICE_CLEAR_MINOR(minors, 31);
  ok(!DEVICE_TEST_MINOR(minors, 31), "Bit 31 cleared correctly");

  // Other bits remain
  ok(DEVICE_TEST_MINOR(minors, 0), "Bit 0 still set");
  ok(DEVICE_TEST_MINOR(minors, 32), "Bit 32 still set");
  ok(DEVICE_TEST_MINOR(minors, 255), "Bit 255 still set");
}

void
run_device_tests (void) {
  device_getters_test();
  device_setters_test();
}
