# Advanced Sagebrush & Shootouts: The Game
## GBA Top-Down RPG — Design Document

---

## World Overview

**Setting**: The Dallas-Fort Worth metroplex — a sprawling urban landscape where crime has gotten so out of control that the regular police can't handle it. Enter the **DFWTF** (Dallas-Fort Worth Task Force), an elite (well, "elite") buddy-cop unit tasked with saving everybody's bacon.

**Tone**: Absurdist buddy-cop comedy. Think Archer meets a tabletop RPG played by three friends who can barely keep a straight face. The humor is improvisational, character-driven, and revels in the gap between how cool the characters think they are vs. how chaotic everything actually gets.

**System Roots**: The podcast uses the Fate RPG system, so the game will nod to that with Fate-inspired mechanics — aspects, stunts, and fate points translated into GBA RPG terms.

---

## Main Characters (Playable)

### Trevor Steel
- **Role**: Ex-FBI agent, now DFWTF's finest
- **Personality**: Thinks he's James Bond, acts more like Sterling Archer. Suave self-image, chaotic reality. Lacrosse enthusiast.
- **Stats**: High SHOOT, high TALK, mediocre BRAINS
- **Special Ability**: "Federal Authority" — can flash old FBI badge to intimidate or bluff

### Kip Simpkins
- **Role**: Trevor's partner, head-banger extraordinaire
- **Personality**: Somehow juggles being a cop AND playing in seven bands. Laid-back, surprisingly competent, great comic foil to Trevor's ego.
- **Stats**: High BRAWN, high COOL, mediocre SHOOT
- **Special Ability**: "Seven Bands" — can use music knowledge to solve problems or rally allies

---

## Supporting Cast (NPCs)

- **Jim Rubik** — Fellow DFWTF agent, the straight man who watches in horror as Trevor and Kip improvise
- **Shauna Peacemaker** — DFWTF agent with a name that writes its own jokes
- **Donny** — Friend of the crew, always down for boys' night and Taco-Bongo
- **LaRue** — Goes missing periodically, creating side quests
- **Ramis** — Gets the crew into trouble with expensive equipment (see: robot car incident)

---

## Game Structure

### Chapter-Based Story Arcs
Each chapter is inspired by a podcast story arc:

1. **Chapter 1: "The Task Force"** — Tutorial/intro. Trevor's first day at the DFWTF. Meet Kip. Learn the ropes. Bust a small-time crook.
2. **Chapter 2: "R.A.D."** — An AI supercar has gone rogue in DFW. Chase it down before it causes more damage (and before the department finds out Ramis lost it).
3. **Chapter 3: "Boys' Night"** — LaRue is missing. Investigate his disappearance while Donny insists on making it a night out. Taco-Bongo is involved.
4. **Chapter 4: "Nasty on Crime"** — The DFWTF teams up with the AFPP for a joint operation. Loot distribution gets competitive.
5. **Chapter 5: "The Big One"** — Final showdown. All plot threads converge.

### Gameplay Loop
1. **Explore** the DFW overworld (city streets, offices, Taco-Bongo, band practice venues)
2. **Talk** to NPCs — dialogue trees with humor choices that affect Fate Points
3. **Investigate** crime scenes — find clues, use items
4. **Combat** — top-down turn-based encounters against criminals, rogue robots, etc.

---

## Core Mechanics

### Stats (Fate-Inspired)
- **SHOOT** — Ranged combat, firearms
- **BRAWN** — Melee, physical feats
- **BRAINS** — Investigation, tech, puzzles
- **TALK** — Persuasion, intimidation, bluffing
- **COOL** — Composure under pressure, style points

### Fate Points
- Earned by making hilarious or in-character choices in dialogue
- Spent to activate special abilities, re-roll combat, or trigger "Compel" moments (funny forced failures that advance the plot)

### Aspects
- Each character has 2-3 Aspects (e.g., Trevor: "Ex-FBI Hotshot", "Lacrosse Is Life")
- Aspects can be invoked in relevant situations for bonuses
- NPCs and locations also have Aspects that can be discovered and exploited

### Combat
- Top-down grid, 2-character party (Trevor + Kip, player switches between them)
- Turn-based with action points
- Use SHOOT for ranged, BRAWN for melee, COOL to dodge
- "Stunt" system: special moves unlocked per character

---

## Visual Style

- 16-bit GBA pixel art, top-down perspective
- Urban DFW aesthetic: strip malls, highways, office buildings, dive bars
- Character sprites with exaggerated personality (Trevor's swagger walk, Kip's headbang idle)
- UI styled like a police case file / Fate character sheet hybrid

---

## Technical Target

- **Platform**: Game Boy Advance (ARM7TDMI, 240x160, 16-bit color)
- **Toolchain**: devkitARM / libgba
- **Language**: C
- **ROM Size Target**: ≤ 4MB (comfortable for a text-heavy RPG with tile maps)
- **Audio**: MOD/tracker music (GBA hardware sound channels)

---

## Art & Asset Pipeline

All graphics will be 4bpp (16-color palette per tile/sprite set):
- 8x8 or 16x16 tile maps for overworld and interiors
- 16x16 or 32x32 character sprites with 4-directional walk cycles
- Dialogue portraits (32x32 or larger)
- UI elements (text boxes, menus, health bars)

---

## Humor Guidelines

The game's writing should capture the podcast's style:
- Characters are self-aware about absurd situations but play them straight
- Trevor always has an overly confident plan that goes sideways
- Kip's band commitments create running gags
- Bureaucratic humor (task force acronyms, paperwork as gameplay elements)
- Pop culture references filtered through a DFW lens
- The "actual play" format means occasional fourth-wall-adjacent moments
