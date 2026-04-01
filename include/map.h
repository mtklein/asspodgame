// map.h — Tilemap / scrolling background system
#ifndef MAP_H
#define MAP_H

#include "gba.h"

// Map dimensions in tiles (each tile is 8x8 pixels)
#define MAP_MAX_W 64
#define MAP_MAX_H 64

// A map layer
typedef struct {
    u16 width;       // Width in tiles
    u16 height;      // Height in tiles
    const u16 *data; // Tile indices (screen entries)
} MapLayer;

// Full map with collision
typedef struct {
    MapLayer bg;           // Background decorative layer
    MapLayer fg;           // Foreground walkable layer
    const u8 *collision;   // 1 byte per tile: 0=walkable, 1=solid, 2+=special
    u16 width;             // Map width in tiles
    u16 height;            // Map height in tiles
} Map;

// Current map state
extern const Map *current_map;
extern s32 camera_x, camera_y;  // Camera position in pixels

// Load a map and set up BG layers
void map_load(const Map *map);

// Update camera to follow a target position (typically player)
void map_update_camera(int target_x, int target_y);

// Apply camera scroll to BG registers
void map_apply_scroll(void);

// Check if a tile at pixel coords is walkable
int map_is_walkable(int px, int py);

// Get collision value at pixel position
u8 map_get_collision(int px, int py);

// Load tileset graphics into BG VRAM
void map_load_tileset(const void *tiles, int num_bytes, int cbb);

// Load tileset palette
void map_load_palette(const u16 *pal, int num_colors);

#endif // MAP_H
