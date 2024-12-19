ROOTPATH ?= $(shell readlink -f ..)
include $(ROOTPATH)/Makefile.config

.PHONY: all clean grub
.DELETE_ON_ERROR:

GRUB_BASE_DIR := bin/boot

$(TARGET): $(BOOTPATH)/bin/bootblock $(KERNELPATH)/bin/kernel
	dd if=/dev/zero of=$(TARGET) count=10000
	dd if=$(BOOTPATH)/bin/bootblock of=$(TARGET) conv=notrunc
# TODO: placement
	dd if=$(KERNELPATH)/bin/kernel of=$(TARGET) seek=12 conv=notrunc

all: $(TARGET)

grub:
	mkdir -p $(GRUB_BASE_DIR)/grub
	cp $(KERNELPATH)/bin/kernel $(GRUB_BASE_DIR)/kernel
	cp grub/grub.cfg $(GRUB_BASE_DIR)/grub/grub.cfg
	grub-mkrescue -o $(TARGET) bin
# TODO: separate run target
	$(QEMU) -cdrom $(TARGET) -vga std

clean:
	rm -rf $(TARGET)
	rm -rf $(GRUB_BASE_DIR)
