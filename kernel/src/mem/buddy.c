#include "mem/buddy.h"

#include "drivers/dev/char/tmpcon.h"
#include "lib/string.h"
#include "mem/base.h"
#include "mem/paging.h"

static kbuddy_head_t *freelist[BUDDY_MAX_LEVEL + 1];

void
kbuddy_init (void) {
  kmemset(freelist, 0, sizeof(freelist));
}
