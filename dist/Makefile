ROOTPATH ?= $(shell readlink -f ..)
include $(ROOTPATH)/Makefile.config

.PHONY: all clean
.DELETE_ON_ERROR:

$(TARGET): $(BOOTPATH)/bin/bootblock $(KERNELPATH)/bin/kernel
	dd if=/dev/zero of=$(TARGET) count=10000
	dd if=$(BOOTPATH)/bin/bootblock of=$(TARGET) conv=notrunc
# TODO: placement
	dd if=$(KERNELPATH)/bin/kernel of=$(TARGET) seek=12 conv=notrunc

all: $(TARGET)

clean:
	rm -rf $(TARGET)
