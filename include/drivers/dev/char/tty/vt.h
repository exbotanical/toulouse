#ifndef DRIVER_DEV_CHAR_VT_H
#define DRIVER_DEV_CHAR_VT_H

#include "drivers/dev/char/tty/tty.h"

// 0x56 is 'V'
typedef enum {
  VT_OPENQRY      = 0x5600, /* Find available vt */
  VT_GETMODE      = 0x5601, /* Get mode of active vt */
  VT_SETMODE      = 0x5602, /* Set mode of active vt */
  VT_GETSTATE     = 0x5603, /* get global vt state info */
  VT_SENDSIG      = 0x5604, /* Signal to send to bitmask of vts */
  VT_RELDISP      = 0x5605, /* Release display */
  VT_ACTIVATE     = 0x5606, /* Make vt active */
  VT_WAITACTIVE   = 0x5607, /* Wait for vt active */
  VT_DISALLOCATE  = 0x5608, /* Free memory associated to vt */
  VT_RESIZE       = 0x5609, /* Set kernel's idea of screensize */
  VT_RESIZEX      = 0x560A, /* Set kernel's idea of screensize + more */
  VT_LOCKSWITCH   = 0x560B, /* Disallow vt switching */
  VT_UNLOCKSWITCH = 0x560C, /* Allow vt switching */
} vt_action;

typedef enum {
  KDGETLED    = 0x4B31, /* Return current led state */
  KDSETLED    = 0x4B32, /* Set led state (lights, not flags) */
  KDGKBTYPE   = 0x4B33, /* Get keyboard type */
  KDADDIO     = 0x4B34, /* Add I/O port as valid */
  KDDELIO     = 0x4B35, /* Del I/O port as valid */
  KDENABIO    = 0x4B36, /* Enable I/O to video board */
  KDDISABIO   = 0x4B37, /* Disable I/O to video board */
  KDSETMODE   = 0x4B3A, /* Set text/graphics mode */
  KDGETMODE   = 0x4B3B, /* Get current mode */
  KDMAPDISP   = 0x4B3C, /* Map display into address space */
  KDUNMAPDISP = 0x4B3D, /* Unmap display from address space */
} kd_ioctl;

typedef enum {
  /**
   * The kernel switches terminals automatically without involving any userspace logic
   */
  VT_AUTO    = 0x00,

  /**
   * Userspace process handles switching. Kernel sends signals (relsig/acqsig) to notify the
   * process and waits for an acknowledgment (using VT_ACKACQ) before completing the switch
   */
  VT_PROCESS = 0x01
} vt_mode_mod;

/**
 * Defines how virtual terminal switching is handled, either automatically by the kernel or manually
 * by a userspace process
 */
typedef struct {
  vt_mode_mod mode;  /* VT mode */
  char        waitv; /* If non-zero, write() operations block until the VT becomes active */
  short int relsig;  /* Signal to raise on release req. If in VT_PROCESS mode, the kernel sends this
                        signal to the controlling process when the VT is about to be switched away */
  short int acqsig;  /* Signal sent to the process when the VT is acquired again */
  short int frsig;   /* Unused; legacy field, always zero */
} vt_mode_t;

typedef struct {
  unsigned short int v_active; /* The number of the currently active virtual terminal */
  unsigned short int v_signal; /* The signal number that should be sent to the process managing the
                                  VT when switching occurs */
  unsigned short int v_state;  /* A bitmask representing which VTs are currently open (in use) */
} vt_stat_t;

/**
 * Apply I/O controls to the tty that are specific to vts.
 * See: https://man7.org/linux/man-pages/man2/ioctl_vt.2.html
 *
 * @return int 0 or an errno
 *
 * TODO: KDGKBLED and KDSKBLED; convert 'scrlock, numlock, capslock' into led_flags.
 */
int vt_ioctl(tty_t *tty, int cmd, unsigned int arg);

#endif /* DRIVER_DEV_CHAR_VT_H */
