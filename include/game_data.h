// game_data.h — Game content declarations
// Maps and dialogue are generated from levels/ by bake scripts.
#ifndef GAME_DATA_H
#define GAME_DATA_H

#include "gba.h"
#include "map.h"
#include "dialogue.h"
#include "gen_maps.h"
#include "gen_dialogue.h"
#include "gen_tileset.h"

// --- Character Sprites ---
#define SPR_TILES_PER_CHAR  32
#define SPR_WORDS_PER_CHAR  (SPR_TILES_PER_CHAR * 8)
#define SPR_BYTES_PER_CHAR  (SPR_TILES_PER_CHAR * 32)

extern const u32 spr_trevor[SPR_WORDS_PER_CHAR];
extern const u32 spr_kip[SPR_WORDS_PER_CHAR];
extern const u32 spr_npc_rubik[SPR_WORDS_PER_CHAR];
extern const u32 spr_npc_generic[SPR_WORDS_PER_CHAR];

extern const u16 spr_pal_characters[16];

#define SPR_TREVOR_BASE   0
#define SPR_KIP_BASE      32
#define SPR_RUBIK_BASE    64
#define SPR_GENERIC_BASE  96

// --- Interaction Indicator Sprite ---
extern const u32 spr_a_indicator[8];
#define SPR_INDICATOR_TILE  128
#define INDICATOR_OAM_SLOT  32

#endif // GAME_DATA_H
