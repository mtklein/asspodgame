// game_data.c — All game content data for Advanced Sagebrush & Shootouts: The Game
#include "game_data.h"

// ============================================================================
// CHARACTER SPRITE PALETTES
// ============================================================================

// 16-color palette for character sprites (RGB555 format)
const u16 spr_pal_characters[16] = {
    RGB15(0, 0, 0),       // 0: Transparent/Black
    RGB15(31, 31, 31),    // 1: White
    RGB15(20, 20, 20),    // 2: Dark Grey
    RGB15(25, 25, 25),    // 3: Grey
    RGB15(31, 20, 0),     // 4: Brown (skin tone)
    RGB15(31, 25, 0),     // 5: Tan
    RGB15(31, 0, 0),      // 6: Red (jacket)
    RGB15(0, 0, 31),      // 7: Blue (suit/shirt)
    RGB15(15, 15, 0),     // 8: Dark Yellow/Gold
    RGB15(31, 31, 0),     // 9: Yellow
    RGB15(0, 20, 0),      // 10: Dark Green
    RGB15(31, 0, 31),     // 11: Magenta (Kip's hair)
    RGB15(0, 31, 0),      // 12: Bright Green
    RGB15(20, 0, 0),      // 13: Dark Red (tie)
    RGB15(0, 10, 20),     // 14: Dark Blue
    RGB15(15, 15, 15),    // 15: Mid Grey
};

// 16-color palette for urban tileset (RGB555 format)
const u16 tileset_urban_pal[16] = {
    RGB15(0, 0, 0),       // 0: Transparent/Black
    RGB15(31, 31, 31),    // 1: White
    RGB15(20, 20, 20),    // 2: Dark Grey (road)
    RGB15(25, 25, 25),    // 3: Medium Grey (sidewalk)
    RGB15(15, 25, 10),    // 4: Dark Green (grass)
    RGB15(25, 15, 10),    // 5: Brown (wood)
    RGB15(20, 15, 10),    // 6: Dark Brown (door)
    RGB15(25, 20, 15),    // 7: Light Brown (brick)
    RGB15(20, 20, 25),    // 8: Blue-grey (concrete)
    RGB15(31, 31, 15),    // 9: Light Yellow
    RGB15(31, 0, 0),      // 10: Red (trim)
    RGB15(0, 20, 0),      // 11: Dark Green (vegetation)
    RGB15(31, 20, 0),     // 12: Orange
    RGB15(15, 15, 15),    // 13: Dark Grey
    RGB15(30, 30, 30),    // 14: Light Grey
    RGB15(10, 10, 15),    // 15: Very Dark Grey
};

// ============================================================================
// CHARACTER SPRITE DATA (16x16 pixels = 4x4 tiles at 4bpp)
// Simplified silhouettes with character-defining features
// ============================================================================

// TREVOR STEEL - Walking right (4 frames)
// Broad shoulders, suit and tie silhouette
const u32 spr_trevor_right[16] = {
    // Frame 1
    0x00000000, 0x00055500, 0x05577750, 0x55777775,
    0x57777775, 0x57733375, 0x57733375, 0x57533375,
    0x07533375, 0x00533370, 0x00533370, 0x00050000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
};

const u32 spr_trevor_left[16] = {
    // Mirror of right
    0x00000000, 0x00555000, 0x05777500, 0x57777755,
    0x57777755, 0x57333755, 0x57333755, 0x57333750,
    0x57333750, 0x07333500, 0x07333500, 0x00050000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
};

const u32 spr_trevor_down[16] = {
    // Frontal view
    0x00055500, 0x05577750, 0x57777755, 0x57777775,
    0x57733375, 0x57733375, 0x07733370, 0x00533300,
    0x00055000, 0x00055000, 0x00033000, 0x00033000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
};

const u32 spr_trevor_up[16] = {
    // Back view (bald spot visible)
    0x00055500, 0x05577750, 0x57777755, 0x57777775,
    0x57333375, 0x57333375, 0x07333370, 0x00533300,
    0x00055000, 0x00055000, 0x00033000, 0x00033000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
};

// KIP SIMPKINS - Walking right (4 frames)
// Long/spiky hair, heavy metal vibe
const u32 spr_kip_right[16] = {
    // Spiky hair on top
    0x000BB000, 0x00BBBB00, 0x0BBBBBB0, 0xBBBBBB44,
    0x4477BB44, 0x44777744, 0x44777744, 0x04777740,
    0x00777700, 0x00777700, 0x00077000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
};

const u32 spr_kip_left[16] = {
    // Mirror
    0x000BB000, 0x00BBBB00, 0x0BBBBBB0, 0x44BBBBBB,
    0x44BB7744, 0x44777744, 0x44777744, 0x04777740,
    0x00777700, 0x00777700, 0x00077000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
};

const u32 spr_kip_down[16] = {
    // Frontal with spiky hair
    0x000BBBB0, 0x00BBBBBB, 0x0BBBBBBB, 0x4BBBBBB4,
    0x44777744, 0x44777744, 0x04777740, 0x00777700,
    0x00077000, 0x00077000, 0x00033000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
};

const u32 spr_kip_up[16] = {
    // Back with spiky hair
    0x000BBBB0, 0x00BBBBBB, 0x0BBBBBBB, 0x4BBBBBB4,
    0x44777744, 0x44777744, 0x04777740, 0x00777700,
    0x00077000, 0x00077000, 0x00033000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
};

// JIM RUBIK - Walking right (4 frames)
// Round/stocky, straight-man appearance
const u32 spr_rubik_right[16] = {
    // Round head, stocky body
    0x00044400, 0x00444440, 0x04444444, 0x04444444,
    0x44777744, 0x44777744, 0x44777744, 0x04777740,
    0x00777700, 0x00777700, 0x00033000, 0x00033000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
};

const u32 spr_rubik_left[16] = {
    // Mirror
    0x00044400, 0x04444400, 0x44444440, 0x44444440,
    0x44777744, 0x44777744, 0x44777744, 0x04777740,
    0x00777700, 0x00777700, 0x00033000, 0x00033000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
};

