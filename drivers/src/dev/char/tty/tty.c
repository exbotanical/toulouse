#include "drivers/dev/char/tty/tty.h"

#include "drivers/dev/char/console/console.h"
#include "drivers/dev/char/tmpcon.h"
#include "drivers/dev/char/tty/termios.h"
#include "drivers/dev/device.h"
#include "fs/fcntl.h"
#include "interrupt/timer.h"
#include "kernel.h"  // TODO: move extern kstat to kstat.h
#include "kstat.h"
#include "lib/ctype.h"
#include "lib/errno.h"
#include "lib/math.h"
#include "lib/string.h"
#include "proc/proc.h"
#include "proc/sched.h"
#include "proc/sleep.h"

tty_t tty_table[NUM_TTYS];

static void
wait_vtime_wrapper (unsigned int arg) {
  unsigned int* fn = (unsigned int*)arg;

  wakeup(fn);
}

/**
 * Performs output processing on the write queue
 */
static retval_t
opost (tty_t* tty, unsigned char ch) {
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

#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
      case '\b':
        if (tty->column > 0) {
          tty->column--;
        }
#pragma GCC diagnostic pop

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

/**
 * Writes the final, processed character to the write queue
 */
static void
out_char (struct tty* tty, unsigned char ch) {
  if (ISCTRL(ch) && !ISSPACE(ch) && (termios_should_echo_ctrl_chars(tty))) {
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

/**
 * Erases a character from the write queue
 */
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
      if (ISCTRL(ch) && !ISSPACE(ch) && termios_should_echo_ctrl_chars(tty)) {
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
    // i.e. if signals are enabled and the last character wasn't escaped (via VLNEXT):
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
        erase_char(tty, ch);
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

      if (termios_has_vlnext(tty, ch) && termios_extended_proc_enabled(tty)) {
        tty->flags |= TTY_HAS_LNEXT;
        if (termios_should_echo_ctrl_chars(tty)) {
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

        if (termios_any_char_resumes_output(tty)) {
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
    // TODO: Wakeup any process sleeping on select syscall
    // wakeup(&do_select);
  }
  wakeup(SLEEP_FN(&tty_read));
}

int
tty_read (inode_t* i, fd_t* fd_table, char* buffer, size_t count) {
  tty_t* tty;
  if (!(tty = tty_get(i->devnum))) {
    return -ENXIO;
  }

  // Only the foreground process group is allowed to read from the tty.
  // Check if that's not the case and handle accordingly.
  if (proc_current->controlling_tty == tty && proc_current->pgid != tty->pgid) {
    // In this case it's a background process trying to read, which isn't allowed.

    // If SIGTTIN is ignored (SIGHANDLER_IGN), or blocked, or the process group is orphaned
    // (no parent in same session to resume it if it stops), then sending SIGTTIN would hang the
    // process permanently, so we fail with an I/O error.
    if (proc_current->sigaction_table[SIGTTIN - 1].sa_handler == SIGHANDLER_IGN
        || proc_current->signal_blocked & (1 << (SIGTTIN - 1))
        || proc_is_orphaned_pgrp(proc_current->pgid)) {
      return -EIO;
    }

    // Otherwise, we send SIGTTIN effectively telling it to wait
    // TODO:
    // kill_pgrp(proc_current->pgid, SIGTTIN, KERNEL);

    return -ERESTART;
  }

  int           n        = 0;
  unsigned int  min_read = 0;
  unsigned char ch;

  while (count > 0) {
    // If it's a raw mode, we don't care about line buffering; we can put everything into the
    // buffer. Note we read from the read_q here.
    if (tty->kbd_state.mode == KBD_MODE_RAW || tty->kbd_state.mode == KBD_MODE_MEDRAW) {
      n = 0;
      while ((size_t)n < count) {
        if ((ch = charq_get_char(&tty->read_q))) {
          buffer[n++] = ch;
        } else {
          break;
        }
      }
      if (n) {
        break;
      }
    }

    // If we're in canonical mode, everything is line buffered.
    // Thus, we must check for a line delimiter.
    if (termios_is_canonical_mode(tty)) {
      if ((ch = LAST_CHAR(&tty->cooked_q))) {
        if (ch == '\n' || termios_is_eol_char(tty, ch) || termios_is_eof_char(tty, ch)
            || (termios_extended_proc_enabled(tty) && termios_is_eol2_char(tty, ch))) {
          tty->has_canon_ln = false;
          // EOF is not passed to the reader process, so we remove it. This is a POSIX thing.
          if (termios_is_eof_char(tty, ch)) {
            charq_unput_char(&tty->cooked_q);
          }

          // Copy all the characters from the cooked queue into the provided buffer
          while ((size_t)n < count) {
            if ((ch = charq_get_char(&tty->cooked_q))) {
              buffer[n++] = ch;
            } else {
              break;
            }
          }
          break;
        }
      }
    } else {
      timer_task_request_t ttr;

      // "Pure Timeout Mode"
      // If we're not in canonical mode, we return chars as soon as possible.
      // But we might have timeouts or min char limits to deal with...
      if (termios_has_noncanon_read_timeout(tty)) {
        unsigned int initial_ticks = kstat.ticks;

        if (!termios_has_min_read_batch_size(tty)) {
          // Convert VTIME (tenths of a second) into kernel ticks
          unsigned int timeout = tty->termios.c_cc[VTIME] * (HZ / 10);

          // Wait for input or timeout
          while (kstat.ticks - initial_ticks < timeout && !tty->cooked_q.size) {
            // Setup a timer task to resume the process later
            // TODO: Needed before NONBLOCK check?
            ttr.fn  = wait_vtime_wrapper;
            ttr.arg = (unsigned int)&tty->cooked_q;
            timer_task_add(&ttr, timeout);

            // If the file descriptor is non-blocking, we can't wait
            if (fd_table->flags & O_NONBLOCK) {
              return -EAGAIN;
            }

            if (sleep(SLEEP_FN(&tty_read), PROC_INTERRUPTIBLE)) {
              return -EINTR;
            }
          }

          while ((size_t)n < count) {
            if ((ch = charq_get_char(&tty->cooked_q))) {
              buffer[n++] = ch;
            } else {
              break;
            }
          }
          break;
        } else {
          // Else - we have a min batch size for reads

          // If we have a character buffered...
          if (tty->cooked_q.size > 0) {
            // If n (how many bytes we've read so far) is less than VMIN (or count, whichever is
            // smaller), go ahead and read.
            if ((size_t)n < min(tty->termios.c_cc[VMIN], count)) {
              ch          = charq_get_char(&tty->cooked_q);
              buffer[n++] = ch;
            }

            // If we've read at least VMIN characters (enough to satisfy the blocking read),
            // cancel any pending timeouts and return - we're done.
            if ((size_t)n >= min(tty->termios.c_cc[VMIN], count)) {
              timer_task_remove(&ttr);
              break;
            }

            // At this point, we've not read enough to fulfill VMIN.
            // We set a timer and sleep until we've more chars to read (or the timeout expires).
            unsigned int timeout = tty->termios.c_cc[VTIME] * (HZ / 10);
            ttr.fn               = wait_vtime_wrapper;
            ttr.arg              = (unsigned int)&tty->cooked_q;
            timer_task_add(&ttr, timeout);

            // Again, handle non-blocking mode
            if (fd_table->flags & O_NONBLOCK) {
              // TODO: dedicated retval; n still needs to be zero'd for atime check
              n = -EAGAIN;
              break;
            }
            if (sleep(SLEEP_FN(&tty_read), PROC_INTERRUPTIBLE)) {
              n = -EINTR;
              break;
            }

            // Check again after sleeping
            if (!tty->cooked_q.size) {
              break;
            }
            continue;
          }
        }
      } else {
        // No timeout. This part is basically "block indefinitely until VMIN chars read"

        // If we have input to read...
        if (tty->cooked_q.size > 0) {
          // Should we read more chars?
          if (min_read < tty->termios.c_cc[VMIN] || !termios_has_min_read_batch_size(tty)) {
            if ((size_t)n < count) {
              ch                = charq_get_char(&tty->cooked_q);
              buffer[n++]       = ch;
              tty->has_canon_ln = false;
            }
            min_read++;
          }
        }

        if (min_read >= tty->termios.c_cc[VMIN]) {
          break;
        }
      }
    }

    // TODO: fn
    if (fd_table->flags & O_NONBLOCK) {
      n = -EAGAIN;
      break;
    }
    if (sleep(SLEEP_FN(&tty_read), PROC_INTERRUPTIBLE)) {
      n = -EINTR;
      break;
    }
  }

  if (n) {
    // TODO:
    // i->i_atime = CURRENT_TIME;
  }
  return n;
}

int
tty_write (inode_t* i, fd_t* fd_table, const char* buffer, size_t count) {
  tty_t* tty;
  if (!(tty = tty_get(i->devnum))) {
    return -ENXIO;
  }

  if (proc_current->controlling_tty == tty && proc_current->pgid != tty->pgid) {
    if (termios_bg_proc_can_write_to_tty(tty)) {
      if (proc_current->sigaction_table[SIGTTIN - 1].sa_handler == SIGHANDLER_IGN
          || proc_current->signal_blocked & (1 << (SIGTTIN - 1))) {
        if (proc_is_orphaned_pgrp(proc_current->pgid)) {
          return -EIO;
        }

        // TODO:
        // kill_pgrp(proc_current->pgid, SIGTTOU, SIGSENDER_KERNEL);
        return -ERESTART;
      }
    }
  }

  int n = 0;
  while (true) {
    // If we've a pending and non-blocked signal, we need to try again later
    if (proc_current->signal_pending & ~proc_current->signal_blocked) {
      return -ERESTART;
    }

    // While we have bytes to write, send 'em to the tty
    while (count && (size_t)n < count) {
      unsigned char ch = *(buffer + n);
      if (opost(tty, ch) == RET_FAIL) {
        break;
      }
      n++;
    }
    tty->output(tty);

    if ((size_t)n == count) {
      break;
    }

    // If we're in non-blocking mode we must return now
    if (fd_table->flags & O_NONBLOCK) {
      n = -EAGAIN;
      break;
    }

    // If there's more data in the write queue after calling `output`, we got interrupted by another
    // write syscall
    if (tty->write_q.size > 0) {
      if (sleep(SLEEP_FN(&tty_write), PROC_INTERRUPTIBLE)) {
        n = -EINTR;
        break;
      }
    }

    if (needs_resched) {
      sched_run();
    }
  }

  if (n) {
    // TODO:
    // i->i_mtime = CURRENT_TIME;
  }
  return n;
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
      tty_table[num].devnum     = devnum;
      tty_table[num].open_count = 0;

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
