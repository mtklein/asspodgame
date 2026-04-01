// map.c — Tilemap and scrolling implementation
#include "map.h"

#define SCREEN_W 240
#define SCREEN_H 160

const Map *current_map = 0;
s32 camera_x = 0;
s32 camera_y = 0;

void map_load(const Map *map) {
    current_map = map;
    camera_x = 0;
    camera_y = 0;

    // Copy foreground map data to screenblock 28 (BG0)
    if (map->fg.data) {
        u16 *sbb = (u16*)SBB_ADDR(28);
        for (int y = 0; y < map->fg.height && y < 32; y++) {
            for (int x = 0; x < map->fg.width && x < 32; x++) {
                sbb[y * 32 + x] = map->fg.data[y * map->fg.width + x];
            }
        }
    }

    // Copy background map data to screenblock 30 (BG1)
    if (map->bg.data) {
        u16 *sbb = (u16*)SBB_ADDR(30);
        for (int y = 0; y < map->bg.height && y < 32; y++) {
            for (int x = 0; x < map->bg.width && x < 32; x++) {
                sbb[y * 32 + x] = map->bg.data[y * map->bg.width + x];
            }
        }
    }

    // Configure BG0: foreground, cbb 0, sbb 28, 256x256
    REG_BG0CNT = BG_PRIO(0) | BG_CBB(0) | BG_SBB(28) | BG_4BPP | BG_SIZE_256x256;
    // Configure BG1: background, cbb 0, sbb 30, 256x256
    REG_BG1CNT = BG_PRIO(1) | BG_CBB(0) | BG_SBB(30) | BG_4BPP | BG_SIZE_256x256;
}

void map_update_camera(int target_x, int target_y) {
    if (!current_map) return;

    // Center camera on target
    camera_x = target_x - SCREEN_W / 2;
    camera_y = target_y - SCREEN_H / 2;

    // Clamp to map bounds
    int max_x = (int)current_map->width * 8 - SCREEN_W;
    int max_y = (int)current_map->height * 8 - SCREEN_H;
    if (max_x < 0) max_x = 0;
    if (max_y < 0) max_y = 0;

    if (camera_x < 0) camera_x = 0;
    if (camera_y < 0) camera_y = 0;
    if (camera_x > max_x) camera_x = max_x;
    if (camera_y > max_y) camera_y = max_y;
}

void map_apply_scroll(void) {
    REG_BG0HOFS = (u16)camera_x;
    REG_BG0VOFS = (u16)camera_y;
    REG_BG1HOFS = (u16)camera_x;
    REG_BG1VOFS = (u16)camera_y;
}

int map_is_walkable(int px, int py) {
    u8 col = map_get_collision(px, py);
    return col == 0 || col >= 10;
}

u8 map_get_collision(int px, int py) {
    if (!current_map || !current_map->collision) return 1;

    int tx = px / 8;
    int ty = py / 8;

    if (tx < 0 || tx >= current_map->width ||
        ty < 0 || ty >= current_map->height) {
        return 1; // Out of bounds = solid
    }

    return current_map->collision[ty * current_map->width + tx];
}

void map_load_tileset(const void *tiles, int num_bytes, int cbb) {
    dma3_copy((void*)CBB_ADDR(cbb), tiles, num_bytes / 4, DMA_32 | DMA_NOW);
}

void map_load_palette(const u16 *pal, int num_colors) {
    u16 *dst = (u16*)MEM_PAL_BG;
    for (int i = 0; i < num_colors; i++) {
        dst[i] = pal[i];
    }
}