const u32 spr_rubik_down[16] = {
    // Frontal
    0x00044400, 0x00444440, 0x04444444, 0x44444444,
    0x44777744, 0x44777744, 0x04777740, 0x00777700,
    0x00077000, 0x00077000, 0x00033000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
};

const u32 spr_rubik_up[16] = {
    // Back view
    0x00044400, 0x00444440, 0x04444444, 0x44444444,
    0x44777744, 0x44777744, 0x04777740, 0x00777700,
    0x00077000, 0x00077000, 0x00033000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
};

// GENERIC NPC - Walking right (4 frames)
// Simple humanoid for background characters
const u32 spr_generic_right[16] = {
    0x00044400, 0x00444440, 0x04444440, 0x44444440,
    0x44333344, 0x44333344, 0x44333344, 0x04333340,
    0x00333300, 0x00333300, 0x00033000, 0x00033000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
};

const u32 spr_generic_left[16] = {
    0x00044400, 0x04444400, 0x04444440, 0x04444444,
    0x44333344, 0x44333344, 0x44333344, 0x04333340,
    0x00333300, 0x00333300, 0x00033000, 0x00033000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
};

const u32 spr_generic_down[16] = {
    0x00044400, 0x00444440, 0x04444440, 0x44444444,
    0x44333344, 0x44333344, 0x04333340, 0x00333300,
    0x00033000, 0x00033000, 0x00033000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
};

const u32 spr_generic_up[16] = {
    0x00044400, 0x00444440, 0x04444440, 0x44444444,
    0x44333344, 0x44333344, 0x04333340, 0x00333300,
    0x00033000, 0x00033000, 0x00033000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
};

// ============================================================================
// URBAN TILESET (8x8 tiles, 4bpp)
// 64 tiles total
// ============================================================================

// Tile 0: Empty/Transparent
const u32 tile_empty[4] = { 0, 0, 0, 0 };

// Tile 1: Sidewalk (light grey)
const u32 tile_sidewalk[4] = { 0x33333333, 0x33333333, 0x33333333, 0x33333333 };

// Tile 2: Road (dark grey)
const u32 tile_road[4] = { 0x22222222, 0x22222222, 0x22222222, 0x22222222 };

// Tile 3: Grass (green)
const u32 tile_grass[4] = { 0x44444444, 0x44444444, 0x44444444, 0x44444444 };

// Tile 4: Brick wall (brown)
const u32 tile_brick[4] = { 0x77777777, 0x77177177, 0x77777777, 0x71717171 };

// Tile 5: Window (blue)
const u32 tile_window[4] = { 0x77777777, 0x78888877, 0x78888877, 0x77777777 };

// Tile 6: Door (dark brown)
const u32 tile_door[4] = { 0x77777777, 0x76666677, 0x76666677, 0x77777777 };

// Tile 7: Wall variant (light brown)
const u32 tile_wall_light[4] = { 0x88888888, 0x88188188, 0x88888888, 0x81818181 };

// Tile 8: Wood floor (brown)
const u32 tile_floor_wood[4] = { 0x55555555, 0x55155155, 0x55555555, 0x51515151 };

// Tile 9: Tile floor (light grey)
const u32 tile_floor_tile[4] = { 0x33333333, 0x33133133, 0x33333333, 0x31313131 };

// Tile 10: Carpet (reddish)
const u32 tile_carpet[4] = { 0xAAAAAAAA, 0xAA1A1A1A, 0xAAAAAAAA, 0xA1A1A1A1 };

// Tile 11: Concrete (blue-grey)
const u32 tile_concrete[4] = { 0x88888888, 0x88388388, 0x88888888, 0x83838383 };

// Tile 12: Desk (brown with detail)
const u32 tile_desk[4] = { 0x55555555, 0x55555555, 0x55115511, 0x55555555 };

// Tile 13: Chair (light brown)
const u32 tile_chair[4] = { 0x00555500, 0x05555550, 0x55555555, 0x05555550 };

// Tile 14: Table (brown)
const u32 tile_table[4] = { 0x00000000, 0x55555555, 0x55555555, 0x05555550 };

// Tile 15: Plant pot (green with brown pot)
const u32 tile_plant[4] = { 0x00444400, 0x04444440, 0x04555540, 0x00555500 };

// Tile 16: Door open left
const u32 tile_door_open_l[4] = { 0x77777777, 0x70000077, 0x70000077, 0x77777777 };

// Tile 17: Door open right
const u32 tile_door_open_r[4] = { 0x77777777, 0x77000007, 0x77000007, 0x77777777 };

// Tile 18: Doorway top
const u32 tile_doorway_top[4] = { 0x77777777, 0x77000077, 0x77000077, 0x77777777 };

// Tile 19: Doorway bottom
const u32 tile_doorway_bot[4] = { 0x77000077, 0x77000077, 0x77777777, 0x77777777 };

// Tile 20: Sign post
const u32 tile_sign[4] = { 0x00055500, 0x00555500, 0xCC555500, 0x00555500 };

// Tile 21: Lamp post
const u32 tile_lamp[4] = { 0x00099900, 0x00099900, 0x00055500, 0x00055500 };

// Tile 22: Trash can
const u32 tile_trash[4] = { 0x00AAAA00, 0x0AAAAAA0, 0x0AAAAAA0, 0x00AAAA00 };

// Tile 23: Fire hydrant (red)
const u32 tile_hydrant[4] = { 0x00AAA000, 0x0AAAAAA0, 0x0AA0AAA0, 0x00AAA000 };

// Tile 24-31: Variations of road markings, grass patches
const u32 tile_road_line[4] = { 0x22222222, 0x22999922, 0x22222222, 0x29999922 };

const u32 tile_grass_light[4] = { 0x44444444, 0x44C44C44, 0x44444444, 0x4C4C4C44 };

const u32 tile_grass_dark[4] = { 0x44444444, 0x44B44B44, 0x44444444, 0x4B4B4B44 };

