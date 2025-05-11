ROOTPATH ?= $(shell readlink -f ..)
include $(ROOTPATH)/Makefile.config

.PHONY: all clean dirs linker unit_test unit_test_dev
.DELETE_ON_ERROR:

UNIT_TARGET  := unit_test
LD_TARGET    := $(shell mktemp)

BUILD_DIR    := build
BIN_DIR      := bin

SRC_DIR      := src
INC_DIR      := include
DEPS_DIR      := deps
TEST_DIR      := t

CSRCS        := $(shell find -L $(SRC_DIR) -type f -name "*.c")
ASM_SRCS     := $(shell find -L $(SRC_DIR) -type f -name "*.S")

OBJS         := $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(CSRCS))
ASM_OBJS     := $(patsubst $(SRC_DIR)/%.S, $(BUILD_DIR)/%.asm.o, $(ASM_SRCS))
SHARED_OBJS  := $(shell find -L ../shared/ -type f -name "*.o")
TEST_DEPS    := $(wildcard $(DEPS_DIR)/libtap/*.c)
TEST_SRCS    := $(shell find -L $(TEST_DIR) -type f -name "*.c")
TEST_ASM_OBJS    := $(filter-out $(BUILD_DIR)/loader.asm.o, $(ASM_OBJS))

CFLAGS       := -m32 -std=gnu99 -fno-strict-aliasing -nostdlib -ffreestanding \
	-c -mno-sse
# Why no SSE - see: https://forum.osdev.org/viewtopic.php?p=234155
# TODO: -fstack-protector-all (need to link)

INC          := -I$(INC_DIR) -I../shared/include/
CFLAGS       += $(C_STRICTMODE_FLAGS)
CFLAGS       += $(INC)

LDFLAGS      := -nostdlib
LDFLAGS      += -m $(shell $(LD) -V | grep elf_i386 2>/dev/null | head -n 1)

ifdef DEBUG
	CFLAGS  += -O0 -g -DDEBUG
	LDFLAGS += -O0 -g
else
	CFLAGS  += -O3
	LDFLAGS += -O3
endif

$(SHARED_OBJS):
	$(MAKE) -C ../shared all

$(OBJS): $(BUILD_DIR)/%.o : $(SRC_DIR)/%.c
	@mkdir -p $(shell dirname $@)
	$(CC) $(CFLAGS) -o $@ $<

$(ASM_OBJS): $(BUILD_DIR)/%.asm.o : $(SRC_DIR)/%.S
	@mkdir -p $(shell dirname $@)
	$(CC) $(CFLAGS) -o $@ $<

all: dirs linker $(OBJS) $(SHARED_OBJS) $(ASM_OBJS)
	$(LD) $(LDFLAGS) -T $(LD_TARGET) $(ASM_OBJS) $(OBJS) $(SHARED_OBJS) -o bin/kernel

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)
	rm -rf $(UNIT_TARGET) $(LD_TARGET)

dirs:
	@mkdir -p $(BUILD_DIR) $(BIN_DIR)

linker:
	cpp -P linker.ld $(INC) >| $(LD_TARGET)
# TODO: NEXT: We gotta break up into modules for testing
unit_test: $(TEST_ASM_OBJS) $(OBJS)
	gcc -m32 $(TEST_SRCS) $(TEST_DEPS) $(TEST_ASM_OBJS) $(OBJS) -I$(DEPS_DIR) -I$(INC_DIR) -I../shared/include -o $(UNIT_TARGET)
	./$(UNIT_TARGET)
	$(MAKE) clean

unit_test_dev:
	ls {$(INC_DIR),$(SRC_DIR),$(TEST_DIR)}/**/*.{h,c} | entr -s 'make -s unit_test'
