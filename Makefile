include Makefile.config

.PHONY: lib drivers bootblock kernel dist grub dev debug test clean
.DELETE_ON_ERROR:

DEBUG_SYM_FILE  := kernel.sym
QEMU_LOG_FILE   := qemu_log.txt
QEMU_ARGS       := -d int -no-reboot -D $(QEMU_LOG_FILE)
QEMU_DRIVE_CONF := file=$(DISTPATH)/$(TARGET),index=0,media=disk,format=raw

# Builds the library
lib:
	$(MAKE) -C lib all

# Builds the drivers
drivers: lib
	$(MAKE) -C drivers all

# Builds the bootblock (2-stage bootloader)
bootblock: drivers lib
	$(MAKE) -C boot all

# Builds the kernel core.
kernel: drivers lib
	$(MAKE) -C kernel all

# Packages the kernel and bootloader into a bootable image.
dist: bootblock kernel
	$(MAKE) -C dist all

# Packages the kernel into a GRUB2 bootable image.
# TODO: flag instead of target - currently also runs the kernel in QEMU for dev.
grub: kernel
	$(MAKE) -C dist grub

# Runs the kernel in QEMU using the local 2-stage bootloader.
dev:
	$(MAKE) clean
	$(MAKE) dist
# Exit Qemu focus trap with ctrl+alt+g
	$(QEMU) -drive $(QEMU_DRIVE_CONF) $(QEMU_ARGS)

# Runs the kernel in QEMU using the local 2-stage bootloader.
# Enables debug mode and attaches gdb to the running emu instance.
debug:
	$(MAKE) clean
	$(MAKE) dist DEBUG=1
	$(OBJCOPY) --only-keep-debug $(KERNELPATH)/bin/kernel $(DEBUG_SYM_FILE)
	$(QEMU) -drive $(QEMU_DRIVE_CONF) $(QEMU_ARGS) -s -S &
	$(DEBUGGER) -x debug.gdb

# Runs all tests across all packages.
test:
	$(MAKE) -C kernel unit_test

# Removes all build artifacts.
clean:
	$(MAKE) -C lib clean
	$(MAKE) -C drivers clean
	$(MAKE) -C boot clean
	$(MAKE) -C kernel clean
	$(MAKE) -C dist clean
	$(shell rm $(DEBUG_SYM_FILE) &> /dev/null)
	$(shell rm $(QEMU_LOG_FILE) &> /dev/null)
