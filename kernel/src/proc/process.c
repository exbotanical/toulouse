#include "proc/process.h"

#include "lib/string.h"

process_t   *proc_table;
process_t   *proc_pool_head;
process_t   *proc_table_head;
process_t   *proc_table_tail;
unsigned int proc_free_slots = 0;

void
proc_init (void) {
  kmemset(proc_table, 0, proc_table_size);

  proc_pool_head = NULL;
  unsigned int n = (proc_table_size / sizeof(process_t)) - 1;
  do {
    process_t *p   = &proc_table[n];
    p->next        = proc_pool_head;
    proc_pool_head = p;
    proc_free_slots++;
  } while (n--);

  proc_table_head = proc_table_tail = NULL;
}