const u32 tile_pavement[4] = { 0x33333333, 0x33233233, 0x33333333, 0x32323233 };

const u32 tile_gravel[4] = { 0x22222222, 0x22322322, 0x22222222, 0x23232322 };

const u32 tile_dirt[4] = { 0x55555555, 0x55755755, 0x55555555, 0x57575755 };

const u32 tile_shadow[4] = { 0x00000000, 0x00111100, 0x01111110, 0x00111100 };

const u32 tile_water[4] = { 0x88888888, 0x88088088, 0x88888888, 0x80808088 };

// Tile 32-47: Interior variations
const u32 tile_wall_corner[4] = { 0x77777777, 0x77777777, 0x77700077, 0x77700077 };

const u32 tile_shelf[4] = { 0x55555555, 0x55555555, 0x51515151, 0x55555555 };

const u32 tile_cabinet[4] = { 0x66666666, 0x61616166, 0x66666666, 0x61616166 };

const u32 tile_counter[4] = { 0x77777777, 0x77777777, 0x71717171, 0x77777777 };

const u32 tile_bed[4] = { 0x55555555, 0x55555555, 0x55115511, 0x55115511 };

const u32 tile_sofa[4] = { 0x55555555, 0x51515151, 0x55555555, 0x51515151 };

const u32 tile_sink[4] = { 0x88888888, 0x88088088, 0x80008008, 0x88888888 };

const u32 tile_stove[4] = { 0x66666666, 0x61116166, 0x61116166, 0x66666666 };

const u32 tile_fridge[4] = { 0x88888888, 0x81811188, 0x81811188, 0x88888888 };

const u32 tile_toilet[4] = { 0x00333300, 0x03333330, 0x03088830, 0x00333300 };

const u32 tile_bathtub[4] = { 0x08888880, 0x88888888, 0x88888888, 0x08888880 };

const u32 tile_mirror[4] = { 0x88888888, 0x80008008, 0x80008008, 0x88888888 };

const u32 tile_bookshelf[4] = { 0xCCCCCCCC, 0xC1C1C1CC, 0xCCCCCCCC, 0xC1C1C1CC };

const u32 tile_painting[4] = { 0x77777777, 0x7CCCCCC7, 0x7CCCCCC7, 0x77777777 };

// Tile 48-63: Additional variations
const u32 tile_metal_wall[4] = { 0x22222222, 0x22122122, 0x22222222, 0x21212122 };

const u32 tile_glass[4] = { 0x88888888, 0x80088008, 0x88088088, 0x08008088 };

const u32 tile_stone[4] = { 0x77777777, 0x77177177, 0x77777777, 0x71717171 };

const u32 tile_marble[4] = { 0x33333333, 0x33233233, 0x33333333, 0x32323233 };

const u32 tile_carpet_fancy[4] = { 0xAAAAAAAA, 0xA2A2A2AA, 0xAAAAAAAA, 0xA2A2A2AA };

const u32 tile_parquet[4] = { 0x55555555, 0x55255255, 0x55555555, 0x52525255 };

const u32 tile_stairs[4] = { 0x55555555, 0x55445555, 0x55444555, 0x55444455 };

const u32 tile_railing[4] = { 0x77777777, 0x71717177, 0x77777777, 0x71717171 };

const u32 tile_hedge[4] = { 0x44444444, 0x4C4C4C44, 0x4C4C4C44, 0x44444444 };

const u32 tile_tree[4] = { 0x00C44C00, 0x0C4444C0, 0x0C4444C0, 0x00555500 };

const u32 tile_bench[4] = { 0x00555500, 0x05555550, 0x00055000, 0x00055000 };

const u32 tile_fountain[4] = { 0x00888800, 0x08888880, 0x08088080, 0x00888800 };

const u32 tile_path[4] = { 0x33333333, 0x33133133, 0x33333333, 0x31313131 };

const u32 tile_roof[4] = { 0x10101010, 0x11111111, 0x11111111, 0x10101010 };

const u32 tile_vent[4] = { 0x77777777, 0x71211217, 0x71211217, 0x77777777 };

// Collect all tileset tiles
const u32* tileset_urban[64] = {
    tile_empty, tile_sidewalk, tile_road, tile_grass,
    tile_brick, tile_window, tile_door, tile_wall_light,
    tile_floor_wood, tile_floor_tile, tile_carpet, tile_concrete,
    tile_desk, tile_chair, tile_table, tile_plant,
    tile_door_open_l, tile_door_open_r, tile_doorway_top, tile_doorway_bot,
    tile_sign, tile_lamp, tile_trash, tile_hydrant,
    tile_road_line, tile_grass_light, tile_grass_dark, tile_pavement,
    tile_gravel, tile_dirt, tile_shadow, tile_water,
    tile_wall_corner, tile_shelf, tile_cabinet, tile_counter,
    tile_bed, tile_sofa, tile_sink, tile_stove,
    tile_fridge, tile_toilet, tile_bathtub, tile_mirror,
    tile_bookshelf, tile_painting, tile_metal_wall, tile_glass,
    tile_stone, tile_marble, tile_carpet_fancy, tile_parquet,
    tile_stairs, tile_railing, tile_hedge, tile_tree,
    tile_bench, tile_fountain, tile_path, tile_roof,
    tile_vent, tile_sidewalk, tile_road, tile_grass
};

// ============================================================================
// MAP 1: DFWTF Headquarters (32x32 tiles)
// Layout: Briefing room, desks, hallway, interrogation room
// ============================================================================

