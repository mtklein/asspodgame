// test_main.c — Automated test suite for AS&S Game
// This replaces main.c when building the test ROM.
// Tests exercise all game systems and write results to:
//   1. mGBA debug console (visible in emulator log)
//   2. SRAM at 0x0E000000 (persisted as .sav file for automated checking)

#include "gba.h"
#include "game.h"
#include "input.h"
#include "sprite.h"
#include "map.h"
#include "entity.h"
#include "dialogue.h"
#include "combat.h"
#include "game_data.h"
#include "mgba_log.h"

// --- SRAM result protocol ---
// Byte 0-3: magic "TEST"
// Byte 4:   total test count
// Byte 5:   pass count
// Byte 6:   fail count
// Byte 7:   done flag (0xAA = all tests ran)
// Byte 8+:  per-test results (0=pass, 1=fail), one byte each

#define SRAM       ((volatile u8*)0x0E000000)
#define MAX_TESTS  128

static int test_count = 0;
static int pass_count = 0;
static int fail_count = 0;

static void sram_write_byte(int offset, u8 val) {
    SRAM[offset] = val;
}

static void record_result(int passed, const char *name) {
    int idx = test_count;
    test_count++;

    if (passed) {
        pass_count++;
        mgba_log(MGBA_LOG_INFO, name);
        mgba_log(MGBA_LOG_INFO, "  PASS");
    } else {
        fail_count++;
        mgba_log(MGBA_LOG_ERROR, name);
        mgba_log(MGBA_LOG_ERROR, "  FAIL");
    }

    // Write to SRAM
    if (idx < MAX_TESTS) {
        sram_write_byte(8 + idx, passed ? 0 : 1);
    }
}

// Simulate pressing a key for one frame
// Must be called BEFORE game_update() — input_test_mode prevents
// input_poll() from overwriting these values with hardware state.
static void sim_key(u16 key) {
    key_prev = key_cur;
    key_cur = key;
}

static void sim_no_key(void) {
    key_prev = key_cur;
    key_cur = 0;
}

// ============================================================================
// TEST CASES
// ============================================================================

static void test_game_init(void) {
    game_init();
    record_result(game_state == STATE_TITLE, "game_init: state is TITLE");
}

static void test_entity_init(void) {
    entity_init();
    // All entities should be inactive
    int all_inactive = 1;
    for (int i = 0; i < MAX_ENTITIES; i++) {
        if (entities[i].active) { all_inactive = 0; break; }
    }
    record_result(all_inactive, "entity_init: all entities inactive");
}

static void test_entity_create(void) {
    entity_init();
    int id = entity_create(ENT_PLAYER_TREVOR, 100, 200);
    record_result(id >= 0, "entity_create: returns valid id");
    record_result(entities[id].type == ENT_PLAYER_TREVOR, "entity_create: type set");
    record_result(entities[id].x == 100 && entities[id].y == 200, "entity_create: position set");
    record_result(entities[id].active == 1, "entity_create: active flag set");
    record_result(entities[id].dir == DIR_DOWN, "entity_create: default dir DOWN");
}

static void test_entity_create_fills_slots(void) {
    entity_init();
    // Fill all slots
    int last_id = -1;
    for (int i = 0; i < MAX_ENTITIES; i++) {
        last_id = entity_create(ENT_NPC, i * 8, 0);
    }
    record_result(last_id >= 0, "entity_create: can fill all slots");
    // Next create should fail
    int overflow = entity_create(ENT_NPC, 0, 0);
    record_result(overflow == -1, "entity_create: returns -1 when full");
}

static void test_map_load(void) {
    map_load(&map_dfwtf_hq);
    record_result(current_map == &map_dfwtf_hq, "map_load: current_map set");
    record_result(camera_x == 0 && camera_y == 0, "map_load: camera reset to 0,0");
}

