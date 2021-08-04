ASSEMBLER=nasm
FLAGS=-f bin 

EMULATOR=qemu-system-x86_64
EMU_FLAGS=-hda

ENTRY=src/boot/boot.asm
OUT=bin/boot.bin

all:
	$(ASSEMBLER) $(FLAGS) $(ENTRY) -o $(OUT)

run: all
	$(EMULATOR) $(EMU_FLAGS) ./$(OUT)

usb: all
	sudo dd if=./$(OUT) of=/dev/sdb

clean:
	rm  -f ./bin/boot.bin

.PHONY: clean
