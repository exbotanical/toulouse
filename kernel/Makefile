ROOTPATH ?= $(shell readlink -f ..)
include $(ROOTPATH)/Makefile.config

.PHONY: all clean dirs
.DELETE_ON_ERROR:
UNIT_TARGET := unit_test

BUILD_DIR    := build
BIN_DIR      := bin

SRC_DIR      := src
INC_DIR      := include
DEPSDIR      := deps
TESTDIR      := t

CSRCS        := $(shell find -L $(SRC_DIR) -type f -name "*.c")
OBJS         := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(CSRCS))
SHARED_OBJS  := $(shell find -L ../shared/ -type f -name "*.o")
TEST_DEPS    := $(wildcard $(DEPSDIR)/libtap/*.c)
TESTSRCS     := $(shell find -L $(TESTDIR) -type f -name "*.c")

CFLAGS       := -m32 -std=gnu99 -fno-strict-aliasing -nostdlib -ffreestanding \
	-c -mno-sse -O3 -fno-stack-protector
# Why no SSE - see: https://forum.osdev.org/viewtopic.php?p=234155
# TODO: -fstack-protector-all (need to link)

CFLAGS       += $(C_STRICTMODE_FLAGS)
CFLAGS       += -I$(INC_DIR)

LDFLAGS      := -nostdlib -O3
LDFLAGS      += -m $(shell $(LD) -V | grep elf_i386 2>/dev/null | head -n 1)

build/loader.o: src/loader.S
	$(CC) $(CFLAGS) -c $< -o $@

$(SHARED_OBJS):
	$(MAKE) -C ../shared all

$(OBJS): $(BUILD_DIR)/%.o : $(SRC_DIR)/%.c
	@mkdir -p $(shell dirname $@)
	$(CC) $(CFLAGS) -I../shared/include -o $@ $<

# TODO: fix
build/arch_idt.o:
	$(CC) $(CFLAGS) src/arch/idt.s -o build/arch_idt.o

all: dirs $(OBJS) $(SHARED_OBJS) build/loader.o build/arch_idt.o
	$(LD) $(LDFLAGS) -T linker.ld build/loader.o $(OBJS) $(SHARED_OBJS) build/arch_idt.o -o bin/kernel

clean:
	rm -rf $(BUILD_DIR)
	rm -rf $(BIN_DIR)
	rm -rf $(UNIT_TARGET)

dirs:
	@mkdir -p $(BUILD_DIR) $(BIN_DIR)

unit_test: $(OBJS)
	gcc -m32 $(TESTSRCS) $(TEST_DEPS) $(OBJS) -I$(DEPSDIR) -I$(INC_DIR) -I../shared/include -o $(UNIT_TARGET)
	./$(UNIT_TARGET)
	$(MAKE) clean
