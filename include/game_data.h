// game_data.h — All game content declarations
#ifndef GAME_DATA_H
#define GAME_DATA_H

#include "gba.h"
#include "map.h"
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
extern const Map map_dfwtf_hq;
extern const Map map_dfw_streets;
extern const Map map_taco_bongo;

// --- Tileset (flat array for DMA copy to VRAM charblock) ---
// 64 tiles × 8 u32 words per tile = 512 words
#define TILESET_URBAN_TILES  64
#define TILESET_URBAN_SIZE   (TILESET_URBAN_TILES * 32)  // bytes
extern const u32 tileset_urban[TILESET_URBAN_TILES * 8];
extern const u16 tileset_urban_pal[16];

// --- Character Sprites ---
// Each character: 4 directions × 2 frames × 4 tiles per 16x16 = 32 tiles
// (frames 2-3 mirror frames 0-1 for walk cycle)
// Each tile = 8 u32 words (4bpp 8×8)
// Total per character: 32 tiles × 8 words = 256 u32 words
#define SPR_TILES_PER_CHAR  32
#define SPR_WORDS_PER_CHAR  (SPR_TILES_PER_CHAR * 8)
#define SPR_BYTES_PER_CHAR  (SPR_TILES_PER_CHAR * 32)

extern const u32 spr_trevor[SPR_WORDS_PER_CHAR];
extern const u32 spr_kip[SPR_WORDS_PER_CHAR];
extern const u32 spr_npc_rubik[SPR_WORDS_PER_CHAR];
extern const u32 spr_npc_generic[SPR_WORDS_PER_CHAR];

extern const u16 spr_pal_characters[16];

// Sprite tile base offsets (in tile units, into OBJ VRAM)
#define SPR_TREVOR_BASE   0
#define SPR_KIP_BASE      32
#define SPR_RUBIK_BASE    64
#define SPR_GENERIC_BASE  96

#endif // GAME_DATA_H
