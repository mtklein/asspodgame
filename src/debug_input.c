// debug_input.c — On-screen key press display for debugging
// Shows currently held keys as small sprites in the bottom-left corner.
#include "debug_input.h"
#include "gba.h"
#include "input.h"
#include "sprite.h"

// We use OAM slots 118-127 and OBJ tiles starting at 256.
// Each key gets one 8x8 tile with a 2-3 letter label.
// Palette bank 1 is used for debug display colors.

#define DBG_OAM_BASE   118
#define DBG_TILE_BASE  256
#define DBG_PALBANK    1

#define NUM_KEYS 10

// Key definitions in display order
static const u16 key_masks[NUM_KEYS] = {
    KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT,
    KEY_A, KEY_B, KEY_START, KEY_SELECT,
    KEY_L, KEY_R,
};

// 8x8 4bpp tile data for each key label.
// Pixel value 1 = label color, 2 = background when pressed.
// Each row of an 8x8 4bpp tile = 1 u32 word (4 bits per pixel, 8 pixels).

// Helper: encode 8 pixels, each 0-F, packed little-endian
// Pixel 0 is low nibble of byte 0, pixel 1 is high nibble, etc.
#define PX8(a,b,c,d,e,f,g,h) \
    ((u32)(a) | ((u32)(b)<<4) | ((u32)(c)<<8) | ((u32)(d)<<12) | \
     ((u32)(e)<<16) | ((u32)(f)<<20) | ((u32)(g)<<24) | ((u32)(h)<<28))

static const u32 key_tiles[NUM_KEYS][8] = {
    // UP: "U" with up arrow
    {   PX8(0,0,1,1,0,0,0,0),
        PX8(0,1,1,1,1,0,0,0),
        PX8(0,0,1,1,0,0,0,0),
        PX8(1,0,1,1,0,1,0,0),
        PX8(1,0,1,1,0,1,0,0),
        PX8(1,0,1,1,0,1,0,0),
        PX8(0,1,1,1,1,0,0,0),
        PX8(0,0,0,0,0,0,0,0),
    },
    // DOWN: "D" with down arrow
    {   PX8(1,1,1,0,0,0,0,0),
        PX8(1,0,0,1,0,0,0,0),
        PX8(1,0,0,1,0,0,0,0),
        PX8(1,0,0,1,0,0,0,0),
        PX8(1,1,1,0,0,0,0,0),
        PX8(0,0,1,1,0,0,0,0),
        PX8(0,1,1,1,1,0,0,0),
        PX8(0,0,1,1,0,0,0,0),
    },
    // LEFT: "<" arrow
    {   PX8(0,0,0,1,0,0,0,0),
        PX8(0,0,1,0,0,0,0,0),
        PX8(0,1,0,0,0,0,0,0),
        PX8(1,0,0,0,0,0,0,0),
        PX8(0,1,0,0,0,0,0,0),
        PX8(0,0,1,0,0,0,0,0),
        PX8(0,0,0,1,0,0,0,0),
        PX8(0,0,0,0,0,0,0,0),
    },
    // RIGHT: ">" arrow
    {   PX8(1,0,0,0,0,0,0,0),
        PX8(0,1,0,0,0,0,0,0),
        PX8(0,0,1,0,0,0,0,0),
        PX8(0,0,0,1,0,0,0,0),
        PX8(0,0,1,0,0,0,0,0),
        PX8(0,1,0,0,0,0,0,0),
        PX8(1,0,0,0,0,0,0,0),
        PX8(0,0,0,0,0,0,0,0),
    },
    // A
    {   PX8(0,1,1,0,0,0,0,0),
        PX8(1,0,0,1,0,0,0,0),
        PX8(1,0,0,1,0,0,0,0),
        PX8(1,1,1,1,0,0,0,0),
        PX8(1,0,0,1,0,0,0,0),
        PX8(1,0,0,1,0,0,0,0),
        PX8(0,0,0,0,0,0,0,0),
        PX8(0,0,0,0,0,0,0,0),
    },
    // B
    {   PX8(1,1,1,0,0,0,0,0),
        PX8(1,0,0,1,0,0,0,0),
        PX8(1,1,1,0,0,0,0,0),
        PX8(1,0,0,1,0,0,0,0),
        PX8(1,0,0,1,0,0,0,0),
        PX8(1,1,1,0,0,0,0,0),
        PX8(0,0,0,0,0,0,0,0),
        PX8(0,0,0,0,0,0,0,0),
    },
    // START: "ST"
    {   PX8(0,1,1,0,1,1,1,0),
        PX8(1,0,0,0,0,1,0,0),
        PX8(0,1,0,0,0,1,0,0),
        PX8(0,0,1,0,0,1,0,0),
        PX8(1,1,0,0,0,1,0,0),
        PX8(0,0,0,0,0,0,0,0),
        PX8(0,0,0,0,0,0,0,0),
        PX8(0,0,0,0,0,0,0,0),
    },
    // SELECT: "SE"
    {   PX8(0,1,1,0,1,1,1,0),
        PX8(1,0,0,0,1,0,0,0),
        PX8(0,1,0,0,1,1,0,0),
        PX8(0,0,1,0,1,0,0,0),
        PX8(1,1,0,0,1,1,1,0),
        PX8(0,0,0,0,0,0,0,0),
        PX8(0,0,0,0,0,0,0,0),
        PX8(0,0,0,0,0,0,0,0),
    },
    // L
    {   PX8(1,0,0,0,0,0,0,0),
        PX8(1,0,0,0,0,0,0,0),
        PX8(1,0,0,0,0,0,0,0),
        PX8(1,0,0,0,0,0,0,0),
        PX8(1,0,0,0,0,0,0,0),
        PX8(1,1,1,0,0,0,0,0),
        PX8(0,0,0,0,0,0,0,0),
        PX8(0,0,0,0,0,0,0,0),
    },
    // R
    {   PX8(1,1,1,0,0,0,0,0),
        PX8(1,0,0,1,0,0,0,0),
        PX8(1,0,0,1,0,0,0,0),
        PX8(1,1,1,0,0,0,0,0),
        PX8(1,0,1,0,0,0,0,0),
        PX8(1,0,0,1,0,0,0,0),
        PX8(0,0,0,0,0,0,0,0),
        PX8(0,0,0,0,0,0,0,0),
    },
};

