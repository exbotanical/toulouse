ROOTPATH ?= $(shell readlink -f ..)
include $(ROOTPATH)/Makefile.config

.PHONY: all clean dirs
.DELETE_ON_ERROR:

BUILD_DIR  := build

SRC_DIR    := src
INC_DIR    := include
CSRCS      := $(shell find -L $(SRC_DIR) -type f -name "*.c")
OBJS       := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(CSRCS))

CFLAGS     := -m32 -std=gnu99 -fno-strict-aliasing -nostdlib -ffreestanding -c -O3
CFLAGS     += $(C_STRICTMODE_FLAGS)
CFLAGS     += -I$(INC_DIR)

LDFLAGS    := -nostdlib -O3
LDFLAGS    += -m $(shell $(LD) -V | grep elf_i386 2>/dev/null | head -n 1)

$(OBJS): $(BUILD_DIR)/%.o : $(SRC_DIR)/%.c
	@mkdir -p $(shell dirname $@)
	$(CC) $(CFLAGS) -o $@ $<

all: dirs $(OBJS)

clean:
	rm -rf $(BUILD_DIR)

dirs:
	@mkdir -p $(BUILD_DIR)
