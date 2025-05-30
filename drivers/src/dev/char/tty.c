#include "drivers/dev/char/tty.h"

#include "drivers/dev/char/console.h"
#include "drivers/dev/char/termios.h"
#include "drivers/dev/char/tmpcon.h"
#include "drivers/dev/device.h"
#include "lib/ctype.h"
#include "lib/string.h"
#include "proc/proc.h"

tty_t tty_table[NUM_TTYS];

static retval_t
opost (tty_t* tty, unsigned char ch) {
  int status = 0;

  if (termios_should_proc_output(tty)) {
    switch (ch) {
      case '\n':
        if (termios_should_map_nl_to_nlcr(tty)) {
          if (charq_remaining(&tty->write_q) >= 2) {
            charq_put_char(&tty->write_q, '\r');
            tty->column = 0;
          } else {
            return RET_FAIL;
          }
        }
        break;

      case '\t':
        while (tty->column < (tty->win.ws_col - 1)) {
          if (tty->tab_stop[++tty->column]) {
            break;
          }
        }
        break;

      case '\b':
        if (tty->column > 0) {
          tty->column--;
        }

      default:
        if (termios_should_uppercase(tty)) {
          ch = TOUPPER(ch);
        }
        if (!ISCTRL(ch)) {
          tty->column++;
        }
        break;
    }
  }

  return charq_put_char(&tty->write_q, ch);
}

static void
out_char (struct tty* tty, unsigned char ch) {
  if (ISCNTRL(ch) && !ISSPACE(ch) && (termios_should_echo_ctrl_chars(tty))) {
    if (termios_should_ignore_last_char(tty)
        || (!(termios_should_ignore_last_char(tty)) && termios_is_eof_char(tty, ch))) {
      charq_put_char(&tty->write_q, '^');
      charq_put_char(&tty->write_q, ch + 64);
      tty->column += 2;
    }
  } else {
    opost(tty, ch);
  }
}

static void
erase_char (tty_t* tty, unsigned char char2erase) {
  unsigned char ch;

  if (termios_should_erase_char(tty, char2erase)) {
    if ((ch = charq_unput_char(&tty->cooked_q)) && termios_should_echo(tty)) {
      // Create a backspace
      charq_put_char(&tty->write_q, '\b');
      charq_put_char(&tty->write_q, ' ');
      charq_put_char(&tty->write_q, '\b');

      if (ch == '\t') {
        tty->delete_tab(tty);
      }

      // If we echoed a control char, we need to erase it
      if (ISCNTRL(ch) && !ISSPACE(ch) && termios_should_echo_ctrl_chars(tty)) {
        charq_put_char(&tty->write_q, '\b');
        charq_put_char(&tty->write_q, ' ');
        charq_put_char(&tty->write_q, '\b');
      }
    }
  }

  if (termios_should_erase_word(tty, char2erase)) {
    bool word_seen = false;

    while (tty->cooked_q.size > 0) {
      ch = LAST_CHAR(&tty->cooked_q);
      if ((ch == ' ' || ch == '\t') && word_seen) {
        break;
      }

      if (ch != ' ' && ch != '\t') {
        word_seen = true;
      }

      erase_char(tty, tty->termios.c_cc[VERASE]);
    }
  }

  if (termios_should_kill_proc(tty, char2erase)) {
    while (tty->cooked_q.size > 0) {
      erase_char(tty, tty->termios.c_cc[VERASE]);
    }
    if (termios_should_echo_nl_on_kill(tty) && !termios_should_visual_erase(tty)) {
      charq_put_char(&tty->write_q, '\n');
    }
  }
}

