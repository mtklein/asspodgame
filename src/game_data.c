// game_data.c — Sprite art and palette data
// Tileset, maps, and dialogue are generated from levels/ by bake scripts.
#include "game_data.h"

const u16 spr_pal_characters[16] = {
    RGB15( 0,  0,  0),  //  0: Transparent
    RGB15(31, 31, 31),  //  1: White
    RGB15(12, 12, 15),  //  2: Dark grey (pants/legs)
    RGB15(22, 22, 22),  //  3: Grey (Rubik's suit)
    RGB15(28, 20, 14),  //  4: Skin tone
    RGB15(20, 15,  8),  //  5: Tan/brown hair
    RGB15(28,  4,  4),  //  6: Red (jacket/tie)
    RGB15( 4,  4, 28),  //  7: Blue (suit)
    RGB15(15, 15,  0),  //  8: Gold
    RGB15(28, 28,  0),  //  9: Yellow
    RGB15( 0, 18,  0),  // 10: Dark green
    RGB15(28,  0, 28),  // 11: Magenta (Kip hair)
    RGB15( 0, 28,  0),  // 12: Bright green
    RGB15(18,  0,  0),  // 13: Dark red
    RGB15( 0,  8, 18),  // 14: Dark blue
    RGB15(15, 15, 15),  // 15: Mid grey
};

#define PX8(a,b,c,d,e,f,g,h) \
    ((u32)(a) | ((u32)(b)<<4) | ((u32)(c)<<8) | ((u32)(d)<<12) | \
     ((u32)(e)<<16) | ((u32)(f)<<20) | ((u32)(g)<<24) | ((u32)(h)<<28))

const u32 spr_a_indicator[8] = {
    PX8(0,0,0,9,9,0,0,0),
    PX8(0,0,9,0,0,9,0,0),
    PX8(0,0,9,0,0,9,0,0),
    PX8(0,0,9,9,9,9,0,0),
    PX8(0,0,9,0,0,9,0,0),
    PX8(0,0,9,0,0,9,0,0),
    PX8(0,0,0,0,0,0,0,0),
    PX8(0,0,0,0,0,0,0,0),
};

#undef PX8
