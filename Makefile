# Makefile — Advanced Sagebrush & Shootouts: The Game
# Targets GBA (ARM7TDMI) via Zig as a cross-compiler
#
# Setup:
#   brew install zig          (macOS)
#   pip install ziglang       (anywhere with Python)
#
# Optional (for emulator):
#   brew install --cask mgba  (macOS)
#
# Usage:
#   make          — Build the .gba ROM
#   make clean    — Remove build artifacts
#   make run      — Build and open in mGBA
#   make test     — Build test ROM and run automated tests via mGBA
#   make test-rom — Build just the test ROM

# --- Toolchain (zig as a C cross-compiler) ---
ZIG     ?= zig
ZIGCC   := $(ZIG) cc
OBJCOPY := $(ZIG) objcopy
# Host compiler for tools
HOSTCC  := cc

# --- ARM7TDMI target flags ---
TARGET  := -target arm-freestanding-none -mcpu=arm7tdmi -mfloat-abi=soft

# --- Project ---
ROM     := ass_game
BUILD   := build

SRCDIR  := src
INCDIR  := include
TOOLDIR := tools

# --- Source files (exclude test_main.c from the regular build) ---
CFILES  := $(filter-out $(SRCDIR)/test_main.c,$(wildcard $(SRCDIR)/*.c))
SFILES  := $(wildcard $(SRCDIR)/*.s)
OFILES  := $(patsubst $(SRCDIR)/%.c,$(BUILD)/%.o,$(CFILES)) \
           $(patsubst $(SRCDIR)/%.s,$(BUILD)/%.o,$(SFILES))

# --- Flags ---
CFLAGS  := $(TARGET) -mthumb -O2 -Wall -Wextra -Wno-unused-parameter \
           -I$(INCDIR) -fno-strict-aliasing
ASFLAGS := $(TARGET)
LDFLAGS := $(TARGET) -mthumb -T gba_cart.ld -nostartfiles

# --- Rules ---
.PHONY: all clean run font test test-rom

all: $(ROM).gba

# --- Test ROM (uses test_main.c instead of main.c) ---
TEST_ROM  := $(ROM)_test
TEST_SRC  := $(filter-out $(SRCDIR)/main.c,$(CFILES)) $(SRCDIR)/test_main.c
TEST_OBJ  := $(patsubst $(SRCDIR)/%.c,$(BUILD)/test_%.o,$(TEST_SRC)) \
             $(patsubst $(SRCDIR)/%.s,$(BUILD)/%.o,$(SFILES))

$(BUILD)/test_%.o: $(SRCDIR)/%.c | $(BUILD)
	$(ZIGCC) $(CFLAGS) -c $< -o $@

$(BUILD)/$(TEST_ROM).elf: $(TEST_OBJ)
	$(ZIGCC) $(LDFLAGS) $^ -o $@

$(TEST_ROM).gba: $(BUILD)/$(TEST_ROM).elf $(BUILD)/gbafix
	python3 $(TOOLDIR)/elf2bin.py $< $@
	$(BUILD)/gbafix $@

test-rom: $(TEST_ROM).gba

test: $(TEST_ROM).gba
	python3 $(TOOLDIR)/run_tests.py $(TEST_ROM).gba

$(BUILD):
	mkdir -p $(BUILD)

# Build the gbafix tool from source (bundled, no external dep)
$(BUILD)/gbafix: $(TOOLDIR)/gbafix.c | $(BUILD)
	$(HOSTCC) -O2 -o $@ $<

$(BUILD)/%.o: $(SRCDIR)/%.c | $(BUILD)
	$(ZIGCC) $(CFLAGS) -c $< -o $@

$(BUILD)/%.o: $(SRCDIR)/%.s | $(BUILD)
	$(ZIGCC) $(ASFLAGS) -c $< -o $@

$(BUILD)/$(ROM).elf: $(OFILES)
	$(ZIGCC) $(LDFLAGS) $^ -o $@

# Extract flat ROM binary from ELF (uses LMA, no external objcopy needed)
$(ROM).gba: $(BUILD)/$(ROM).elf $(BUILD)/gbafix
	python3 $(TOOLDIR)/elf2bin.py $< $@
	$(BUILD)/gbafix $@

clean:
	rm -rf $(BUILD) $(ROM).gba $(TEST_ROM).gba $(TEST_ROM).sav

run: $(ROM).gba
	open -a mGBA $(ROM).gba

# Regenerate font data from Python script
font:
	python3 $(TOOLDIR)/genfont.py > $(INCDIR)/font_data.h

# Regenerate sprite data from Python script
sprites:
	python3 $(TOOLDIR)/gensprites.py > $(SRCDIR)/sprite_data.c
