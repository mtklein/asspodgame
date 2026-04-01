// game_data.h — All game content: maps, dialogue, sprites
// Advanced Sagebrush & Shootouts: The Game
#ifndef GAME_DATA_H
#define GAME_DATA_H

#include "gba.h"
#include "dialogue.h"

// --- Dialogue Trees ---
#define NUM_DIALOGUE_TREES 8

extern const DialogueTree dialogue_trees[NUM_DIALOGUE_TREES];

// Tree IDs (referenced by NPC dialogue_id)
#define DLG_RUBIK_INTRO     1
#define DLG_PEACEMAKER_HINT 2
#define DLG_DONNY_TACOS     3
#define DLG_RAMIS_CAR       4
#define DLG_RANDOM_COP_1    5
#define DLG_RANDOM_COP_2    6
#define DLG_INFORMANT       7

// --- Map Data ---
// DFWTF HQ map (the starting area)
extern const Map map_dfwtf_hq;
// DFW Streets map
extern const Map map_dfw_streets;
// Taco-Bongo restaurant
extern const Map map_taco_bongo;

// --- Tileset ---
extern const u32 tileset_urban[];
extern const u16 tileset_urban_pal[];
#define TILESET_URBAN_SIZE  (64 * 32)  // 64 tiles × 32 bytes each
#define TILESET_URBAN_COLORS 16

// --- Character Sprites ---
// Each character: 4 directions × 4 frames × 4 tiles (16x16 @ 4bpp) = 256 tiles
extern const u32 spr_trevor[];
extern const u32 spr_kip[];
extern const u32 spr_npc_rubik[];
extern const u32 spr_npc_generic[];

extern const u16 spr_pal_characters[];
#define SPR_PAL_CHARS_COLORS 16

// Sprite tile base offsets
#define SPR_TREVOR_BASE   0
#define SPR_KIP_BASE      64   // 4 dir × 4 frame × 4 tiles
#define SPR_RUBIK_BASE    128
#define SPR_GENERIC_BASE  192

// --- Portrait data ---
#define NUM_PORTRAITS 6
#define PORTRAIT_TREVOR    0
#define PORTRAIT_KIP       1
#define PORTRAIT_RUBIK     2
#define PORTRAIT_PEACEMAKER 3
#define PORTRAIT_DONNY     4
#define PORTRAIT_RAMIS     5

#endif // GAME_DATA_H
