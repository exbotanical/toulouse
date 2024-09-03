include Makefile.config

.PHONY: shared bootblock dist clean
.DELETE_ON_ERROR:

shared:
	$(MAKE) -C shared all

bootblock: shared
	$(MAKE) -C boot all

kernel: shared
	$(MAKE) -C kernel all

dist: bootblock kernel
	$(MAKE) -C dist all

# Exit Qemu focus trap with ctrl+alt+g
test:
	$(MAKE) clean
	$(MAKE) dist
	$(QEMU) -drive file=$(DISTPATH)/$(TARGET),index=0,media=disk,format=raw
#-d int

clean:
	$(MAKE) -C shared clean
	$(MAKE) -C boot clean
	$(MAKE) -C kernel clean
	$(MAKE) -C dist clean