const u8 map_dfwtf_hq[32 * 32] = {
    // Row 0
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,

    // Row 1
    4, 9, 9, 9, 9, 9, 9, 9, 4, 18, 18, 18, 4, 9, 9, 9,
    9, 9, 9, 9, 4, 12, 12, 12, 4, 9, 9, 9, 4, 4, 4, 4,

    // Row 2
    4, 9, 13, 14, 14, 14, 12, 9, 4, 19, 6, 19, 4, 9, 12, 14,
    12, 12, 12, 9, 4, 12, 12, 12, 4, 9, 9, 9, 4, 4, 4, 4,

    // Row 3
    4, 9, 12, 12, 12, 12, 12, 9, 4, 9, 9, 9, 4, 9, 12, 12,
    12, 12, 12, 9, 4, 12, 12, 12, 4, 9, 13, 9, 4, 4, 4, 4,

    // Row 4
    4, 9, 12, 12, 12, 12, 12, 9, 4, 9, 12, 12, 4, 9, 12, 12,
    12, 12, 12, 9, 4, 12, 12, 12, 4, 9, 9, 9, 4, 4, 4, 4,

    // Row 5
    4, 9, 12, 12, 12, 12, 12, 9, 4, 9, 12, 12, 4, 9, 12, 12,
    12, 12, 12, 9, 4, 12, 14, 12, 4, 9, 9, 9, 4, 4, 4, 4,

    // Row 6
    4, 9, 9, 9, 9, 9, 9, 9, 4, 9, 12, 12, 4, 9, 9, 9,
    9, 9, 9, 9, 4, 9, 9, 9, 4, 9, 9, 9, 4, 4, 4, 4,

    // Row 7 (hallway)
    4, 9, 9, 9, 9, 9, 9, 9, 1, 9, 9, 9, 1, 9, 9, 9,
    9, 9, 9, 9, 1, 9, 9, 9, 1, 9, 9, 9, 4, 4, 4, 4,

    // Row 8 (hallway continues)
    4, 9, 9, 9, 9, 9, 9, 9, 1, 9, 9, 9, 1, 9, 9, 9,
    9, 9, 9, 9, 1, 9, 9, 9, 1, 9, 9, 9, 4, 4, 4, 4,

    // Row 9
    4, 9, 9, 9, 9, 9, 9, 9, 4, 9, 9, 9, 4, 9, 9, 9,
    9, 9, 9, 9, 4, 9, 9, 9, 4, 9, 9, 9, 4, 4, 4, 4,

    // Row 10
    4, 9, 12, 12, 12, 12, 12, 9, 4, 9, 13, 9, 4, 9, 12, 12,
    12, 12, 12, 9, 4, 12, 12, 12, 4, 9, 9, 9, 4, 4, 4, 4,

    // Row 11
    4, 9, 12, 12, 12, 12, 12, 9, 4, 9, 9, 9, 4, 9, 12, 12,
    12, 12, 12, 9, 4, 12, 12, 12, 4, 9, 13, 9, 4, 4, 4, 4,

    // Row 12
    4, 9, 12, 12, 12, 12, 12, 9, 4, 9, 12, 12, 4, 9, 12, 12,
    12, 12, 12, 9, 4, 12, 12, 12, 4, 9, 9, 9, 4, 4, 4, 4,

    // Row 13
    4, 9, 9, 9, 9, 9, 9, 9, 4, 9, 12, 12, 4, 9, 9, 9,
    9, 9, 9, 9, 4, 12, 14, 12, 4, 9, 9, 9, 4, 4, 4, 4,

    // Row 14
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,

    // Row 15-31 (additional levels/rooms - keep same pattern for brevity)
    4, 9, 9, 9, 9, 9, 9, 9, 4, 9, 9, 9, 4, 9, 9, 9,
    9, 9, 9, 9, 4, 9, 9, 9, 4, 9, 9, 9, 4, 4, 4, 4,

    4, 9, 12, 12, 12, 12, 12, 9, 4, 9, 12, 12, 4, 9, 12, 12,
    12, 12, 12, 9, 4, 12, 12, 12, 4, 9, 9, 9, 4, 4, 4, 4,

    4, 9, 12, 12, 12, 12, 12, 9, 4, 9, 12, 12, 4, 9, 12, 12,
    12, 12, 12, 9, 4, 12, 12, 12, 4, 9, 13, 9, 4, 4, 4, 4,

    4, 9, 12, 12, 12, 12, 12, 9, 4, 9, 9, 9, 4, 9, 12, 12,
    12, 12, 12, 9, 4, 9, 9, 9, 4, 9, 9, 9, 4, 4, 4, 4,

    4, 9, 9, 9, 9, 9, 9, 9, 4, 9, 9, 9, 4, 9, 9, 9,
    9, 9, 9, 9, 4, 9, 9, 9, 4, 9, 9, 9, 4, 4, 4, 4,

    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,

    4, 9, 9, 9, 9, 9, 9, 9, 4, 9, 9, 9, 4, 9, 9, 9,
    9, 9, 9, 9, 4, 9, 9, 9, 4, 9, 9, 9, 4, 4, 4, 4,

    4, 9, 12, 12, 12, 12, 12, 9, 4, 9, 12, 12, 4, 9, 12, 12,
    12, 12, 12, 9, 4, 12, 12, 12, 4, 9, 9, 9, 4, 4, 4, 4,

    4, 9, 12, 12, 12, 12, 12, 9, 4, 9, 12, 12, 4, 9, 12, 12,
    12, 12, 12, 9, 4, 12, 12, 12, 4, 9, 13, 9, 4, 4, 4, 4,

    4, 9, 12, 12, 12, 12, 12, 9, 4, 9, 9, 9, 4, 9, 12, 12,
    12, 12, 12, 9, 4, 9, 9, 9, 4, 9, 9, 9, 4, 4, 4, 4,

    4, 9, 9, 9, 9, 9, 9, 9, 4, 9, 9, 9, 4, 9, 9, 9,
    9, 9, 9, 9, 4, 9, 9, 9, 4, 9, 9, 9, 4, 4, 4, 4,

    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
};

