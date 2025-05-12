#ifndef TESTS_H
#define TESTS_H

#include "libtap/libtap.h"

#undef INTERRUPTS_ON
#undef INTERRUPTS_OFF

#define INTERRUPTS_ON() \
  { printf("X\n"); }
#define INTERRUPTS_OFF() \
  { printf("X\n"); }

void run_string_tests(void);
void run_flist_tests(void);
void run_list_tests(void);
void run_spinlock_tests(void);
void run_device_tests(void);
void run_charq_tests(void);

#endif /* TESTS_H */
