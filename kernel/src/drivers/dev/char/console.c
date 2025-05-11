#include "drivers/dev/char/console.h"

#include "drivers/dev/char/tty.h"
#include "drivers/dev/char/vga.h"
#include "drivers/dev/char/video.h"
#include "drivers/dev/device.h"

static vconsole_t consoles[NUM_CONSOLES + 1];

static void
console_start (tty_t* tty) {
  vconsole_t* cons    = (vconsole_t*)tty->data;
  cons->screen_locked = false;
}

static void
console_stop (tty_t* tty) {
  vconsole_t* cons    = (vconsole_t*)tty->data;
  cons->screen_locked = true;
}

static void
console_delete_tab (tty_t* tty) {
  // vconsole_t* cons = (vconsole_t*)tty->data;
}

void
console_init (void) {
  bool video_using_vga(void);
  for (int num = 1; num <= NUM_CONSOLES; num++) {
    deviceno_t devnum = DEVICE_MKDEV(CONSOLES_MAJOR, num);

    if (tty_register(devnum) == RET_OK) {
      tty_t* tty = tty_get(devnum);
      tty->data  = (void*)&consoles[num];
    }
  }
}
