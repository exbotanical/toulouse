#include "drivers/dev/char/charq.h"

#include "arch/eflags.h"
#include "arch/interrupt.h"
#include "lib/string.h"
#include "mem/alloc.h"

static bool
cblock_has_unread_chars (cblock_t* cb) {
  return cb->next_write_index > cb->next_read_index;
}

static bool
cblock_is_empty (cblock_t* cb) {
  return cb->next_write_index - cb->next_read_index == 0;
}

static cblock_t*
insert_cblock_in_tail (charq_t* q) {
  if (q->size >= CHARQ_SIZE) {
    return NULL;
  }

  // TODO: calloc
  cblock_t* cb = (cblock_t*)kmalloc(sizeof(cblock_t));
  if (!cb) {
    return NULL;
  }

  kmemset(cb, 0, sizeof(cblock_t));
  q->size++;

  if (!q->tail) {
    q->head = q->tail = cb;
  } else {
    cb->prev      = q->tail;
    cb->next      = NULL;
    q->tail->next = cb;
    q->tail       = cb;
  }

  return cb;
}

static void
delete_cblock_from_head (charq_t* q) {
  if (!q->head) {
    return;
  }

  cblock_t* tmp = q->head;
  if (q->head == q->tail) {
    q->head = q->tail = NULL;
  } else {
    q->head       = q->head->next;
    q->head->prev = NULL;
  }

  q->current_cblock_size -= tmp->next_write_index - tmp->next_read_index;
  q->size--;

  kfree((unsigned int)tmp);
}

static void
delete_cblock_from_tail (charq_t* q) {
  if (!q->tail) {
    return;
  }

  cblock_t* tmp = q->tail;
  if (q->head == q->tail) {
    q->head = q->tail = NULL;
  } else {
    q->tail       = q->tail->prev;
    q->tail->next = NULL;
  }

  q->current_cblock_size -= tmp->next_write_index - tmp->next_read_index;
  q->size--;
}

retval_t
charq_put_char (charq_t* q, unsigned char c) {
  INTERRUPTS_OFF();

  retval_t ret = RET_FAIL;

  cblock_t* cb = q->tail;
  // If no node on the list, create one
  if (!cb) {
    cb = insert_cblock_in_tail(q);
    if (!cb) {
      goto done;
    }
  }

  // If the current cblock hasn't reached the max size...
  if (cb->next_write_index < CBLOCK_SIZE) {
    // Insert the char at the end
    cb->data[cb->next_write_index] = c;
    cb->next_write_index++;
    q->current_cblock_size++;

    ret = RET_OK;
  }
  // We've reached the max; create a new cblock and try again
  else if (insert_cblock_in_tail(q)) {
    charq_put_char(q, c);
    ret = RET_OK;
  } else {
    goto done;
  }

done:
  INTERRUPTS_ON();

  return ret;
}

retval_t
charq_unput_char (charq_t* q) {
  INTERRUPTS_OFF();

  cblock_t* cb = q->tail;
  if (cb) {
    if (cblock_has_unread_chars(cb)) {
      cb->next_write_index--;
      q->current_cblock_size--;
    }

    if (cblock_is_empty(cb)) {
      delete_cblock_from_tail(q);
    }
  }

  INTERRUPTS_ON();

  return RET_OK;
}

unsigned char
charq_get_char (charq_t* q) {
  INTERRUPTS_OFF();

  unsigned char c  = 0;
  cblock_t*     cb = q->head;
  if (cb) {
    if (cblock_has_unread_chars(cb)) {
      c = cb->data[cb->next_read_index];
      cb->next_read_index++;
      q->current_cblock_size--;
    }

    if (cblock_is_empty(cb)) {
      delete_cblock_from_head(q);
    }
  }

  INTERRUPTS_ON();

  return c;
}

void
charq_flush (charq_t* q) {
  INTERRUPTS_OFF();

  while (q->head) {
    delete_cblock_from_head(q);
  }

  INTERRUPTS_ON();
}

int
charq_remaining (charq_t* q) {
  return (CHARQ_SIZE * CBLOCK_SIZE) - q->current_cblock_size;
}
