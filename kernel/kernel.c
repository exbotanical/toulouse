#include "kernel.h"

#include "termio.h"

void init(void) {
  term_init();
  term_print("Hello world!");
}
