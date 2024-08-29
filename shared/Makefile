ROOTPATH ?= $(shell readlink -f ..)
include $(ROOTPATH)/Makefile.config

.PHONY: all dirs clean
.DELETE_ON_ERROR:

BUILD_DIR			:= build
BIN_DIR				:= bin

SRC_DIR				:= src
INC_DIR 		 	:= include
CSRCS 				:= $(shell find -L $(SRC_DIR) -type f -name "*.c")
OBJS 				:= $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(CSRCS))

CFLAGS := -Wall -Wextra -Werror -Wno-missing-field-initializers \
  -Wno-unused-parameter -Wno-unused-function -Wno-unused-value                 \
	-m32 -I$(INC_DIR) -std=gnu99 -fno-strict-aliasing -nostdlib -ffreestanding -c \
	-O3

LDFLAGS := -nostdlib -O3
LDFLAGS += -m $(shell $(LD) -V | grep elf_i386 2>/dev/null | head -n 1)

$(OBJS): $(BUILD_DIR)/%.o : $(SRC_DIR)/%.c
	@mkdir -p $(shell dirname $@)
	$(CC) $(CFLAGS) -o $@ $<

all: dirs $(OBJS)

clean:
	rm -rf $(BUILD_DIR)
	rm -rf $(BIN_DIR)

dirs:
	@mkdir -p $(BUILD_DIR) $(BIN_DIR)