// Collision layer for DFWTF HQ (1 = solid, 0 = walkable)
const u8 map_dfwtf_hq_collision[32 * 32] = {
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,

    1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0,
    0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 1,

    1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 0, 0,
    0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 1,

    1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0,
    0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 1, 1, 1,

    1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0,
    0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 1,

    1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0,
    0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 1,

    1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0,
    0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 1,

    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1,

    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1,

    1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0,
    0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 1,

    1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 1, 0, 0, 0,
    0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 1,

    1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0,
    0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 1, 1, 1,

    1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0,
    0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 1,

    1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0,
    0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 1,

    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,

    1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0,
    0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 1,

    1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0,
    0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 1,

    1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0,
    0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 1, 1, 1,

    1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0,
    0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 1,

    1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0,
    0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 1,

    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,

    1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0,
    0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 1,

    1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0,
    0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 1,

    1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0,
    0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 1, 1, 1,

    1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0,
    0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 1,

    1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0,
    0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 1,

    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
};

// ============================================================================
// MAP 2: DFW Streets (32x32 tiles)
// Layout: Urban environment with roads, sidewalks, buildings, parking
// ============================================================================

const u8 map_dfw_streets[32 * 32] = {
    4, 7, 7, 7, 4, 7, 7, 7, 4, 7, 7, 7, 4, 7, 7, 7,
    4, 7, 7, 7, 4, 7, 7, 7, 4, 7, 7, 7, 4, 7, 7, 7,

    4, 7, 5, 5, 4, 7, 5, 5, 4, 7, 5, 5, 4, 7, 5, 5,
    4, 7, 5, 5, 4, 7, 5, 5, 4, 7, 5, 5, 4, 7, 5, 5,

    4, 7, 5, 5, 4, 7, 5, 5, 4, 7, 5, 5, 4, 7, 5, 5,
    4, 7, 5, 5, 4, 7, 5, 5, 4, 7, 5, 5, 4, 7, 5, 5,

    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,

    4, 7, 7, 7, 4, 7, 7, 7, 4, 7, 7, 7, 4, 7, 7, 7,
    4, 7, 7, 7, 4, 7, 7, 7, 4, 7, 7, 7, 4, 7, 7, 7,

    4, 7, 5, 5, 4, 7, 5, 5, 4, 7, 5, 5, 4, 7, 5, 5,
    4, 7, 5, 5, 4, 7, 5, 5, 4, 7, 5, 5, 4, 7, 5, 5,

    4, 7, 5, 5, 4, 7, 5, 5, 4, 7, 5, 5, 4, 7, 5, 5,
    4, 7, 5, 5, 4, 7, 5, 5, 4, 7, 5, 5, 4, 7, 5, 5,

    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,

    4, 7, 7, 7, 4, 7, 7, 7, 4, 7, 7, 7, 4, 7, 7, 7,
    4, 7, 7, 7, 4, 7, 7, 7, 4, 7, 7, 7, 4, 7, 7, 7,

    4, 7, 5, 5, 4, 7, 5, 5, 4, 7, 5, 5, 4, 7, 5, 5,
    4, 7, 5, 5, 4, 7, 5, 5, 4, 7, 5, 5, 4, 7, 5, 5,

    4, 7, 5, 5, 4, 7, 5, 5, 4, 7, 5, 5, 4, 7, 5, 5,
    4, 7, 5, 5, 4, 7, 5, 5, 4, 7, 5, 5, 4, 7, 5, 5,

    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,

    4, 7, 7, 7, 4, 7, 7, 7, 4, 7, 7, 7, 4, 7, 7, 7,
    4, 7, 7, 7, 4, 7, 7, 7, 4, 7, 7, 7, 4, 7, 7, 7,

    4, 7, 5, 5, 4, 7, 5, 5, 4, 7, 5, 5, 4, 7, 5, 5,
    4, 7, 5, 5, 4, 7, 5, 5, 4, 7, 5, 5, 4, 7, 5, 5,

    4, 7, 5, 5, 4, 7, 5, 5, 4, 7, 5, 5, 4, 7, 5, 5,
    4, 7, 5, 5, 4, 7, 5, 5, 4, 7, 5, 5, 4, 7, 5, 5,

    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,

    4, 7, 7, 7, 4, 7, 7, 7, 4, 7, 7, 7, 4, 7, 7, 7,
    4, 7, 7, 7, 4, 7, 7, 7, 4, 7, 7, 7, 4, 7, 7, 7,
};

// Collision for streets
const u8 map_dfw_streets_collision[32 * 32] = {
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,

    1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1,
    1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1,

    1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1,
    1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1,

    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,

    1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1,
    1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1,

    1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1,
    1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1,

    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,

    1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1,
    1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1,

    1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1,
    1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1,

    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,

    1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1,
    1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1,

    1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1,
    1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1,

    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
};

// ============================================================================
// MAP 3: Taco-Bongo Restaurant (20x20 tiles)
// Small intimate space: tables, bar, kitchen
// ============================================================================

const u8 map_taco_bongo[20 * 20] = {
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    4, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 4,
    4, 8, 14, 14, 8, 14, 14, 8, 14, 14, 8, 14, 14, 8, 14, 14, 8, 8, 8, 4,
    4, 8, 14, 14, 8, 14, 14, 8, 14, 14, 8, 14, 14, 8, 14, 14, 8, 8, 8, 4,
    4, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 4,
    4, 8, 14, 14, 8, 14, 14, 8, 14, 14, 8, 14, 14, 8, 14, 14, 8, 8, 8, 4,
    4, 8, 14, 14, 8, 14, 14, 8, 14, 14, 8, 14, 14, 8, 14, 14, 8, 8, 8, 4,
    4, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 4,
    4, 8, 14, 14, 8, 14, 14, 8, 14, 14, 8, 14, 14, 8, 14, 14, 8, 8, 8, 4,
    4, 8, 14, 14, 8, 14, 14, 8, 14, 14, 8, 14, 14, 8, 14, 14, 8, 8, 8, 4,
    4, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 35, 35, 8, 4,
    4, 8, 35, 35, 35, 35, 35, 8, 35, 35, 35, 35, 35, 8, 35, 35, 8, 8, 8, 4,
    4, 8, 35, 35, 35, 35, 35, 8, 35, 35, 35, 35, 35, 8, 35, 35, 8, 8, 8, 4,
    4, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 4,
    4, 8, 35, 35, 8, 35, 35, 8, 35, 35, 8, 35, 35, 8, 35, 35, 8, 8, 8, 4,
    4, 8, 35, 35, 8, 35, 35, 8, 35, 35, 8, 35, 35, 8, 35, 35, 8, 8, 8, 4,
    4, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 4,
    4, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 4,
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
};

