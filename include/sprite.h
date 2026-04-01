// sprite.h — Sprite management for OAM
#ifndef SPRITE_H
#define SPRITE_H

#include "gba.h"

#define MAX_SPRITES 128

// Shadow OAM buffer (written to real OAM during vblank)
extern OBJ_ATTR obj_buffer[MAX_SPRITES];

// Initialize all sprites to hidden
void sprite_init(void);

// Copy shadow OAM to hardware OAM
void sprite_update_oam(void);

// Set a sprite's position
void sprite_set_pos(int id, int x, int y);

// Set a sprite's tile and palette
void sprite_set_tile(int id, int tile, int palbank);

// Hide a specific sprite
void sprite_hide(int id);

// Show a sprite (set to regular mode)
void sprite_show(int id);

// Set sprite size (use ATTR1_SIZE_* constants)
void sprite_set_size(int id, u16 shape, u16 size);

// Load sprite tile data into VRAM (obj tiles start at 0x06010000)
void sprite_load_tiles(const void *data, int tile_start, int num_bytes);

// Load sprite palette
void sprite_load_palette(const u16 *pal, int palbank, int num_colors);

#endif // SPRITE_H
