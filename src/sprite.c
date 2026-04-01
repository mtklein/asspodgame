// sprite.c — Sprite management implementation
#include "sprite.h"

#define OBJ_VRAM_BASE 0x06010000

OBJ_ATTR obj_buffer[MAX_SPRITES];

void sprite_init(void) {
    for (int i = 0; i < MAX_SPRITES; i++) {
        obj_buffer[i].attr0 = ATTR0_HIDE;
        obj_buffer[i].attr1 = 0;
        obj_buffer[i].attr2 = 0;
        obj_buffer[i].pad   = 0;
    }
    sprite_update_oam();
}

void sprite_update_oam(void) {
    // DMA copy shadow OAM → hardware OAM
    dma3_copy((void*)MEM_OAM, obj_buffer,
              MAX_SPRITES * sizeof(OBJ_ATTR) / 4, DMA_32 | DMA_NOW);
}

void sprite_set_pos(int id, int x, int y) {
    obj_buffer[id].attr0 = (obj_buffer[id].attr0 & ~0xFF) | ATTR0_Y(y);
    obj_buffer[id].attr1 = (obj_buffer[id].attr1 & ~0x1FF) | ATTR1_X(x);
}

void sprite_set_tile(int id, int tile, int palbank) {
    obj_buffer[id].attr2 = (u16)(ATTR2_TILE(tile) | ATTR2_PALBANK(palbank) | ATTR2_PRIO(0));
}

void sprite_hide(int id) {
    obj_buffer[id].attr0 = (obj_buffer[id].attr0 & ~0x0300) | ATTR0_HIDE;
}

void sprite_show(int id) {
    obj_buffer[id].attr0 = (obj_buffer[id].attr0 & ~0x0300) | ATTR0_REG;
}

void sprite_set_size(int id, u16 shape, u16 size) {
    obj_buffer[id].attr0 = (obj_buffer[id].attr0 & ~0xC000) | shape;
    obj_buffer[id].attr1 = (obj_buffer[id].attr1 & ~0xC000) | size;
}

void sprite_load_tiles(const void *data, int tile_start, int num_bytes) {
    // Each 4bpp tile is 32 bytes; tile_start is in tile units
    u32 dest = (u32)(OBJ_VRAM_BASE + tile_start * 32);
    dma3_copy((void*)dest, data, (u32)(num_bytes / 4), DMA_32 | DMA_NOW);
}

void sprite_load_palette(const u16 *pal, int palbank, int num_colors) {
    u16 *dst = (u16*)(MEM_PAL_OBJ + palbank * 32);
    for (int i = 0; i < num_colors; i++) {
        dst[i] = pal[i];
    }
}