// Collision for restaurant
const u8 map_taco_bongo_collision[20 * 20] = {
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 0, 0, 1,
    1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 0, 0, 1,
    1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 0, 0, 1,
    1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1,
    1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 0, 0, 0, 1,
    1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 0, 0, 1,
    1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
};

// ============================================================================
// DIALOGUE TREES
// Maximum 8 trees (0-7), where 0 is unused/null
// Each tree has up to 8 nodes with up to 4 choices
// ============================================================================

// Tree 1: Jim Rubik's introduction at DFWTF HQ
const DialogueNode dlg_rubik_intro_nodes[5] = {
    // Node 0: Opening line
    {
        .text = "Welcome to DFWTF, partners. I'm Jim Rubik.",
        .choices = {
            { .text = "Greetings, citizen.", .next_node = 1, .fate_reward = 1 },
            { .text = "What's DFWTF?", .next_node = 2, .fate_reward = 0 },
            { .text = "*nods while sunglasses reflect* OK.", .next_node = 2, .fate_reward = 2 },
            { .text = "", .next_node = 0, .fate_reward = 0 }
        }
    },
    // Node 1: Respond to James Bond impression
    {
        .text = "That's... not really how we do things here.",
        .choices = {
            { .text = "Moving on.", .next_node = 3, .fate_reward = 0 },
            { .text = "", .next_node = 0, .fate_reward = 0 },
            { .text = "", .next_node = 0, .fate_reward = 0 },
            { .text = "", .next_node = 0, .fate_reward = 0 }
        }
    },
    // Node 2: Explain DFWTF
    {
        .text = "Dallas Fort Worth Task Force. Catch bad guys.",
        .choices = {
            { .text = "Say no more. I got this.", .next_node = 3, .fate_reward = 1 },
            { .text = "So like... regular cops?", .next_node = 3, .fate_reward = 2 },
            { .text = "", .next_node = 0, .fate_reward = 0 },
            { .text = "", .next_node = 0, .fate_reward = 0 }
        }
    },
    // Node 3: End
    {
        .text = "Your first case is waiting.",
        .choices = {
            { .text = "", .next_node = 0, .fate_reward = 0 },
            { .text = "", .next_node = 0, .fate_reward = 0 },
            { .text = "", .next_node = 0, .fate_reward = 0 },
            { .text = "", .next_node = 0, .fate_reward = 0 }
        }
    },
    // Node 4: Unused
    {
        .text = "",
        .choices = {
            { .text = "", .next_node = 0, .fate_reward = 0 },
            { .text = "", .next_node = 0, .fate_reward = 0 },
            { .text = "", .next_node = 0, .fate_reward = 0 },
            { .text = "", .next_node = 0, .fate_reward = 0 }
        }
    }
};

// Tree 2: Shauna Peacemaker mission briefing
const DialogueNode dlg_peacemaker_nodes[4] = {
    // Node 0: Opening
    {
        .text = "Listen up. There's been a robbery.",
        .choices = {
            { .text = "Details?", .next_node = 1, .fate_reward = 0 },
            { .text = "*scratches head*", .next_node = 1, .fate_reward = 1 },
            { .text = "Was it the Taco-Bongo?", .next_node = 2, .fate_reward = 2 },
            { .text = "", .next_node = 0, .fate_reward = 0 }
        }
    },
    // Node 1: Facts
    {
        .text = "Jewelry store on Main. You track the perp.",
        .choices = {
            { .text = "On it.", .next_node = 3, .fate_reward = 1 },
            { .text = "What did they steal?", .next_node = 3, .fate_reward = 0 },
            { .text = "", .next_node = 0, .fate_reward = 0 },
            { .text = "", .next_node = 0, .fate_reward = 0 }
        }
    },
    // Node 2: Wrong location
    {
        .text = "No, the Taco-Bongo is sacred. Focus.",
        .choices = {
            { .text = "Right, sorry.", .next_node = 3, .fate_reward = 0 },
            { .text = "", .next_node = 0, .fate_reward = 0 },
            { .text = "", .next_node = 0, .fate_reward = 0 },
            { .text = "", .next_node = 0, .fate_reward = 0 }
        }
    },
    // Node 3: Dismissed
    {
        .text = "Go. Report back when you find something.",
        .choices = {
            { .text = "", .next_node = 0, .fate_reward = 0 },
            { .text = "", .next_node = 0, .fate_reward = 0 },
            { .text = "", .next_node = 0, .fate_reward = 0 },
            { .text = "", .next_node = 0, .fate_reward = 0 }
        }
    }
};

