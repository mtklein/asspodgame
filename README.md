# Advanced Sagebrush & Shootouts: The Game

A GBA top-down RPG based on the [Advanced Sagebrush & Shootouts](https://open.spotify.com/show/1ss6ycqLBCnw01irUbR3gC) podcast.

*"In the DFW, crime is a real tough nut, and the regular police can't crack it... now it's up to the DFWTF to save everybody's bacon."*

Play as **Trevor Steel** (ex-FBI hotshot) and **Kip Simpkins** (head-banger in seven bands) as they tackle crime, incompetence, and Taco-Bongo cravings across the Dallas-Fort Worth metroplex.

## Building

### Prerequisites

Just the ARM cross-compiler from Homebrew (no tap needed):

**macOS** (Homebrew):
```bash
brew install arm-none-eabi-gcc arm-none-eabi-binutils
```

**Linux** (apt):
```bash
sudo apt install gcc-arm-none-eabi binutils-arm-none-eabi libnewlib-arm-none-eabi
```

That's it — `gbafix` (the ROM header fixer) is bundled in `tools/` and built automatically by Make.

### Build

```bash
make
```

This produces `ass_game.gba` — a ROM playable in any GBA emulator.

### Run

```bash
# With mGBA:
make run
# Or open ass_game.gba in your emulator of choice
```

## Controls

| Button | Action |
|--------|--------|
| D-Pad | Move |
| A | Interact / Advance dialogue / Confirm |
| B | Cancel / Speed up text |
| SELECT | Switch between Trevor and Kip |
| START | Pause menu |

## Game Systems

- **Fate-Inspired Stats**: SHOOT, BRAWN, BRAINS, TALK, COOL
- **Fate Points**: Earned by making funny/in-character dialogue choices, spent for combat bonuses
- **Buddy-Cop Switching**: Swap between Trevor and Kip — each has different strengths
- **Turn-Based Combat**: Fate dice rolls + stats vs enemy difficulty

## Project Structure

```
src/            C source files
  main.c        Entry point
  game.c        Game state machine
  game_data.c   All content: maps, dialogue, sprites
  entity.c      Entity/NPC system
  dialogue.c    Text box and dialogue trees
  combat.c      Turn-based combat
  map.c         Tilemap and scrolling
  sprite.c      OAM sprite management
  input.c       Input with edge detection
  crt0.s        GBA startup assembly
include/        Headers
tools/          Asset generation scripts
  genfont.py    Pixel font generator
gba_cart.ld     GBA linker script
Makefile        Build system
DESIGN.md       Game design document
```