static void test_map_collision(void) {
    map_load(&map_dfwtf_hq);
    // Tile (0,0) in HQ map is wall (collision=1)
    u8 col_wall = map_get_collision(4, 4);  // Pixel (4,4) = tile (0,0)
    record_result(col_wall == 1, "map_collision: wall tile is solid");

    // Tile (1,1) in HQ map is floor (collision=0)
    u8 col_floor = map_get_collision(12, 12);  // Pixel (12,12) = tile (1,1)
    record_result(col_floor == 0, "map_collision: floor tile is walkable");

    // Out of bounds should be solid
    u8 col_oob = map_get_collision(-8, -8);
    record_result(col_oob == 1, "map_collision: out of bounds is solid");
}

static void test_map_transition_trigger(void) {
    map_load(&map_dfwtf_hq);
    // Tile (27,14) has collision value 10 (map transition)
    u8 col = map_get_collision(27 * 8 + 4, 14 * 8 + 4);
    record_result(col == 10, "map_collision: transition trigger value 10");
    // Transition tiles must be walkable so players can reach them
    record_result(map_is_walkable(27 * 8 + 4, 14 * 8 + 4),
                  "map_collision: transition tile is walkable");
}

static void test_passage_widths(void) {
    map_load(&map_dfwtf_hq);
    // HQ room A aisle (cols 3-4, row 2) must both be walkable
    record_result(map_is_walkable(3*8+4, 2*8+4), "passage: HQ room A col 3 walkable");
    record_result(map_is_walkable(4*8+4, 2*8+4), "passage: HQ room A col 4 walkable");
    // HQ exit must be 3 tiles wide (cols 26-28, row 14)
    record_result(map_is_walkable(26*8+4, 14*8+4), "passage: HQ exit col 26 walkable");
    record_result(map_is_walkable(27*8+4, 14*8+4), "passage: HQ exit col 27 walkable");
    record_result(map_is_walkable(28*8+4, 14*8+4), "passage: HQ exit col 28 walkable");

    map_load(&map_taco_bongo);
    // Taco table aisles (cols 3-4) must be walkable
    record_result(map_is_walkable(3*8+4, 2*8+4), "passage: taco aisle col 3 walkable");
    record_result(map_is_walkable(4*8+4, 2*8+4), "passage: taco aisle col 4 walkable");
    // Taco collision must work for rows > 0 (stride bug regression)
    // Row 4 col 1 is floor (walkable), row 4 col 2 is table (solid)
    record_result(map_is_walkable(1*8+4, 4*8+4), "passage: taco row4 col1 walkable");
    record_result(!map_is_walkable(2*8+4, 2*8+4), "passage: taco row2 col2 solid");
}

static void test_camera_clamp(void) {
    map_load(&map_dfwtf_hq);
    // Camera should clamp to map bounds
    map_update_camera(-100, -100);
    record_result(camera_x == 0 && camera_y == 0, "camera: clamps to 0,0 minimum");
}

static void test_entity_move_walkable(void) {
    entity_init();
    map_load(&map_dfwtf_hq);
    // Place entity on walkable tile (1,1) = pixel (8,8)
    // But collision checks inset hitbox, so place at a safe floor area
    // Row 7-8 in HQ is the hallway: y=56..71, collision=0 for x=8..224
    int id = entity_create(ENT_PLAYER_TREVOR, 40, 56);
    record_result(id >= 0, "entity_move: created on walkable tile");
    int old_x = entities[id].x;
    int moved = entity_move(id, 2, 0);
    record_result(moved == 1, "entity_move: can move on walkable");
    record_result(entities[id].x == old_x + 2, "entity_move: position updated");
}

static void test_entity_move_blocked(void) {
    entity_init();
    map_load(&map_dfwtf_hq);
    // Place entity right next to a wall
    // Tile (0,*) is all walls. Place entity at x=0, it should be blocked going left
    int id = entity_create(ENT_PLAYER_TREVOR, 10, 56);
    int moved = entity_move(id, -20, 0);  // Try to move into wall at x<8
    record_result(moved == 0, "entity_move: blocked by wall");
}

