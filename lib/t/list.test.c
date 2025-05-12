#include "lib/list.h"

#include "tests.h"

typedef struct {
  int         data;
  list_head_t list_ref;
} list_data_t;

void
list_init_test (void) {
  list_head_t head;
  list_init(&head);

  ok(head.next == &head, "initializes a circular doubly-linked list");
  ok(head.prev == &head, "initializes a circular doubly-linked list");
}

void
list_insert_test (void) {
  list_head_t head;
  list_init(&head);

  list_head_t next;
  list_insert(&next, &head, &head);

  ok(head.next == &next, "inserts the new node into the list");
  ok(head.prev == &next, "inserts the new node into the list");
  ok(next.next == &head, "back-pointers the new node to the list");
  ok(next.prev == &head, "back-pointers the new node to the list");
}

void
list_append_test (void) {
  list_head_t head;
  list_init(&head);

  list_head_t next;
  list_append(&next, &head);

  ok(head.next == &next, "appends the new node onto the end of the list");
  ok(head.prev == &next, "appends the new node onto the end of the list");
  ok(next.next == &head, "back-pointers the new node onto the end of the list");
  ok(next.prev == &head, "back-pointers the new node onto the end of the list");

  list_head_t next2;
  list_append(&next2, &next);

  ok(head.next == &next, "maintains the first two nodes");
  ok(head.prev == &next2, "updates the head back-pointer");
  ok(next.next == &next2, "updates next <-> next2");
  ok(next.prev == &head, "maintains the next node back-pointer");
  ok(next2.next == &head, "new node points around to the head");
  ok(next2.prev == &next, "back-pointers the new node at the end");

  list_head_t next3;
  list_append(&next3, &next);

  ok(head.next == &next, "maintains head <-> next");
  ok(head.prev == &next2, "maintains head back-pointer");
  ok(next.prev == &head, "maintains head back-pointer");
  ok(next.next == &next3, "updates next -> next3");
  ok(next3.prev == &next, "back-pointers the new node at the end of next");
  ok(next3.next == &next2, "links next <-> next3 <-> next2");
  ok(next2.next == &head, "next2 node still points around to the head");
  ok(next2.prev == &next3, "back-pointers the new node at the end");
}

void
list_remove_test (void) {
  list_head_t head;
  list_init(&head);
  list_head_t next;
  list_append(&next, &head);
  list_head_t next2;
  list_append(&next2, &next);
  list_head_t next3;
  list_append(&next3, &next);
  // head <-> next <-> next3 <-> next2

  list_remove(&next3);
  // head <-> next <-> next2

  ok(head.prev == &next2, "removes the node");
  ok(head.next == &next, "removes the node");
  ok(next.prev == &head, "removes the node");
  ok(next.next == &next2, "removes the node");
  ok(next2.next == &head, "removes the node");
  ok(next2.prev == &next, "removes the node");

  ok(next3.prev == &next3, "removes pointers back to the list");
  ok(next3.next == &next3, "removes pointers back to the list");
}

void
list_entry_test (void) {
  static define_list(test_list);

  list_data_t d = {
    .data = 10,
  };
  list_data_t d2 = {
    .data = 100,
  };
  list_data_t d3 = {
    .data = 1000,
  };

  list_init(&test_list);
  list_append(&d.list_ref, &test_list);
  list_append(&d2.list_ref, &d.list_ref);
  list_append(&d3.list_ref, &d2.list_ref);

  list_data_t* r = list_first(&test_list, list_data_t, list_ref);
  ok(r->data == 10, "retrieves the correct node data");

  list_data_t* r2 = list_entry((&d.list_ref)->next, list_data_t, list_ref);
  ok(r2->data == 100, "retrieves the correct node data");

  list_data_t* r3 = list_entry((&test_list)->prev, list_data_t, list_ref);
  ok(r3->data == 1000, "retrieves the correct node data");
}

void
list_entry_foreach_test (void) {
  static define_list(test_list);

  list_data_t d = {
    .data = 10,
  };
  list_data_t d2 = {
    .data = 100,
  };
  list_data_t d3 = {
    .data = 1000,
  };

  list_init(&test_list);
  list_append(&d.list_ref, &test_list);
  list_append(&d2.list_ref, &test_list);
  list_append(&d3.list_ref, &test_list);
  // H <-> 1000 <-> 100 <-> 10

  unsigned int count = 0;
  list_data_t* el;
  list_foreach_entry(el, &test_list, list_ref) {
    switch (count++) {
      case 0: ok(el->data == 1000, "iterates the list (data=1000)"); break;
      case 1: ok(el->data == 100, "iterates the list (data=100)"); break;
      case 2: ok(el->data == 10, "iterates the list (data=10)"); break;
    }
  }
}

void
run_list_tests (void) {
  list_init_test();
  list_insert_test();
  list_append_test();
  list_remove_test();
  list_entry_test();
  list_entry_foreach_test();
}
