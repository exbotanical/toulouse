ROOTPATH ?= $(shell readlink -f ..)
include $(ROOTPATH)/Makefile.config

.PHONY: all clean
.DELETE_ON_ERROR:

$(TARGET): $(BOOTPATH)/bin/bootblock
	dd if=/dev/zero of=$(TARGET) count=10000
	dd if=$(BOOTPATH)/bin/bootblock of=$(TARGET) conv=notrunc
# TODO: placement
# dd if=bin/kernel of=t.img seek=10 conv=notrunc

all: $(TARGET)

clean:
	rm -rf $(TARGET)