static void test_player_stats(void) {
    // Verify stat struct assignment works
    Stats s = {
        .shoot = 3, .brawn = 1, .brains = 2,
        .talk = 4, .cool = 2,
        .hp = 8, .hp_max = 8,
        .fate_points = 3, .fate_max = 5
    };
    record_result(s.shoot == 3 && s.talk == 4, "stats: field assignment");
    record_result(s.hp == s.hp_max, "stats: hp equals hp_max");
    record_result(s.fate_points <= s.fate_max, "stats: fp within max");
}

static void test_fate_roll_range(void) {
    // Fate roll: 4dF (-4 to +4) + stat - dc
    // With stat=0 and dc=0, result should be in [-4, +4]
    int min_seen = 99, max_seen = -99;
    for (int i = 0; i < 200; i++) {
        int r = fate_roll(0, 0);
        if (r < min_seen) min_seen = r;
        if (r > max_seen) max_seen = r;
    }
    record_result(min_seen >= -4 && max_seen <= 4, "fate_roll: range [-4,+4] with stat=0 dc=0");
    // Should see some variance
    record_result(max_seen > min_seen, "fate_roll: produces varying results");
}

static void test_fate_roll_stat_bonus(void) {
    // With high stat, results should shift positive
    int sum = 0;
    for (int i = 0; i < 100; i++) {
        sum += fate_roll(5, 0);
    }
    // Average should be around +5 (stat bonus)
    int avg_x10 = sum * 10 / 100;
    record_result(avg_x10 > 30, "fate_roll: stat bonus shifts results positive");
}

static void test_dialogue_tree_valid(void) {
    // Check that all dialogue trees are properly initialized
    record_result(dialogue_trees[0].nodes == 0, "dialogue: tree 0 is null");
    record_result(dialogue_trees[DLG_RUBIK_INTRO].nodes != 0, "dialogue: Rubik tree exists");
    record_result(dialogue_trees[DLG_RUBIK_INTRO].num_nodes == 4, "dialogue: Rubik has 4 nodes");
    record_result(dialogue_trees[DLG_DONNY_TACOS].nodes != 0, "dialogue: Donny tree exists");
    record_result(dialogue_trees[DLG_INFORMANT].nodes != 0, "dialogue: Informant tree exists");
}

static void test_dialogue_node_content(void) {
    const DialogueNode *n = &dialogue_trees[DLG_RUBIK_INTRO].nodes[0];
    record_result(n->speaker != 0, "dialogue: Rubik node 0 has speaker");
    record_result(n->text != 0, "dialogue: Rubik node 0 has text");
    record_result(n->num_choices == 3, "dialogue: Rubik node 0 has 3 choices");
    record_result(n->choices[0].text != 0, "dialogue: Rubik choice 0 has text");
}

static void test_dialogue_start_stop(void) {
    text_init();
    record_result(!dialogue_active(), "dialogue: initially inactive");
    dialogue_start(DLG_RUBIK_INTRO);
    record_result(dialogue_active(), "dialogue: active after start");
    record_result(dlg_state == DLG_PRINTING, "dialogue: state is PRINTING");

    // Clean up: reset dialogue state so it doesn't interfere with
    // later game boot tests (EXPLORE checks dialogue_active())
    dlg_state = DLG_INACTIVE;
}

static void test_map_data_integrity(void) {
    // Verify map dimensions make sense
    record_result(map_dfwtf_hq.width == 32, "map_data: HQ width=32");
    record_result(map_dfwtf_hq.height == 16, "map_data: HQ height=16");
    record_result(map_dfwtf_hq.collision != 0, "map_data: HQ has collision data");
    record_result(map_dfwtf_hq.fg.data != 0, "map_data: HQ has fg layer");

    record_result(map_dfw_streets.width == 32, "map_data: streets width=32");
    record_result(map_taco_bongo.width == 20, "map_data: taco width=20");
}

