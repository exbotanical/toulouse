ROOTPATH ?= $(shell readlink -f ..)
include $(ROOTPATH)/Makefile.config

.PHONY: all clean dirs
.DELETE_ON_ERROR:

BUILD_DIR    := build
BIN_DIR      := bin

SRC_DIR      := src
INC_DIR      := include
CSRCS        := $(shell find -L $(SRC_DIR) -type f -name "*.c")
OBJS         := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(CSRCS))
SHARED_OBJS  := $(shell find -L ../shared/ -type f -name "*.o")

CFLAGS       := -m32 -std=gnu99 -fno-strict-aliasing -nostdlib -ffreestanding -c -O3
CFLAGS       += $(C_STRICTMODE_FLAGS)
CFLAGS       += -I$(INC_DIR)

LDFLAGS      := -nostdlib -O3
LDFLAGS      += -m $(shell $(LD) -V | grep elf_i386 2>/dev/null | head -n 1)

build/main.o: src/main.S
	$(CC) $(CFLAGS) -c $< -o $@

$(SHARED_OBJS):
	$(MAKE) -C ../shared all

$(OBJS): $(BUILD_DIR)/%.o : $(SRC_DIR)/%.c
	@mkdir -p $(shell dirname $@)
	$(CC) $(CFLAGS) -I../shared/include -o $@ $<

all: dirs $(OBJS) $(SHARED_OBJS) build/main.o
	$(LD) $(LDFLAGS) -T linker.ld build/main.o $(OBJS) $(SHARED_OBJS) -o bin/kernel

clean:
	rm -rf $(BUILD_DIR)
	rm -rf $(BIN_DIR)

dirs:
	@mkdir -p $(BUILD_DIR) $(BIN_DIR)