void
tty_cook_input (tty_t* tty) {
  while (tty->read_q.size > 0) {
    unsigned char ch = charq_get_char(&tty->read_q);

    // Handle special signal-generating chars e.g. VINTR (Ctrl-C), VQUIT (Ctrl-), VSUSP (Ctrl-Z)
    // i.e. if signals are enabled and the last character wasn’t escaped (via VLNEXT):
    if ((tty->termios.c_lflag & ISIG) && !termios_should_ignore_last_char(tty)) {
      // Ctrl-C → VINTR: flush queues unless NOFLSH, then send SIGINT to the process group
      if (termios_should_flush_queues(tty, ch)) {
        if (!termios_noflush(tty)) {
          charq_flush(&tty->read_q);
          charq_flush(&tty->cooked_q);
        }

        if (tty->pgid > 0) {
          // TODO:
          // kill_pgrp(tty->pgid, SIGINT, KERNEL);
        }

        break;
      }

      // Ctrl-\ → VQUIT: send SIGQUIT
      if (termios_is_quit_char(tty, ch)) {
        if (tty->pgid > 0) {
          // kill_pgrp(tty->pgid, SIGQUIT, KERNEL);
        }
        break;
      }

      // Ctrl-Z → VSUSP: send SIGTSTP.
      if (termios_is_suspend_char(tty, ch)) {
        if (tty->pgid > 0) {
          // kill_pgrp(tty->pgid, SIGTSTP, KERNEL);
        }
        break;
      }
    }

    // Handle obligatory character transformations based on termios flags, where applicable
    if (termios_should_strip_8bit_ascii(tty)) {
      ch = TOASCII(ch);
    }
    if (termios_should_lowercase(tty)) {
      if (ISUPPER(ch)) {
        ch = TOLOWER(ch);
      }
    }

    // If not ignoring last char, handle carriage returns and newlines
    if (!termios_should_ignore_last_char(tty)) {
      if (ch == '\r') {
        if (termios_should_ignore_cr(tty)) {
          continue;
        }
        if (termios_should_map_cr_to_nl(tty)) {
          ch = '\n';
        }
      } else if (ch == '\n' && termios_should_map_nl_to_cr(tty)) {
        ch = '\r';
      }
    }

    // Interpret special editing chars such as erase, kill, etc.
    if (termios_is_canonical_mode(tty) && !termios_should_ignore_last_char(tty)) {
      if (termios_should_erase_char(tty, ch) || termios_should_erase_word(tty, ch)
          || termios_should_kill_proc(tty, ch)) {
        erase_char(tty, );
        continue;
      }

      if (termios_should_reprint_line(tty, ch)) {
        out_char(tty, ch);
        charq_put_char(&tty->write_q, '\n');

        cblock_t* cb = tty->cooked_q.head;
        while (cb) {
          for (int n = 0; n < cb->next_write_index; n++) {
            if (n >= cb->next_read_index) {
              out_char(tty, cb->data[n]);
            }
          }
          cb = cb->next;
        }
        continue;
      }

      if (termios_has_vlnext(tty, ch) && termios_extended_proc_enabled()) {
        tty->flags |= TTY_HAS_LNEXT;
        if (termios_should_echo_ctrl_chars()) {
          charq_put_char(&tty->write_q, '^');
          charq_put_char(&tty->write_q, '\b');
        }
        break;
      }

      if (termios_output_ctrl_enabled(tty)) {
        if (termios_should_resume_output(tty, ch)) {
          tty->start(tty);
          continue;
        }
        if (termios_should_pause_output(tty, ch)) {
          tty->stop(tty);
          continue;
        }

        if (termios_any_char_resumes_output(tty) {
          tty->start(tty);
        }
      }
    }

    if (termios_is_canonical_mode(tty)) {
      if (ISCTRL(ch) && !ISSPACE(ch) && termios_should_echo_ctrl_chars(tty)) {
        out_char(tty, ch);
        charq_put_char(&tty->cooked_q, ch);
        tty->flags &= ~TTY_HAS_LNEXT;
        continue;
      }

      if (ch == '\n') {
        tty->has_canon_ln = true;
      }
    }

    if (termios_should_echo(tty) || (termios_should_echo_newline(tty) && ch == '\n')) {
      out_char(tty, ch);
    }

    charq_put_char(&tty->cooked_q, ch);
    tty->flags &= ~TTY_HAS_LNEXT;
  }

  tty->output(tty);
  if (!termios_is_canonical_mode(tty) || (termios_is_canonical_mode(tty) && tty->has_canon_ln)) {
    // wakeup(&do_select);
  }
  // wakeup(&tty_read);
}

tty_t*
tty_get (deviceno_t devnum) {
  if (!devnum) {
    return NULL;
  }

  if (devnum == DEVICE_MKDEV(SYSCON_MAJOR, 1)) {
    // devnum = TODO:
  }

  // /dev/tty0
  if (devnum == DEVICE_MKDEV(VCONSOLE_MAJOR, 0)) {
    devnum = DEVICE_MKDEV(VCONSOLE_MAJOR, current_console);
  }

  if (devnum == DEVICE_MKDEV(SYSCON_MAJOR, 0)) {
    if (!proc_current->controlling_tty) {
      return NULL;
    }

    devnum = proc_current->controlling_tty->devnum;
  }

  // TODO: Check console, tty0, tty

  for (int num; num < NUM_TTYS; num++) {
    if (tty_table[num].devnum == devnum) {
      return &tty_table[num];
    }
  }

  return NULL;
}

retval_t
tty_register (deviceno_t devnum) {
  for (int num = 0; num < NUM_TTYS; num++) {
    if (tty_table[num].devnum) {
      // klog_error("tty device %d,%d already registered", DEVICE_MAJOR(devnum),
      // DEVICE_MINOR(devnum));
      return RET_FAIL;
    }

    if (!tty_table[num].devnum) {
      tty_table[num].devnum = devnum;
      tty_table[num].count  = 0;

      return RET_OK;
    }
  }

  klog_error("tty table is full");
  return RET_FAIL;
}

void
tty_init (void) {
  kmemset(tty_table, 0, sizeof(tty_table));
}
