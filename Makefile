DEPS_DIR := deps

# Exit Qemu focus trap with ctrl+alt+g
.PHONY: run; run:
	$(EMU) -hda $(OS_TARGET)

.PHONY: clean_deps; clean_deps:
	rm -rf $(DEPS_DIR)