// "Pressed" version: fill background with color 2
static u32 pressed_tile[8];

static void make_pressed_tile(const u32 src[8]) {
    for (int i = 0; i < 8; i++) {
        u32 row = 0;
        for (int p = 0; p < 8; p++) {
            u32 px = (src[i] >> (p * 4)) & 0xF;
            if (px == 0) px = 2;  // Fill transparent with bg color
            row |= (px << (p * 4));
        }
        pressed_tile[i] = row;
    }
}

void debug_input_init(void) {
    // Load debug palette into palbank 1
    // 0=transparent, 1=dim grey (inactive label), 2=bright green (pressed bg)
    u16 *pal = (u16*)(MEM_PAL_OBJ + DBG_PALBANK * 32);
    pal[0] = RGB15(0, 0, 0);     // transparent
    pal[1] = RGB15(10, 10, 10);  // dim label (inactive)
    pal[2] = RGB15(0, 28, 0);    // bright green (pressed bg)
    pal[3] = RGB15(31, 31, 31);  // white (pressed label)

    // Load "inactive" tile data for all keys into OBJ VRAM
    for (int k = 0; k < NUM_KEYS; k++) {
        sprite_load_tiles(key_tiles[k], DBG_TILE_BASE + k * 2, 32);
        // Also store a "pressed" variant at tile+1
        make_pressed_tile(key_tiles[k]);
        // Replace color 1 with color 3 (white) in pressed version
        for (int i = 0; i < 8; i++) {
            u32 row = 0;
            for (int p = 0; p < 8; p++) {
                u32 px = (pressed_tile[i] >> (p * 4)) & 0xF;
                if (px == 1) px = 3;  // label → white
                row |= (px << (p * 4));
            }
            pressed_tile[i] = row;
        }
        sprite_load_tiles(pressed_tile, DBG_TILE_BASE + k * 2 + 1, 32);
    }

    // Set up OAM entries — row of 8x8 sprites at bottom-left
    for (int k = 0; k < NUM_KEYS; k++) {
        int slot = DBG_OAM_BASE + k;
        sprite_show(slot);
        sprite_set_size(slot, ATTR0_SQUARE, ATTR1_SIZE_8);
        sprite_set_pos(slot, 2 + k * 10, 150);
        sprite_set_tile(slot, DBG_TILE_BASE + k * 2, DBG_PALBANK);
        // Set priority to 0 (on top of everything)
        obj_buffer[slot].attr2 = ATTR2_TILE(DBG_TILE_BASE + k * 2) |
                                  ATTR2_PALBANK(DBG_PALBANK) | ATTR2_PRIO(0);
    }
}

void debug_input_update(void) {
    for (int k = 0; k < NUM_KEYS; k++) {
        int slot = DBG_OAM_BASE + k;
        int pressed = (key_cur & key_masks[k]) != 0;
        int tile = DBG_TILE_BASE + k * 2 + (pressed ? 1 : 0);
        obj_buffer[slot].attr2 = ATTR2_TILE(tile) |
                                  ATTR2_PALBANK(DBG_PALBANK) | ATTR2_PRIO(0);
    }
}
