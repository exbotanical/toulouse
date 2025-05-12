#include "lib/flist.h"

#include "tests.h"

typedef struct {
  int          data;
  flist_head_t flist_ref;
} flist_data_t;

void
flist_init_test (void) {
  flist_head_t head;
  flist_init(&head);

  ok(head.next == NULL, "initializes a singly-linked forward list");
}

void
flist_append_test (void) {
  flist_head_t head;
  flist_init(&head);

  flist_head_t next;
  flist_init(&next);
  flist_append(&next, &head);

  ok(head.next == &next, "appends the new node onto the end of the list");
  ok(next.next == NULL, "end points to NULL");

  flist_head_t next2;
  flist_init(&next2);
  flist_append(&next2, &head);

  ok(head.next == &next, "maintains the first two nodes");
  ok(next.next == &next2, "updates next -> next2");
  ok(next2.next == NULL, "end points to NULL");

  flist_head_t next3;
  flist_init(&next3);
  flist_append(&next3, &head);

  ok(head.next == &next, "maintains head -> next");
  ok(next.next == &next2, "maintains next -> next2");
  ok(next2.next == &next3, "links next -> next2 -> next3");
  ok(next3.next == NULL, "next3 node points to NULL");

  flist_head_t next4;
  flist_init(&next4);
  flist_append(&next4, &next2);

  ok(head.next == &next, "maintains head -> next");
  ok(next.next == &next2, "maintains next -> next2");
  ok(next2.next == &next3, "links next -> next2 -> next3");
  ok(next3.next == &next4, "links next4 at end despite flist_append head arg being a middle node");
  ok(next4.next == NULL, "next4 node points to NULL");
}

// TODO: insert with links to many
// TODO: insert head into non
void
flist_insert_test (void) {
  flist_head_t head;
  flist_init(&head);

  flist_head_t next;
  flist_init(&next);

  flist_append(&next, &head);

  // TODO: assert init'ed
  flist_head_t next2;
  flist_init(&next2);

  flist_insert(&next2, &head, &next);

  ok(head.next == &next2, "inserts the new node into the list");
  ok(next2.next == &next, "inserts the new node into the list");
  ok(next.next == NULL, "has NULL at the end of the list");
}

void
flist_entry_test (void) {
  static define_list(test_list);

  flist_data_t d = {
    .data = 10,
  };
  flist_data_t d2 = {
    .data = 100,
  };
  flist_data_t d3 = {
    .data = 1000,
  };

  flist_init(&test_list);
  flist_append(&d.flist_ref, &test_list);
  flist_append(&d2.flist_ref, &d.flist_ref);
  flist_append(&d3.flist_ref, &d2.flist_ref);

  flist_data_t* r = flist_first(&test_list, flist_data_t, flist_ref);
  ok(r->data == 10, "retrieves the correct node data");

  flist_data_t* r2 = flist_entry((&d.flist_ref)->next, flist_data_t, flist_ref);
  ok(r2->data == 100, "retrieves the correct node data");
}

void
flist_entry_foreach_test (void) {
  static define_list(test_list);

  flist_data_t d = {
    .data = 10,
  };
  flist_data_t d2 = {
    .data = 100,
  };
  flist_data_t d3 = {
    .data = 1000,
  };

  flist_init(&test_list);
  flist_append(&d.flist_ref, &test_list);
  flist_append(&d2.flist_ref, &test_list);
  flist_append(&d3.flist_ref, &test_list);
  // H -> 10 -> 100 -> 1000

  unsigned int  count = 0;
  flist_data_t* el;
  flist_foreach_entry(el, &test_list, flist_ref) {
    switch (count++) {
      case 0: ok(el->data == 10, "iterates the list (data=10)"); break;
      case 1: ok(el->data == 100, "iterates the list (data=100)"); break;
      case 2: ok(el->data == 1000, "iterates the list (data=1000)"); break;
    }
  }
}

void
run_flist_tests (void) {
  flist_init_test();
  flist_insert_test();
  flist_append_test();
  flist_entry_test();
  flist_entry_foreach_test();
}