static void test_sprite_data_exists(void) {
    // Verify sprite arrays contain data (check a few words, not just [0]
    // since some sprites have transparent top-left corners)
    record_result(spr_trevor[0] != 0, "sprite_data: trevor not empty");
    record_result(spr_kip[0] != 0, "sprite_data: kip not empty");
    record_result(spr_npc_rubik[0] != 0, "sprite_data: rubik not empty");
    // Generic NPC has transparent top-left, check word 1 instead
    record_result(spr_npc_generic[1] != 0, "sprite_data: generic not empty");
}

static void test_tileset_data(void) {
    // Tile 0 should be empty (all zeros)
    record_result(tileset_urban[0] == 0, "tileset: tile 0 is empty");
    // Tile 1 (sidewalk) should not be empty
    record_result(tileset_urban[8] != 0, "tileset: tile 1 is not empty");
}

static void test_palette_data(void) {
    // Palette entry 0 = transparent (black) = RGB15(0,0,0) = 0
    record_result(spr_pal_characters[0] == 0, "palette: sprite pal[0] is transparent");
    // Entry 1 should be white = RGB15(31,31,31)
    record_result(spr_pal_characters[1] == RGB15(31, 31, 31), "palette: sprite pal[1] is white");
    // Tileset palette entry 0 also transparent
    record_result(tileset_urban_pal[0] == 0, "palette: tileset pal[0] is transparent");
}

static void test_game_state_transition(void) {
    game_init();
    record_result(game_state == STATE_TITLE, "game_state: starts at TITLE");

    // Simulate pressing START
    sim_key(KEY_START);
    game_update();
    record_result(game_state == STATE_MAP_TRANSITION, "game_state: START moves to MAP_TRANSITION");

    // Run transition frames
    sim_no_key();
    for (int i = 0; i < 20; i++) {
        game_update();
    }
    record_result(game_state == STATE_EXPLORE, "game_state: transitions to EXPLORE");
}

static void test_chapter1_setup(void) {
    // After transitioning to EXPLORE, entities should exist
    int trevor_found = 0, kip_found = 0, npc_found = 0;
    for (int i = 0; i < MAX_ENTITIES; i++) {
        if (!entities[i].active) continue;
        if (entities[i].type == ENT_PLAYER_TREVOR) trevor_found = 1;
        if (entities[i].type == ENT_PLAYER_KIP) kip_found = 1;
        if (entities[i].type == ENT_NPC) npc_found = 1;
    }
    record_result(trevor_found, "chapter1: Trevor entity exists");
    record_result(kip_found, "chapter1: Kip entity exists");
    record_result(npc_found, "chapter1: NPC entities exist");
    record_result(current_map == &map_dfwtf_hq, "chapter1: loaded HQ map");
}

static void test_player_movement_sim(void) {
    // Move player to known-walkable hallway position (row 7, col 5)
    // before testing movement
    Entity *p = &entities[active_player];
    p->x = 40;
    p->y = 56;
    int start_x = p->x;

    // Simulate holding RIGHT for a few frames
    for (int i = 0; i < 5; i++) {
        sim_key(KEY_RIGHT);
        game_update();
    }
    sim_no_key();

    record_result(p->x > start_x, "movement: player moved right");
    record_result(p->dir == DIR_RIGHT, "movement: player facing right");
}

static void test_character_switch(void) {
    // Ensure clean key state so SELECT registers as a key_hit
    sim_no_key();

    int initial = active_player;
    EntityType initial_type = entities[initial].type;

    sim_key(KEY_SELECT);
    game_update();
    sim_no_key();

    record_result(active_player != initial, "switch: active_player changed");
    EntityType new_type = entities[active_player].type;
    // Should be the other character
    int switched_correctly =
        (initial_type == ENT_PLAYER_TREVOR && new_type == ENT_PLAYER_KIP) ||
        (initial_type == ENT_PLAYER_KIP && new_type == ENT_PLAYER_TREVOR);
    record_result(switched_correctly, "switch: swapped to other character");
}

