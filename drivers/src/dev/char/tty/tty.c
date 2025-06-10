#include "drivers/dev/char/tty/tty.h"

#include "drivers/dev/char/console/console.h"
#include "drivers/dev/char/tmpcon.h"
#include "drivers/dev/char/tty/termios.h"
#include "drivers/dev/char/tty/vt.h"
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
#include "proc/signal.h"  // TODO: Move
#include "proc/sleep.h"
#include "proc/vma.h"
#include "syscall/ioctl.h"

tty_t tty_table[NUM_TTYS];

static void
wait_vtime_wrapper (unsigned int arg) {
  unsigned int *fn = (unsigned int *)arg;

  wakeup(fn);
}

static void
set_termios (tty_t *tty, termios_t *new_termios) {
  kmemcpy(&tty->termios, new_termios, sizeof(termios_t));
  if (tty->set_termios) {
    tty->set_termios(tty);
  }
}

/**
 * Performs output processing on the write queue
 */
static retval_t
opost (tty_t *tty, unsigned char ch) {
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
out_char (struct tty *tty, unsigned char ch) {
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
erase_char (tty_t *tty, unsigned char char_to_erase) {
  unsigned char ch;

  if (termios_should_erase_char(tty, char_to_erase)) {
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

  if (termios_should_erase_word(tty, char_to_erase)) {
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

  if (termios_should_kill_proc(tty, char_to_erase)) {
    while (tty->cooked_q.size > 0) {
      erase_char(tty, tty->termios.c_cc[VERASE]);
    }
    if (termios_should_echo_nl_on_kill(tty) && !termios_should_visual_erase(tty)) {
      charq_put_char(&tty->write_q, '\n');
    }
  }
}

void
tty_disassociate_ctty (tty_t *tty) {
  if (!tty) {
    return;
  }

  // This tty is no longer the controlling tty of any session
  tty->pgid = tty->sid = 0;

  // Clear the controlling tty for all processes in the same SID
  proc_t *p            = proc_list->next;
  while (p) {
    if (p->sid == proc_current->sid) {
      p->ctty = NULL;
    }
    p = p->next;
  }

  sig_kill_pgrp(proc_current->pgid, SIGHUP, SIGSENDER_KERNEL);
  sig_kill_pgrp(proc_current->pgid, SIGCONT, SIGSENDER_KERNEL);
}

long long int
tty_llseek (inode_t *i, long long int offset) {
  return -ESPIPE;
}

int
tty_ioctl (inode_t *i, int cmd, unsigned int arg) {
  tty_t *tty;
  if (!(tty = tty_get(i->devnum))) {
    // printk("%s(): fak! (%x)\n", __FUNCTION__, i->devnum);
    return -ENXIO;
  }

  int errno;
  switch (cmd) {
    /* Get the current serial port settings. */
    // Retrieve the current tty params and store in a termios structure pointed to by the void
    // pointer argument
    case TCGETS: {
      if ((errno = verify_address(VERIFY_WRITE, (void *)arg, sizeof(termios_t)))) {
        return errno;
      }

      kmemcpy((termios_t *)arg, &tty->termios, sizeof(termios_t));
      break;
    }

    /* Set the current serial port settings. */
    // Set the current tty using the parameters found in the provided termios structure (void
    // pointer argument)
    case TCSETS: {
      if ((errno = verify_address(VERIFY_READ, (void *)arg, sizeof(termios_t)))) {
        return errno;
      }

      set_termios(tty, (termios_t *)arg);
      break;
    }

    /* Allow the output buffer to drain, and set the current serial port settings. */
    // Same as TCSETS with one exception: doesn't take effect until all chars queued for output
    // have been transmitted
    case TCSETSW: {
      if ((errno = verify_address(VERIFY_READ, (void *)arg, sizeof(termios_t)))) {
        return errno;
      }

      while (tty->write_q.count) {
        if (sleep(SLEEP_FN(&tty_write), PROC_INTERRUPTIBLE)) {
          return -EINTR;
        }

        sched_run();
      }

      set_termios(tty, (termios_t *)arg);
      break;
    }

    /* Allow output buffer to drain, discard pending input, and set current serial port settings. */
    // Same as TCSETSW except all chars queued for input are discarded
    case TCSETSF: {
      if ((errno = verify_address(VERIFY_READ, (void *)arg, sizeof(termios_t)))) {
        return errno;
      }

      while (tty->write_q.count) {
        if (sleep(SLEEP_FN(&tty_write), PROC_INTERRUPTIBLE)) {
          return -EINTR;
        }

        sched_run();
      }

      set_termios(tty, (termios_t *)arg);
      charq_flush(&tty->read_q);
      break;
    }

    /* Retrieve current tty params and store in a termio structure pointed to by the void ptr arg */
    case TCGETA: {
      if ((errno = verify_address(VERIFY_WRITE, (void *)arg, sizeof(termio_t)))) {
        return errno;
      }

      termios_to_termio(&tty->termios, (termio_t *)arg);
      break;
    }

    /* Set current tty using the params found in the provided termio structure (void ptr arg) */
    case TCSETA: {
      if ((errno = verify_address(VERIFY_READ, (void *)arg, sizeof(termio_t)))) {
        return errno;
      }

      set_termio(tty, (termio_t *)arg);
      break;
    }

    /* Same as TCSETSW but the legacy termio version */
    case TCSETAW: {
      if ((errno = verify_address(VERIFY_READ, (void *)arg, sizeof(termio_t)))) {
        return errno;
      }

      while (tty->write_q.count) {
        if (sleep(SLEEP_FN(&tty_write), PROC_INTERRUPTIBLE)) {
          return -EINTR;
        }

        sched_run();
      }

      set_termio(tty, (termio_t *)arg);
      break;
    }

    /* Legacy (termio) version of TCSETSF */
    case TCSETAF: {
      if ((errno = verify_address(VERIFY_READ, (void *)arg, sizeof(termio_t)))) {
        return errno;
      }

      while (tty->write_q.count) {
        if (sleep(SLEEP_FN(&tty_write), PROC_INTERRUPTIBLE)) {
          return -EINTR;
        }

        sched_run();
      }

      set_termio(tty, (termio_t *)arg);
      charq_flush(&tty->read_q);
      break;
    }

    /* Software flow control. */
    // Perform start / stop control
    case TCXONC: {
      switch ((tcflow_action)arg) {
        case TCFLOW_ACT_OOFF: {
          tty->stop(tty);
          break;
        }

        case TCFLOW_ACT_OON: {
          tty->start(tty);
          break;
        }

        default: {
          return -EINVAL;
        }
      }

      break;
    }

    case TCFLSH: {
      switch (arg) {
        case TCFLUSH_QSEL_TCIFLUSH: {
          charq_flush(&tty->read_q);
          charq_flush(&tty->cooked_q);
          break;
        }

        case TCFLUSH_QSEL_TCOFLUSH: {
          charq_flush(&tty->write_q);
          break;
        }

        case TCFLUSH_QSEL_TCIOFLUSH: {
          charq_flush(&tty->read_q);
          charq_flush(&tty->cooked_q);
          charq_flush(&tty->write_q);
          break;
        }

        default: {
          return -EINVAL;
        }
      }

      break;
    }

    // Make the given terminal the controlling terminal of the calling process. The calling process
    // must be a session leader and not have a controlling terminal already. For this case, arg
    // should be specified as zero.
    case TIOCSCTTY: {
      if (PROC_SESSION_LEADER(proc_current) && (proc_current->sid == tty->sid)) {
        return 0;
      }

      if (!PROC_SESSION_LEADER(proc_current) || proc_current->ctty) {
        return -EPERM;
      }

      if (tty->sid) {
        if ((arg == 1) && PROC_IS_SUPERUSER) {
          proc_t *p = proc_list->next;
          while (p) {
            if (p->ctty == tty) {
              p->ctty = NULL;
            }
            p = p->next;
          }
        } else {
          return -EPERM;
        }
      }

      proc_current->ctty = tty;
      tty->sid           = current->sid;
      tty->pgid          = current->pgid;

      break;
    }

    // Get the process group id of the foreground process group on this terminal.
    case TIOCGPGRP: {
      if ((errno = verify_address(VERIFY_WRITE, (void *)arg, sizeof(int)))) {
        return errno;
      }

      kmemcpy((void *)arg, &tty->pgid, sizeof(int));
      break;
    }

    // Set the foreground process group ID of this terminal.
    case TIOCSPGRP: {
      if (arg < 1) {
        return -EINVAL;
      }

      if ((errno = verify_address(VERIFY_READ, (void *)arg, sizeof(int)))) {
        return errno;
      }

      kmemcpy(&tty->pgid, (void *)arg, sizeof(int));
      break;
    }

    // TODO: case TIOCSID:

    /* Get window size. */
    // The terminal driver's terminal size is stored in the winsize_t (the void pointer arg)
    case TIOCGWINSZ: {
      if ((errno = verify_address(VERIFY_WRITE, (void *)arg, sizeof(winsize_t)))) {
        return errno;
      }

      kmemcpy((void *)arg, &tty->winsize, sizeof(winsize_t));
      break;
    }

    /* Set window size. */
    case TIOCSWINSZ: {
      if ((errno = verify_address(VERIFY_READ, (void *)arg, sizeof(winsize_t)))) {
        return errno;
      }

      winsize_t *ws          = (winsize_t *)arg;
      short int  changed     = 0;
      // clang-format off
      if (
        (tty->winsize.ws_row != ws->ws_row)
        || (tty->winsize.ws_col != ws->ws_col)
        || (tty->winsize.ws_xpixel != ws->ws_xpixel)
        || (tty->winsize.ws_ypixel != ws->ws_ypixel)
      ) {
        changed = 1;
      }
      // clang-format on

      tty->winsize.ws_row    = ws->ws_row;
      tty->winsize.ws_col    = ws->ws_col;
      tty->winsize.ws_xpixel = ws->ws_xpixel;
      tty->winsize.ws_ypixel = ws->ws_ypixel;

      if (changed) {
        sig_kill_pgrp(tty->pgid, SIGWINCH, SIGSENDER_KERNEL);
      }

      break;
    }

    /* Detach the calling process from its controlling terminal. */
    case TIOCNOTTY: {
      if (proc_current->ctty != tty) {
        return -ENOTTY;
      }

      if (PROC_SESSION_LEADER(proc_current)) {
        tty_disassociate_ctty(tty);
      }

      break;
    }

    case TIOCLINUX: {
      if ((errno = verify_address(VERIFY_READ, (void *)arg, sizeof(unsigned char)))) {
        return errno;
      }

      int val = *(unsigned char *)arg;
      switch (val) {
        case 12: {
          return current_console;
        }

        default: {
          return -EINVAL;
        }
      }

      break;
    }

    default: {
      return vt_ioctl(tty, cmd, arg);
    }
  }

  return 0;
}

void
tty_cook_input (tty_t *tty) {
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
          sig_kill_pgrp(tty->pgid, SIGINT, SIGSENDER_KERNEL);
        }

        break;
      }

      // Ctrl-\ → VQUIT: send SIGQUIT
      if (termios_is_quit_char(tty, ch)) {
        if (tty->pgid > 0) {
          sig_kill_pgrp(tty->pgid, SIGQUIT, SIGSENDER_KERNEL);
        }
        break;
      }

      // Ctrl-Z → VSUSP: send SIGTSTP.
      if (termios_is_suspend_char(tty, ch)) {
        if (tty->pgid > 0) {
          sig_kill_pgrp(tty->pgid, SIGTSTP, SIGSENDER_KERNEL);
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

        cblock_t *cb = tty->cooked_q.head;
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
tty_read (inode_t *i, fd_t *fd_table, char *buffer, size_t count) {
  tty_t *tty;
  if (!(tty = tty_get(i->devnum))) {
    return -ENXIO;
  }

  // Only the foreground process group is allowed to read from the tty.
  // Check if that's not the case and handle accordingly.
  if (proc_current->ctty == tty && proc_current->pgid != tty->pgid) {
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
    sig_kill_pgrp(proc_current->pgid, SIGTTIN, SIGSENDER_KERNEL);

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
tty_write (inode_t *i, fd_t *fd_table, const char *buffer, size_t count) {
  tty_t *tty;
  if (!(tty = tty_get(i->devnum))) {
    return -ENXIO;
  }

  if (proc_current->ctty == tty && proc_current->pgid != tty->pgid) {
    if (termios_bg_proc_can_write_to_tty(tty)) {
      if (proc_current->sigaction_table[SIGTTIN - 1].sa_handler == SIGHANDLER_IGN
          || proc_current->signal_blocked & (1 << (SIGTTIN - 1))) {
        if (proc_is_orphaned_pgrp(proc_current->pgid)) {
          return -EIO;
        }

        sig_kill_pgrp(proc_current->pgid, SIGTTOU, SIGSENDER_KERNEL);
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

int
tty_open (inode_t *i, fd_t *fd_table) {
  int no_ctty = fd_table->flags & O_NOCTTY;

  if (MAJOR(i->devnum) == SYSCON_MAJOR && MINOR(i->devnum) == 0) {
    if (!proc_current->ctty) {
      return -ENXIO;
    }
  }

  if (MAJOR(i->devnum) == VCONSOLE_MAJOR && MINOR(i->devnum) == 0) {
    no_ctty = 1;
  }

  tty_t *tty;
  if (!(tty = tty_get(i->devnum))) {
    // printk("%s(): fak! (%x)\n", __FUNCTION__, i->devnum);fwf
    return -ENXIO;
  }

  if (tty->open) {
    int errno;
    if ((errno = tty->open(tty)) < 0) {
      return errno;
    }
  }

  tty->open_count++;
  tty->column = 0;

  if (PROC_SESSION_LEADER(proc_current) && !proc_current->ctty && !no_ctty && !tty->sid) {
    proc_current->ctty = tty;
    tty->sid           = proc_current->sid;
    tty->pgid          = proc_current->pgid;
  }

  return 0;
}

int
tty_close (inode_t *i, fd_t *fd_table) {
  tty_t *tty;
  if (!(tty = tty_get(i->devnum))) {
    // printk("%s(): fak! (%x)\n", __FUNCTION__, i->devnum);
    return -ENXIO;
  }

  if (tty->close) {
    int errno;
    if ((errno = tty->close(tty)) < 0) {
      return errno;
    }
  }

  tty->open_count--;
  if (!tty->open_count) {
    termios_reset(tty);
    tty->pgid = tty->sid = 0;

    // This tty is no longer the controlling tty of any process
    proc_t *p            = proc_list->next;
    while (p) {
      if (p->ctty == tty) {
        p->ctty = NULL;
      }
      p = p->next;
    }
  }

  return 0;
}

tty_t *
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
    if (!proc_current->ctty) {
      return NULL;
    }

    devnum = proc_current->ctty->devnum;
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
tty_select (inode_t *i, fs_tty_select_flag flag) {
  tty_t *tty;
  if (!(tty = tty_get(i->devnum))) {
    // printk("%s(): fak! (%x)\n", __FUNCTION__, i->devnum);
    return RET_OK;
  }

  switch (flag) {
    case FS_TTY_SELECT_FLAG_R: {
      if (tty->cooked_q.size > 0) {
        if (!termios_is_canonical_mode(tty)
            || (termios_is_canonical_mode(tty) && tty->has_canon_ln)) {
          return RET_FAIL;
        }
      }
      break;
    }

    case FS_TTY_SELECT_FLAG_W: {
      if (!tty->write_q.size) {
        return RET_FAIL;
      }
      break;
    }
  }

  return RET_OK;
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
