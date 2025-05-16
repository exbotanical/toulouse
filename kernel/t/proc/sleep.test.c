#define __pid_t_defined  // To avoid typedef collision with pid_t in proc.h
#include "proc/sleep.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../stubs.h"
#include "kconfig.h"
#include "libtap/libtap.h"
#include "proc/proc.h"
#include "proc/sched.h"

#define NUM_SLEEP_HASH_TABLE_BUCKETS ((NUM_PROCS * 10) / 100)

#define TO_SLEEP_TABLE_HASH(addr)    ((addr) % (NUM_SLEEP_HASH_TABLE_BUCKETS))

// Mocks and helpers
static proc_t  test_proc;
static proc_t *sleep_hash_table[NUM_SLEEP_HASH_TABLE_BUCKETS];

extern proc_t *proc_current;
extern proc_t *proc_running_list;
extern bool    needs_resched;

static int fake_sig = 0;

unsigned int
eflags_get (void) {
  return 0;
}

void
int_disable (void) {}

void
eflags_set (uint32_t eflags) {}

int
sig_get (void) {
  return fake_sig;
}

void
proc_not_runnable (proc_t *proc, proc_state state) {
  proc->state = state;
}

void
proc_runnable (proc_t *proc) {
  proc->state = PROC_RUNNING;
}

void
sched_run (void) {}

static inline void
reset_mocks (void) {
  memset(&test_proc, 0, sizeof(test_proc));
  proc_current      = &test_proc;
  fake_sig          = 0;
  proc_running_list = NULL;
  needs_resched     = false;
  sleep_init();
}

static void
assert_sleeping (proc_t *p, void *addr) {
  eq_num(p->state, PROC_SLEEPING, "process is sleeping");
  eq_num(p->sleep_addr, addr, "process sleep addr is expected value");
}

static void
assert_running (proc_t *p) {
  eq_num(p->state, PROC_RUNNING, "process is running");
  eq_num(p->remaining_cpu_time, p->priority, "process quantum refilled");
  eq_null(p->sleep_addr, "sleep address is NULL");
}

static void
uninterruptible_sleep_test (void) {
  void *addr = (void *)0x1234;

  int ret    = sleep(addr, PROC_UNINTERRUPTIBLE);

  eq_num(ret, 0, "Uninterruptible sleep returns 0");
  assert_sleeping(&test_proc, addr);
  ok(test_proc.flags & PROC_FLAG_NOTINTERRUPT, "PROC_FLAG_NOTINTERRUPT is set");
}

static void
interruptible_sleep_no_signal_test (void) {
  void *addr = (void *)0x1234;

  int ret    = sleep(addr, PROC_INTERRUPTIBLE);

  eq_num(ret, 0, "Interruptible sleep without signal returns 0");
  assert_sleeping(&test_proc, addr);
  ok(!(test_proc.flags & PROC_FLAG_NOTINTERRUPT), "PROC_FLAG_NOTINTERRUPT not set");
}

static void
interruptible_sleep_with_signal_test (void) {
  void *addr      = (void *)0x5678;
  fake_sig        = 9;
  test_proc.state = PROC_RUNNING;

  int ret         = sleep(addr, PROC_INTERRUPTIBLE);

  eq_num(ret, 9, "Interruptible sleep with signal returns signal");
  assert_running(&test_proc);
}

static void
wakeup_test (void) {
  void *addr                       = (void *)0xDEADBEEF;

  proc_current->priority           = 42;
  proc_current->remaining_cpu_time = 0;

  sleep(addr, PROC_INTERRUPTIBLE);
  wakeup(addr);

  assert_running(&test_proc);
  ok(!(test_proc.flags & PROC_FLAG_NOTINTERRUPT), "PROC_FLAG_NOTINTERRUPT cleared");
  ok(needs_resched, "Needs resched set on wakeup");
}

static void
sleep_reentry_test (void) {
  void *addr = (void *)0x1337;

  proc_t proc1;
  memset(&proc1, 0, sizeof(proc1));
  proc1.priority           = 10;
  proc1.remaining_cpu_time = 0;
  proc1.state              = PROC_SLEEPING;
  proc1.sleep_addr         = addr;

  int ret                  = sleep(addr, PROC_INTERRUPTIBLE);

  eq_num(ret, 0, "Sleep returns 0 when already sleeping");
  assert_sleeping(&proc1, addr);
}

static void
multiple_sleepers_same_addr_test (void) {
  void *addr = (void *)0xCAFEBABE;

  proc_t proc1;
  memset(&proc1, 0, sizeof(proc1));
  proc1.priority           = 10;
  proc1.remaining_cpu_time = 0;

  proc_t proc2;
  memset(&proc2, 0, sizeof(proc2));
  proc2.priority           = 40;
  proc2.remaining_cpu_time = 0;

  proc_current             = &proc1;
  sleep(addr, PROC_INTERRUPTIBLE);

  eq_null(proc_current->next_sleeping, "no next sleeping pointer yet on proc1");
  eq_null(proc_current->prev_sleeping, "no prev sleeping pointer on proc1");

  proc_current = &proc2;
  sleep(addr, PROC_INTERRUPTIBLE);

  ok(proc_current->next_sleeping == &proc1, "proc2 next sleeping points to proc1");
  ok(proc1.prev_sleeping == proc_current, "proc1 prev sleeping points to proc2");

  assert_sleeping(&proc1, addr);
  assert_sleeping(&proc2, addr);

  wakeup(addr);

  assert_running(&proc1);
  assert_running(&proc2);
}

static void
no_sleeper_wakeup_test (void) {
  void *addr = (void *)0xABCD;
  wakeup(addr);

  ok(1, "Waking up on unused addr does not crash");
}

int
main () {
  plan(35);

  reset_mocks();
  uninterruptible_sleep_test();

  reset_mocks();
  interruptible_sleep_no_signal_test();

  reset_mocks();
  interruptible_sleep_with_signal_test();

  reset_mocks();
  wakeup_test();

  reset_mocks();
  sleep_reentry_test();

  reset_mocks();
  multiple_sleepers_same_addr_test();

  reset_mocks();
  no_sleeper_wakeup_test();

  done_testing();
}