static void test_combat_system(void) {
    CombatEnemy enemies[1] = {{
        .name = "Test Thug",
        .stats = { .shoot = 1, .brawn = 1, .brains = 0, .talk = 0, .cool = 1,
                   .hp = 3, .hp_max = 3, .fate_points = 0, .fate_max = 0 },
        .sprite_tile = 0,
        .alive = 1
    }};
    combat_start(enemies, 1);
    record_result(combat_state == COMBAT_START, "combat: state is START after init");
    record_result(num_combat_enemies == 1, "combat: enemy count correct");
    record_result(combat_enemies[0].alive == 1, "combat: enemy is alive");
}

// ============================================================================
// REGRESSION TESTS
// ============================================================================

// Regression: players must spawn on walkable tiles
static void test_spawn_walkable(void) {
    // Boot the game to EXPLORE
    game_init();
    sim_key(KEY_START);
    game_update();
    sim_no_key();
    for (int i = 0; i < 20; i++) game_update();
    record_result(game_state == STATE_EXPLORE, "reg_spawn: reached EXPLORE");

    // Check that both players can actually move from their spawn
    for (int i = 0; i < MAX_ENTITIES; i++) {
        Entity *e = &entities[i];
        if (!e->active) continue;
        if (e->type == ENT_PLAYER_TREVOR) {
            int old_x = e->x;
            int moved = entity_move(i, 2, 0);
            record_result(moved == 1, "reg_spawn: Trevor can move from spawn");
            e->x = old_x;  // restore
        }
        if (e->type == ENT_PLAYER_KIP) {
            int old_x = e->x;
            int moved = entity_move(i, 2, 0);
            record_result(moved == 1, "reg_spawn: Kip can move from spawn");
            e->x = old_x;  // restore
        }
    }
}

// Regression: START menu must be escapable (no same-frame bounce)
static void test_start_menu_toggle(void) {
    // Ensure we're in EXPLORE first
    game_init();
    sim_key(KEY_START);
    game_update();
    sim_no_key();
    for (int i = 0; i < 20; i++) game_update();
    record_result(game_state == STATE_EXPLORE, "reg_menu: starts in EXPLORE");

    // Press START → should enter MENU
    sim_key(KEY_START);
    game_update();
    sim_no_key();
    record_result(game_state == STATE_MENU, "reg_menu: START enters MENU");

    // Release, then press START again → should return to EXPLORE
    game_update();  // one frame with no keys (clears key_hit)
    sim_key(KEY_START);
    game_update();
    sim_no_key();
    record_result(game_state == STATE_EXPLORE, "reg_menu: START exits MENU");

    // And one more toggle to be sure
    game_update();
    sim_key(KEY_START);
    game_update();
    sim_no_key();
    record_result(game_state == STATE_MENU, "reg_menu: START re-enters MENU");
}

// Regression: SELECT must swap characters in both directions
static void test_select_swap_roundtrip(void) {
    // Boot game
    game_init();
    sim_key(KEY_START);
    game_update();
    sim_no_key();
    for (int i = 0; i < 20; i++) game_update();

    int first = active_player;
    EntityType first_type = entities[first].type;

    // First swap: Trevor → Kip (or whatever order)
    sim_key(KEY_SELECT);
    game_update();
    sim_no_key();
    game_update();  // clear key state

    int second = active_player;
    record_result(second != first, "reg_swap: first SELECT changes player");
    record_result(entities[second].type != first_type, "reg_swap: swapped to different type");

    // Second swap: should return to original
    sim_key(KEY_SELECT);
    game_update();
    sim_no_key();

    record_result(active_player == first, "reg_swap: second SELECT returns to original");
    record_result(entities[active_player].type == first_type, "reg_swap: back to original type");

    // Third swap: verify it keeps working
    game_update();  // clear key state
    sim_key(KEY_SELECT);
    game_update();
    sim_no_key();

    record_result(active_player == second, "reg_swap: third SELECT works too");
}

