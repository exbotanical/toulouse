#include "proc/sched.h"

#include <stdlib.h>
#include <string.h>

#include "../stubs.h"
#include "kconfig.h"
#include "libtap/libtap.h"
#include "mem/segments.h"
#include "proc/proc.h"

// TODO:
// unsigned int
// kmalloc (size_t size) {
//   return (unsigned int)malloc(size);
// }

// void
// kfree (unsigned int ptr) {
//   free((void *)ptr);
// }

static proc_t dummy_procs[3];

bool did_context_switch = false;
bool did_set_tss        = false;

void
do_switch (
  unsigned int      *prev_esp_ptr,
  unsigned int      *prev_eip_ptr,
  unsigned int       idle_esp,
  unsigned int       idle_eip,
  unsigned int       idle_cr3,
  unsigned short int tss
) {
  did_context_switch = true;
}

unsigned int
eflags_get (void) {
  return 0;
}

void
int_disable (void) {}

void
eflags_set (uint32_t eflags) {}

static void
no_switch_if_still_running_test (void) {
  needs_resched                    = false;
  proc_current                     = &dummy_procs[0];
  proc_current->state              = PROC_RUNNING;
  proc_current->remaining_cpu_time = proc_current->priority = 77;

  sched_run();

  ok(!did_context_switch, "No context switch should occur");
}

static void
switches_if_different_proc_selected_test (void) {
  dummy_procs[0].remaining_cpu_time = 0;
  dummy_procs[0].priority           = 1;
  dummy_procs[0].next_running       = &dummy_procs[1];
  dummy_procs[0].state              = PROC_RUNNING;

  dummy_procs[1].remaining_cpu_time = 2;
  dummy_procs[1].priority           = 2;
  dummy_procs[1].next_running       = NULL;
  dummy_procs[1].state              = PROC_RUNNING;

  proc_current->flags               = 0;

  proc_running_list                 = &dummy_procs[0];
  proc_running_list->next_running   = &dummy_procs[1];
  proc_current                      = &dummy_procs[0];
  needs_resched                     = true;

  did_context_switch                = false;

  sched_run();

  ok(did_context_switch, "Should perform context switch");
  eq_num(proc_current, &dummy_procs[1], "Should switch to proc with highest CPU time");
}

static void
recharges_all_on_empty_cpu_time_test (void) {
  dummy_procs[0].remaining_cpu_time = 0;
  dummy_procs[0].priority           = 3;
  dummy_procs[0].next_running       = &dummy_procs[1];
  dummy_procs[1].remaining_cpu_time = 0;
  dummy_procs[1].priority           = 4;
  dummy_procs[1].next_running       = NULL;

  proc_running_list                 = &dummy_procs[0];
  proc_current                      = &dummy_procs[0];

  needs_resched                     = true;
  did_context_switch                = false;
  did_set_tss                       = false;

  sched_run();

  eq_num(dummy_procs[0].remaining_cpu_time, 3, "Proc 0 should be recharged to priority");
  eq_num(dummy_procs[1].remaining_cpu_time, 4, "Proc 1 should be recharged to priority");
  ok(did_context_switch, "Should still switch context");
  eq_num(proc_current, &dummy_procs[1], "Should select highest after recharge");
}

static void
sched_set_tss_sets_gdt_test (void) {
  i386_tss_t tss = {
    .cr3 = 1,
    .ds  = 1,
  };

  proc_t test_proc;
  memset(&test_proc, 0, sizeof(test_proc));
  test_proc.tss.esp = 0xCAFEBABE;

  memset(gdt, 0, sizeof(gdt));
  test_proc.tss = tss;

  seg_desc_t *g = &gdt[TSS / sizeof(seg_desc_t)];

  sched_set_tss(&test_proc);

  unsigned int base = (unsigned int)&test_proc.tss;

  eq_num(g->low_base, (base & 0xFFFFFF), "GDT base should match TSS");
  eq_num(g->low_flags, SD_TSS_PRESENT, "GDT flags should be SD_TSS_PRESENT");
  eq_num(g->hi_base, (base >> 24) & 0xFF, "GDT high base should match");
}

int
main (void) {
  plan(10);

  no_switch_if_still_running_test();
  switches_if_different_proc_selected_test();
  recharges_all_on_empty_cpu_time_test();
  sched_set_tss_sets_gdt_test();

  done_testing();
}
