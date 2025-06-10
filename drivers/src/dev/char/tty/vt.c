#include "drivers/dev/char/tty/vt.h"

#include "drivers/dev/char/console/console.h"
#include "drivers/dev/char/keyboard.h"
#include "drivers/dev/char/keymap.h"
#include "drivers/dev/char/ps2.h"
#include "drivers/dev/char/video.h"
#include "drivers/dev/kd.h"
#include "lib/errno.h"
#include "lib/string.h"
#include "proc/proc.h"
#include "proc/vma.h"

int
vt_ioctl (tty_t *tty, int cmd, unsigned int arg) {
  // Only virtual consoles support the following ioctl commands; exit early if the tty isn't a vc
  if (MAJOR(tty->dev) != VCONSOLE_MAJOR) {
    return -ENXIO;
  }

  vconsole_t *vc = (vconsole_t *)tty->data;

  switch (cmd) {
    /* Get state of LEDs. */
    case KDGETLED: {
      if ((errno = verify_address(VERIFY_WRITE, (void *)arg, sizeof(unsigned char)))) {
        return errno;
      }

      memset_b((void *)arg, vc->led_status, sizeof(char));
      break;
    }

    /* Set state of LEDs. */
    case KDSETLED: {
      if (arg > 7) {
        return -EINVAL;
      }

      vc->led_status = arg;
      ps2_set_leds(vc->led_status);
      break;
    }

    /* Set text/graphics mode. */
    case KDSETMODE: {
      if (arg != KD_TEXT && arg != KD_GRAPHICS) {
        return -EINVAL;
      }

      if (vc->vc_mode != arg) {
        vc->vc_mode = arg;
        if (arg == KD_GRAPHICS) {
          video.clear_screen(vc);
        } else {
          vconsole_unclear_screen(vc);
        }
      }

      break;
    }

    /* Get text/graphics mode. */
    case KDGETMODE: {
      if ((errno = verify_address(VERIFY_WRITE, (void *)arg, sizeof(unsigned char)))) {
        return errno;
      }

      memset_b((void *)arg, vc->vc_mode, sizeof(char));
      break;
    }

    /* Gets current keyboard mode. */
    case KDGKBMODE: {
      if ((errno = verify_address(VERIFY_WRITE, (void *)arg, sizeof(unsigned char)))) {
        return errno;
      }

      memset_b((void *)arg, tty->kbd.mode, sizeof(unsigned char));
      break;
    }

    /* Sets current keyboard mode. */
    case KDSKBMODE: {
      if (arg != KBD_MODE_RAW && arg != KBD_MODE_XLATE && arg != KBD_MODE_MEDRAW) {
        arg = KBD_MODE_XLATE;
      }

      tty->kbd.mode = arg;
      charq_flush(&tty->read_q);
      break;
    }

    /* Sets one entry in translation table. */
    case KDSKBENT: {
      kbentry_t *k = (kbentry_t *)arg;
      if ((errno = verify_address(VERIFY_WRITE, (void *)k, sizeof(kbentry_t)))) {
        return errno;
      }

      if (k->kb_table < NUM_MODIFIERS) {
        if (k->kb_index < NUM_SCODES) {
          keymap[(k->kb_index * NUM_MODIFIERS) + k->kb_table] = k->kb_value;
        } else {
          return -EINVAL;
        }
      } else {
        // printk("%s(): kb_table value '%d' not supported.\n", __FUNCTION__, k->kb_table);
        return -EINVAL;
      }

      break;
    }

    /* Returns the first available (non-opened) console. */
    case VT_OPENQRY: {
      int *val = (int *)arg;
      if ((errno = verify_address(VERIFY_WRITE, (void *)arg, sizeof(unsigned int)))) {
        return errno;
      }

      int n = 0;
      for (n = 1; n < NUM_VCONSOLES + 1; n++) {
        tty = get_tty(MKDEV(VCONSOLE_MAJOR, n));
        if (!tty->open_count) {
          break;
        }
      }

      *val = (n < NUM_VCONSOLES + 1 ? n : -1);

      break;
    }

    /* Get mode of active vt. */
    case VT_GETMODE: {
      vt_mode_t *vt_mode = (vt_mode_t *)arg;
      if ((errno = verify_address(VERIFY_WRITE, (void *)vt_mode, sizeof(vt_mode_t)))) {
        return errno;
      }

      kmemcpy(vt_mode, &vc->vt_mode, sizeof(vt_mode_t));
      break;
    }

    /* Set mode of active vt. */
    case VT_SETMODE: {
      vt_mode_t *vt_mode = (vt_mode_t *)arg;
      if ((errno = verify_address(VERIFY_READ, (void *)vt_mode, sizeof(vt_mode_t)))) {
        return errno;
      }

      if (vt_mode->mode != VT_AUTO && vt_mode->mode != VT_PROCESS) {
        return -EINVAL;
      }

      kmemcpy(&vc->vt_mode, vt_mode, sizeof(vt_mode_t));
      vc->vt_mode.frsig = 0;  // ignored
      tty->pid          = proc_current->pid;
      vc->switch_tty    = 0;

      break;
    }

    /* Get global vt state info. */
    case VT_GETSTATE: {
      vt_stat_t *vt_stat = (vt_stat_t *)arg;
      if ((errno = verify_address(VERIFY_WRITE, (void *)vt_stat, sizeof(vt_stat_t)))) {
        return errno;
      }

      vt_stat->v_active = current_console;
      vt_stat->v_state  = 1;  // /dev/tty0 is always open

      for (int n = 1; n < NUM_VCONSOLES + 1; n++) {
        tty = get_tty(MKDEV(VCONSOLE_MAJOR, n));
        if (tty->open_count) {
          vt_stat->v_state |= (1 << n);
        }
      }

      break;
    }

    /* Release a display. */
    case VT_RELDISP: {
      if (vc->vt_mode.mode != VT_PROCESS) {
        return -EINVAL;
      }

      if (vc->switch_tty < 0) {
        if (arg != VT_ACKACQ) {
          return -EINVAL;
        }
      } else {
        if (arg) {
          int switch_tty = vc->switch_tty;
          vc->switch_tty = -1;
          vconsole_select_final(switch_tty);
        } else {
          vc->switch_tty = -1;
        }
      }
      break;
    }

    /* Switch to vt `arg` */
    case VT_ACTIVATE: {
      if (current_console == MINOR(tty->dev) || PROC_IS_SUPERUSER) {
        if (!arg || arg > NUM_VCONSOLES) {
          return -ENXIO;
        }

        vconsole_select(--arg);
      } else {
        return -EPERM;
      }

      break;
    }

    /* Wait until vt `arg` has been activated. */
    case VT_WAITACTIVE: {
      if (current_console == MINOR(tty->dev)) {
        break;
      }

      if (!arg || arg > NUM_VCONSOLES) {
        return -ENXIO;
      }

      // printk("ACTIVATING another tty!! (cmd = 0x%x)\n", cmd);
      break;
    }

    default: {
      return -EINVAL;
    }
  }

  return 0;
}
