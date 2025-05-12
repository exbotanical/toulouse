#include "drivers/dev/char/charq.h"

#include <stdlib.h>
#include <string.h>

#include "../../tests.h"

static void
insert_and_get_char_test (void) {
  charq_t q    = {0};
  char    c    = 'A';

  retval_t ret = charq_put_char(&q, c);
  ok(ret == RET_OK, "put_char returns RET_OK on insert");

  unsigned char out = charq_get_char(&q);
  ok(out == c, "get_char returns inserted character");
}

static void
charq_capacity_test (void) {
  charq_t q        = {0};
  int     capacity = CHARQ_SIZE * CBLOCK_SIZE;
  int     i;

  for (i = 0; i < capacity; i++) {
    retval_t ret = charq_put_char(&q, 'x');
    ok(ret == RET_OK, "charq_put_char succeeds at index %d", i);
  }

  retval_t overflow_ret = charq_put_char(&q, 'y');
  ok(overflow_ret == RET_FAIL, "charq_put_char fails when full");
}

static void
unput_char_test (void) {
  charq_t q = {0};
  charq_put_char(&q, 'x');
  charq_put_char(&q, 'y');

  retval_t ret = charq_unput_char(&q);
  ok(ret == RET_OK, "unput_char returns RET_OK");

  unsigned char c = charq_get_char(&q);
  ok(c == 'x', "get_char after unput returns previous character");
}

static void
get_from_empty_test (void) {
  charq_t       q = {0};
  unsigned char c = charq_get_char(&q);
  ok(c == 0, "get_char from empty charq returns 0");
}

static void
unput_from_empty_test (void) {
  charq_t  q   = {0};
  retval_t ret = charq_unput_char(&q);
  ok(ret == RET_OK, "unput_char on empty charq returns RET_OK (noop)");
}

static void
flush_test (void) {
  charq_t q = {0};
  charq_put_char(&q, 'x');
  charq_put_char(&q, 'y');

  charq_flush(&q);
  ok(q.size == 0, "flush sets size to 0");
  ok(q.head == NULL && q.tail == NULL, "flush clears head and tail");

  unsigned char c = charq_get_char(&q);
  ok(c == 0, "get_char after flush returns 0");
}

static void
remaining_test (void) {
  charq_t q = {0};
  charq_put_char(&q, 'x');
  int rem = charq_remaining(&q);
  ok(rem == (CHARQ_SIZE * CBLOCK_SIZE - 1), "remaining returns correct value after one insert");
}

void
run_charq_tests (void) {
  insert_and_get_char_test();
  charq_capacity_test();
  unput_char_test();
  get_from_empty_test();
  unput_from_empty_test();
  flush_test();
  remaining_test();
}
