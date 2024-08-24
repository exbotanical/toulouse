include Makefile.config

.PHONY: bootblock dist clean
.DELETE_ON_ERROR:

bootblock:
	$(MAKE) -C boot all

dist: bootblock
	$(MAKE) -C dist all

# Exit Qemu focus trap with ctrl+alt+g
test:
	$(MAKE) clean
	$(MAKE) dist
	$(QEMU) -drive file=$(DISTPATH)/$(TARGET),index=0,media=disk,format=raw

clean:
	$(MAKE) -C boot clean
	$(MAKE) -C dist clean
