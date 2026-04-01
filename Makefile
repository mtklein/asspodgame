# Makefile — Advanced Sagebrush & Shootouts: The Game
# Targets GBA via arm-none-eabi toolchain
#
# Setup (Homebrew, no tap needed):
#   brew install arm-none-eabi-gcc arm-none-eabi-binutils
#
# Optional (for emulator):
#   brew install mgba
#
# Usage:
#   make          — Build the .gba ROM
#   make clean    — Remove build artifacts
#   make run      — Build and open in mGBA (if installed)

# --- Toolchain ---
PREFIX  := arm-none-eabi-
CC      := $(PREFIX)gcc
AS      := $(PREFIX)as
LD      := $(PREFIX)gcc
OBJCOPY := $(PREFIX)objcopy

# --- Project ---
TARGET  := ass_game
BUILD   := build

SRCDIR  := src
INCDIR  := include
TOOLDIR := tools

# --- Source files ---
CFILES  := $(wildcard $(SRCDIR)/*.c)
SFILES  := $(wildcard $(SRCDIR)/*.s)
OFILES  := $(patsubst $(SRCDIR)/%.c,$(BUILD)/%.o,$(CFILES)) \
           $(patsubst $(SRCDIR)/%.s,$(BUILD)/%.o,$(SFILES))

# --- Flags ---
ARCH    := -mthumb-interwork -mthumb
CFLAGS  := -O2 -Wall -Wextra -Wno-unused-parameter \
           -mcpu=arm7tdmi -mtune=arm7tdmi $(ARCH) \
           -I$(INCDIR) -fno-strict-aliasing
ASFLAGS := -mcpu=arm7tdmi -mthumb-interwork
LDFLAGS := -T gba_cart.ld -nostartfiles $(ARCH) \
           -Wl,-Map,$(BUILD)/$(TARGET).map \
           -specs=nosys.specs

# --- Rules ---
.PHONY: all clean run gbafix

all: $(TARGET).gba

$(BUILD):
	mkdir -p $(BUILD)

# Build the gbafix tool from source (bundled, no external dep)
$(BUILD)/gbafix: $(TOOLDIR)/gbafix.c | $(BUILD)
	cc -O2 -o $@ $<

$(BUILD)/%.o: $(SRCDIR)/%.c | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/%.o: $(SRCDIR)/%.s | $(BUILD)
	$(AS) $(ASFLAGS) $< -o $@

$(BUILD)/$(TARGET).elf: $(OFILES)
	$(LD) $(LDFLAGS) $^ -o $@

$(TARGET).gba: $(BUILD)/$(TARGET).elf $(BUILD)/gbafix
	$(OBJCOPY) -O binary $< $@
	$(BUILD)/gbafix $@

clean:
	rm -rf $(BUILD) $(TARGET).gba

run: $(TARGET).gba
	open -a mGBA $(TARGET).gba

# Regenerate font data from Python script
font:
	python3 $(TOOLDIR)/genfont.py > $(INCDIR)/font_data.h

# --- Dependencies ---
$(BUILD)/%.d: $(SRCDIR)/%.c | $(BUILD)
	$(CC) $(CFLAGS) -MM -MT $(BUILD)/$*.o $< -MF $@

-include $(patsubst $(SRCDIR)/%.c,$(BUILD)/%.d,$(CFILES))
