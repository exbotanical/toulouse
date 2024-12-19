include Makefile.config

.PHONY: shared bootblock dist clean grub
.DELETE_ON_ERROR:

DEBUG_SYM_FILE  := kernel.sym
QEMU_LOG_FILE   := qemu_log.txt
QEMU_ARGS       := -d int -no-reboot -D $(QEMU_LOG_FILE)
QEMU_DRIVE_CONF := file=$(DISTPATH)/$(TARGET),index=0,media=disk,format=raw

shared:
	$(MAKE) -C shared all

bootblock: shared
	$(MAKE) -C boot all

kernel: shared
	$(MAKE) -C kernel all

dist: bootblock kernel
	$(MAKE) -C dist all

# TODO: flag instead of target
grub: kernel
	$(MAKE) -C dist grub

# Exit Qemu focus trap with ctrl+alt+g
dev:
	$(MAKE) clean
	$(MAKE) dist
	$(QEMU) -drive $(QEMU_DRIVE_CONF) $(QEMU_ARGS)

debug:
	$(MAKE) clean
	$(MAKE) dist DEBUG=1
	$(OBJCOPY) --only-keep-debug $(KERNELPATH)/bin/kernel $(DEBUG_SYM_FILE)
	$(QEMU) -drive $(QEMU_DRIVE_CONF) $(QEMU_ARGS) -s -S &
	$(DEBUGGER) -x debug.gdb

clean:
	$(MAKE) -C shared clean
	$(MAKE) -C boot clean
	$(MAKE) -C kernel clean
	$(MAKE) -C dist clean
	$(shell rm $(DEBUG_SYM_FILE) &> /dev/null)
	$(shell rm $(QEMU_LOG_FILE) &> /dev/null)