// Tree 3: Donny's Taco-Bongo conversation
const DialogueNode dlg_donny_nodes[5] = {
    // Node 0: Opening
    {
        .text = "Guys! Time for a proper boys' night out!",
        .choices = {
            { .text = "Let's order tacos!", .next_node = 1, .fate_reward = 2 },
            { .text = "We're on a case.", .next_node = 2, .fate_reward = 0 },
            { .text = "Define 'boys night'.", .next_node = 1, .fate_reward = 1 },
            { .text = "", .next_node = 0, .fate_reward = 0 }
        }
    },
    // Node 1: Taco options
    {
        .text = "I'm thinking... surf-and-turf tacos?",
        .choices = {
            { .text = "That's a thing?", .next_node = 3, .fate_reward = 2 },
            { .text = "Sounds delicious.", .next_node = 3, .fate_reward = 1 },
            { .text = "Let's get deep fried oreos too.", .next_node = 4, .fate_reward = 2 },
            { .text = "", .next_node = 0, .fate_reward = 0 }
        }
    },
    // Node 2: Work focus
    {
        .text = "The case can wait an hour. Come on!",
        .choices = {
            { .text = "You got it, Donny.", .next_node = 1, .fate_reward = 1 },
            { .text = "DFWTF comes first.", .next_node = 3, .fate_reward = 0 },
            { .text = "", .next_node = 0, .fate_reward = 0 },
            { .text = "", .next_node = 0, .fate_reward = 0 }
        }
    },
    // Node 3: Accept
    {
        .text = "Let's taco-bongo our feelings!",
        .choices = {
            { .text = "", .next_node = 0, .fate_reward = 0 },
            { .text = "", .next_node = 0, .fate_reward = 0 },
            { .text = "", .next_node = 0, .fate_reward = 0 },
            { .text = "", .next_node = 0, .fate_reward = 0 }
        }
    },
    // Node 4: Even better
    {
        .text = "Now THAT'S what I call a night!",
        .choices = {
            { .text = "", .next_node = 0, .fate_reward = 0 },
            { .text = "", .next_node = 0, .fate_reward = 0 },
            { .text = "", .next_node = 0, .fate_reward = 0 },
            { .text = "", .next_node = 0, .fate_reward = 0 }
        }
    }
};

// Tree 4: Ramis and the R.A.D. situation
const DialogueNode dlg_ramis_nodes[4] = {
    // Node 0: The problem
    {
        .text = "Uh... so I may have lost R.A.D.",
        .choices = {
            { .text = "R.A.D.?", .next_node = 1, .fate_reward = 0 },
            { .text = "The robot? HOW?", .next_node = 2, .fate_reward = 2 },
            { .text = "*sigh* Of course you did.", .next_node = 2, .fate_reward = 1 },
            { .text = "", .next_node = 0, .fate_reward = 0 }
        }
    },
    // Node 1: Explanation
    {
        .text = "My experimental robotic assistant droid.",
        .choices = {
            { .text = "And you lost it?", .next_node = 2, .fate_reward = 2 },
            { .text = "Tell us more.", .next_node = 3, .fate_reward = 0 },
            { .text = "", .next_node = 0, .fate_reward = 0 },
            { .text = "", .next_node = 0, .fate_reward = 0 }
        }
    },
    // Node 2: His shame
    {
        .text = "It got stolen. Cost me $50K.",
        .choices = {
            { .text = "Ouch.", .next_node = 3, .fate_reward = 1 },
            { .text = "", .next_node = 0, .fate_reward = 0 },
            { .text = "", .next_node = 0, .fate_reward = 0 },
            { .text = "", .next_node = 0, .fate_reward = 0 }
        }
    },
    // Node 3: Case given
    {
        .text = "Find R.A.D. and you're heroes.",
        .choices = {
            { .text = "", .next_node = 0, .fate_reward = 0 },
            { .text = "", .next_node = 0, .fate_reward = 0 },
            { .text = "", .next_node = 0, .fate_reward = 0 },
            { .text = "", .next_node = 0, .fate_reward = 0 }
        }
    }
};

// Tree 5: Random cop #1
const DialogueNode dlg_cop1_nodes[2] = {
    {
        .text = "You hear about the new precinct policy?",
        .choices = {
            { .text = "No, what is it?", .next_node = 1, .fate_reward = 0 },
            { .text = "Don't care.", .next_node = 1, .fate_reward = 1 },
            { .text = "", .next_node = 0, .fate_reward = 0 },
            { .text = "", .next_node = 0, .fate_reward = 0 }
        }
    },
    {
        .text = "Mandatory taco Fridays. Life's good.",
        .choices = {
            { .text = "", .next_node = 0, .fate_reward = 0 },
            { .text = "", .next_node = 0, .fate_reward = 0 },
            { .text = "", .next_node = 0, .fate_reward = 0 },
            { .text = "", .next_node = 0, .fate_reward = 0 }
        }
    }
};

// Tree 6: Random cop #2
const DialogueNode dlg_cop2_nodes[2] = {
    {
        .text = "I saw a suspicious character near the park.",
        .choices = {
            { .text = "Description?", .next_node = 1, .fate_reward = 0 },
            { .text = "Was it a duck?", .next_node = 1, .fate_reward = 2 },
            { .text = "", .next_node = 0, .fate_reward = 0 },
            { .text = "", .next_node = 0, .fate_reward = 0 }
        }
    },
    {
        .text = "Tall, shady. Looked like trouble.",
        .choices = {
            { .text = "", .next_node = 0, .fate_reward = 0 },
            { .text = "", .next_node = 0, .fate_reward = 0 },
            { .text = "", .next_node = 0, .fate_reward = 0 },
            { .text = "", .next_node = 0, .fate_reward = 0 }
        }
    }
};

// Tree 7: Street informant
const DialogueNode dlg_informant_nodes[3] = {
    {
        .text = "Psst... you looking for information?",
        .choices = {
            { .text = "Maybe. You got some?", .next_node = 1, .fate_reward = 0 },
            { .text = "What's your angle?", .next_node = 1, .fate_reward = 1 },
            { .text = "", .next_node = 0, .fate_reward = 0 },
            { .text = "", .next_node = 0, .fate_reward = 0 }
        }
    },
    {
        .text = "Heard some folks talking about R.A.D.",
        .choices = {
            { .text = "Where?", .next_node = 2, .fate_reward = 0 },
            { .text = "That robot?", .next_node = 2, .fate_reward = 1 },
            { .text = "", .next_node = 0, .fate_reward = 0 },
            { .text = "", .next_node = 0, .fate_reward = 0 }
        }
    },
    {
        .text = "Warehouse on 5th. Now beat it.",
        .choices = {
            { .text = "", .next_node = 0, .fate_reward = 0 },
            { .text = "", .next_node = 0, .fate_reward = 0 },
            { .text = "", .next_node = 0, .fate_reward = 0 },
            { .text = "", .next_node = 0, .fate_reward = 0 }
        }
    }
};