// Regression: inactive entities must not hide active player sprites
static void test_sprite_visibility_after_draw(void) {
    // Boot game
    game_init();
    sim_key(KEY_START);
    game_update();
    sim_no_key();
    for (int i = 0; i < 20; i++) game_update();

    // Run a draw cycle
    game_draw();

    // Active player's sprite must not be hidden
    Entity *p = &entities[active_player];
    u16 attr0 = obj_buffer[p->sprite_id].attr0;
    int hidden = (attr0 & 0x0300) == ATTR0_HIDE;
    record_result(!hidden, "reg_vis: active player sprite not hidden after draw");

    // Both player sprites must be visible
    for (int i = 0; i < MAX_ENTITIES; i++) {
        if (!entities[i].active) continue;
        if (entities[i].type == ENT_PLAYER_TREVOR) {
            u16 a0 = obj_buffer[entities[i].sprite_id].attr0;
            record_result((a0 & 0x0300) != ATTR0_HIDE,
                          "reg_vis: Trevor sprite visible after draw");
        }
        if (entities[i].type == ENT_PLAYER_KIP) {
            u16 a0 = obj_buffer[entities[i].sprite_id].attr0;
            record_result((a0 & 0x0300) != ATTR0_HIDE,
                          "reg_vis: Kip sprite visible after draw");
        }
    }
}

// Regression: sprites must visually move when arrow keys are held
static void test_sprite_position_updates(void) {
    // Boot game
    game_init();
    sim_key(KEY_START);
    game_update();
    sim_no_key();
    for (int i = 0; i < 20; i++) game_update();

    Entity *p = &entities[active_player];
    p->x = 40;
    p->y = 56;

    // Draw to get initial sprite screen position
    game_draw();
    int initial_sx = obj_buffer[p->sprite_id].attr1 & 0x1FF;

    // Move right for several frames
    for (int i = 0; i < 10; i++) {
        sim_key(KEY_RIGHT);
        game_update();
        game_draw();
    }
    sim_no_key();

    // Sprite screen position must have changed
    int final_sx = obj_buffer[p->sprite_id].attr1 & 0x1FF;
    record_result(final_sx != initial_sx,
                  "reg_vis: sprite screen X changes when moving");

    // And sprite must still be visible
    u16 attr0 = obj_buffer[p->sprite_id].attr0;
    record_result((attr0 & 0x0300) != ATTR0_HIDE,
                  "reg_vis: sprite still visible after moving");
}

// Regression: SELECT swap must keep both characters' sprites visible
static void test_select_swap_visibility(void) {
    game_init();
    sim_key(KEY_START);
    game_update();
    sim_no_key();
    for (int i = 0; i < 20; i++) game_update();

    // Draw initial state
    game_draw();

    // Find both player entity indices
    int trevor_id = -1, kip_id = -1;
    for (int i = 0; i < MAX_ENTITIES; i++) {
        if (entities[i].active && entities[i].type == ENT_PLAYER_TREVOR) trevor_id = i;
        if (entities[i].active && entities[i].type == ENT_PLAYER_KIP) kip_id = i;
    }

    // Both must be visible initially
    record_result((obj_buffer[entities[trevor_id].sprite_id].attr0 & 0x0300) != ATTR0_HIDE,
                  "reg_swap_vis: Trevor visible before swap");
    record_result((obj_buffer[entities[kip_id].sprite_id].attr0 & 0x0300) != ATTR0_HIDE,
                  "reg_swap_vis: Kip visible before swap");

    // Swap to other character
    sim_key(KEY_SELECT);
    game_update();
    game_draw();
    sim_no_key();

    record_result((obj_buffer[entities[trevor_id].sprite_id].attr0 & 0x0300) != ATTR0_HIDE,
                  "reg_swap_vis: Trevor visible after swap");
    record_result((obj_buffer[entities[kip_id].sprite_id].attr0 & 0x0300) != ATTR0_HIDE,
                  "reg_swap_vis: Kip visible after swap");

    // Swap back
    game_update();
    sim_key(KEY_SELECT);
    game_update();
    game_draw();
    sim_no_key();

    record_result((obj_buffer[entities[trevor_id].sprite_id].attr0 & 0x0300) != ATTR0_HIDE,
                  "reg_swap_vis: Trevor visible after swap back");
    record_result((obj_buffer[entities[kip_id].sprite_id].attr0 & 0x0300) != ATTR0_HIDE,
                  "reg_swap_vis: Kip visible after swap back");
}