// Array of dialogue trees (indexed 1-7, index 0 unused)
const DialogueTree dialogue_trees[8] = {
    { .num_nodes = 0, .nodes = NULL },  // Tree 0: Unused
    { .num_nodes = 5, .nodes = dlg_rubik_intro_nodes },  // Tree 1: Rubik intro
    { .num_nodes = 4, .nodes = dlg_peacemaker_nodes },   // Tree 2: Peacemaker
    { .num_nodes = 5, .nodes = dlg_donny_nodes },        // Tree 3: Donny
    { .num_nodes = 4, .nodes = dlg_ramis_nodes },        // Tree 4: Ramis
    { .num_nodes = 2, .nodes = dlg_cop1_nodes },         // Tree 5: Cop 1
    { .num_nodes = 2, .nodes = dlg_cop2_nodes },         // Tree 6: Cop 2
    { .num_nodes = 3, .nodes = dlg_informant_nodes },    // Tree 7: Informant
};

// ============================================================================
// CHARACTER DATA
// ============================================================================

const Character characters[4] = {
    // Trevor Steel
    {
        .id = CHAR_TREVOR,
        .name = "Trevor Steel",
        .sprite_pal = spr_pal_characters,
        .sprite_right = spr_trevor_right,
        .sprite_left = spr_trevor_left,
        .sprite_down = spr_trevor_down,
        .sprite_up = spr_trevor_up,
        .max_hp = 15,
        .max_fate = 5
    },
    // Kip Simpkins
    {
        .id = CHAR_KIP,
        .name = "Kip Simpkins",
        .sprite_pal = spr_pal_characters,
        .sprite_right = spr_kip_right,
        .sprite_left = spr_kip_left,
        .sprite_down = spr_kip_down,
        .sprite_up = spr_kip_up,
        .max_hp = 12,
        .max_fate = 6
    },
    // Jim Rubik
    {
        .id = CHAR_RUBIK,
        .name = "Jim Rubik",
        .sprite_pal = spr_pal_characters,
        .sprite_right = spr_rubik_right,
        .sprite_left = spr_rubik_left,
        .sprite_down = spr_rubik_down,
        .sprite_up = spr_rubik_up,
        .max_hp = 14,
        .max_fate = 4
    },
    // Generic NPC
    {
        .id = CHAR_NPC,
        .name = "Citizen",
        .sprite_pal = spr_pal_characters,
        .sprite_right = spr_generic_right,
        .sprite_left = spr_generic_left,
        .sprite_down = spr_generic_down,
        .sprite_up = spr_generic_up,
        .max_hp = 8,
        .max_fate = 2
    }
};

// ============================================================================
// NPC DEFINITIONS
// ============================================================================

const NPC npcs[8] = {
    // NPC 0: Jim Rubik at HQ
    {
        .id = 0,
        .name = "Jim Rubik",
        .char_id = CHAR_RUBIK,
        .map = 0,  // DFWTF HQ
        .x = 10,
        .y = 5,
        .dialogue_tree = 1,
        .is_essential = 1
    },
    // NPC 1: Shauna Peacemaker at HQ
    {
        .id = 1,
        .name = "Shauna Peacemaker",
        .char_id = CHAR_NPC,
        .map = 0,
        .x = 20,
        .y = 10,
        .dialogue_tree = 2,
        .is_essential = 1
    },
    // NPC 2: Donny at Taco-Bongo
    {
        .id = 2,
        .name = "Donny",
        .char_id = CHAR_NPC,
        .map = 2,
        .x = 10,
        .y = 5,
        .dialogue_tree = 3,
        .is_essential = 0
    },
    // NPC 3: Ramis at HQ
    {
        .id = 3,
        .name = "Ramis",
        .char_id = CHAR_NPC,
        .map = 0,
        .x = 15,
        .y = 15,
        .dialogue_tree = 4,
        .is_essential = 1
    },
    // NPC 4: Random cop at HQ
    {
        .id = 4,
        .name = "Patrol Officer",
        .char_id = CHAR_NPC,
        .map = 0,
        .x = 5,
        .y = 12,
        .dialogue_tree = 5,
        .is_essential = 0
    },
    // NPC 5: Random cop at streets
    {
        .id = 5,
        .name = "Patrol Officer",
        .char_id = CHAR_NPC,
        .map = 1,
        .x = 16,
        .y = 20,
        .dialogue_tree = 6,
        .is_essential = 0
    },
    // NPC 6: Street informant
    {
        .id = 6,
        .name = "Informant",
        .char_id = CHAR_NPC,
        .map = 1,
        .x = 25,
        .y = 8,
        .dialogue_tree = 7,
        .is_essential = 0
    },
    // NPC 7: Unused
    {
        .id = 7,
        .name = "",
        .char_id = CHAR_NPC,
        .map = 0,
        .x = 0,
        .y = 0,
        .dialogue_tree = 0,
        .is_essential = 0
    }
};

// ============================================================================
// MAP DEFINITIONS
// ============================================================================

const Map maps[3] = {
    // Map 0: DFWTF HQ
    {
        .id = 0,
        .name = "DFWTF Headquarters",
        .width = 32,
        .height = 32,
        .tiles = map_dfwtf_hq,
        .collision = map_dfwtf_hq_collision,
        .tileset_pal = tileset_urban_pal,
        .tileset = tileset_urban
    },
    // Map 1: DFW Streets
    {
        .id = 1,
        .name = "DFW Streets",
        .width = 32,
        .height = 32,
        .tiles = map_dfw_streets,
        .collision = map_dfw_streets_collision,
        .tileset_pal = tileset_urban_pal,
        .tileset = tileset_urban
    },
    // Map 2: Taco-Bongo Restaurant
    {
        .id = 2,
        .name = "Taco-Bongo",
        .width = 20,
        .height = 20,
        .tiles = map_taco_bongo,
        .collision = map_taco_bongo_collision,
        .tileset_pal = tileset_urban_pal,
        .tileset = tileset_urban
    }
};