// ============================================================================
// TEST RUNNER
// ============================================================================

int main(void) {
    // Enable test mode: input_poll() will not read hardware
    input_test_mode = 1;

    // Initialize SRAM with empty results
    sram_write_byte(0, 'T');
    sram_write_byte(1, 'E');
    sram_write_byte(2, 'S');
    sram_write_byte(3, 'T');
    sram_write_byte(4, 0);     // total
    sram_write_byte(5, 0);     // pass
    sram_write_byte(6, 0);     // fail
    sram_write_byte(7, 0x00);  // Not done yet
    // Zero per-test result area
    for (int i = 0; i < MAX_TESTS; i++) {
        sram_write_byte(8 + i, 0);
    }

    // Open mGBA debug console
    mgba_open();
    mgba_log(MGBA_LOG_INFO, "=== AS&S Game Test Suite ===");

    // --- Run all tests ---

    // Data integrity tests
    test_tileset_data();
    test_palette_data();
    test_sprite_data_exists();
    test_map_data_integrity();
    test_dialogue_tree_valid();
    test_dialogue_node_content();

    // Core system tests
    test_player_stats();
    test_fate_roll_range();
    test_fate_roll_stat_bonus();

    // Entity system tests
    test_entity_init();
    test_entity_create();
    test_entity_create_fills_slots();

    // Map & collision tests
    test_map_load();
    test_map_collision();
    test_map_transition_trigger();
    test_passage_widths();
    test_camera_clamp();
    test_entity_move_walkable();
    test_entity_move_blocked();

    // Dialogue tests
    test_dialogue_start_stop();

    // Full game boot tests (these must run in order)
    test_game_init();
    test_game_state_transition();
    test_chapter1_setup();
    test_player_movement_sim();
    test_character_switch();

    // Combat tests
    test_combat_system();

    // Regression tests
    test_spawn_walkable();
    test_start_menu_toggle();
    test_select_swap_roundtrip();
    test_sprite_visibility_after_draw();
    test_sprite_position_updates();
    test_select_swap_visibility();

    // --- Report results ---
    mgba_log(MGBA_LOG_INFO, "=== Test Results ===");
    mgba_log_num(MGBA_LOG_INFO, "Total: ", test_count);
    mgba_log_num(MGBA_LOG_INFO, "Pass:  ", pass_count);
    mgba_log_num(MGBA_LOG_INFO, "Fail:  ", fail_count);

    if (fail_count == 0) {
        mgba_log(MGBA_LOG_INFO, "ALL TESTS PASSED");
    } else {
        mgba_log(MGBA_LOG_ERROR, "SOME TESTS FAILED");
    }

    // Write final results to SRAM
    sram_write_byte(4, (u8)test_count);
    sram_write_byte(5, (u8)pass_count);
    sram_write_byte(6, (u8)fail_count);
    sram_write_byte(7, 0xAA);  // Done marker

    // Halt — tests complete
    while (1) {
        vsync();
    }
}
