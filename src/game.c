// game.c — Main game state machine and initialization
#include "game.h"
#include "input.h"
#include "sprite.h"
#include "map.h"
#include "entity.h"
#include "dialogue.h"
#include "combat.h"
#include "game_data.h"
#include "debug_input.h"

GameState game_state = STATE_TITLE;

static int transition_timer = 0;
static int title_blink = 0;

// --- Title screen ---
static void title_draw(void) {
    // Use BG2 for title text
    REG_DISPCNT = DCNT_MODE0 | DCNT_BG2;
    REG_BG2CNT = BG_PRIO(0) | BG_CBB(2) | BG_SBB(24) | BG_4BPP | BG_SIZE_256x256;
    REG_BG2HOFS = 0;
    REG_BG2VOFS = 0;

    text_clear_region(24, 0, 0, 30, 20);

    // Title text
    text_draw_string("ADVANCED", 24, 11, 3, 10);
    text_draw_string("SAGEBRUSH", 24, 10, 5, 12);
    text_draw_string("&", 24, 15, 7, 1);
    text_draw_string("SHOOTOUTS", 24, 10, 9, 12);

    // Subtitle
    text_draw_string("A DFWTF Adventure", 24, 7, 12, 20);

    // Blinking "Press Start"
    title_blink++;
    if ((title_blink / 30) % 2 == 0) {
        text_draw_string("PRESS START", 24, 10, 16, 12);
    }
}

// --- Initialize the game world ---
static void setup_chapter_1(void) {
    // Load tileset
    map_load_tileset(tileset_urban, TILESET_URBAN_SIZE, 0);
    map_load_palette(tileset_urban_pal, 16);

    // Load character sprites
    sprite_load_tiles(spr_trevor, SPR_TREVOR_BASE, SPR_BYTES_PER_CHAR);
    sprite_load_tiles(spr_kip, SPR_KIP_BASE, SPR_BYTES_PER_CHAR);
    sprite_load_tiles(spr_npc_rubik, SPR_RUBIK_BASE, SPR_BYTES_PER_CHAR);
    sprite_load_tiles(spr_npc_generic, SPR_GENERIC_BASE, SPR_BYTES_PER_CHAR);
    sprite_load_palette(spr_pal_characters, 0, 16);

    // Load interaction indicator sprite
    sprite_load_tiles(spr_a_indicator, SPR_INDICATOR_TILE, 32);
    sprite_set_size(INDICATOR_OAM_SLOT, ATTR0_SQUARE, ATTR1_SIZE_8);
    sprite_set_tile(INDICATOR_OAM_SLOT, SPR_INDICATOR_TILE, 0);
    sprite_hide(INDICATOR_OAM_SLOT);

    // Load starting map (DFWTF HQ)
    map_load(&map_dfwtf_hq);

    // Create player entities (spawn in the hallway — rows 7-8 are walkable)
    int trevor = entity_create(ENT_PLAYER_TREVOR, 40, 56);
    if (trevor >= 0) {
        entities[trevor].base_tile = SPR_TREVOR_BASE;
        entities[trevor].walk_speed = 2;
        entities[trevor].stats = (Stats){
            .shoot = 3, .brawn = 1, .brains = 1,
            .talk = 3, .cool = 2,
            .hp = 8, .hp_max = 8,
            .fate_points = 3, .fate_max = 5
        };
        active_player = trevor;
    }

    int kip = entity_create(ENT_PLAYER_KIP, 56, 56);
    if (kip >= 0) {
        entities[kip].base_tile = SPR_KIP_BASE;
        entities[kip].walk_speed = 2;
        entities[kip].stats = (Stats){
            .shoot = 1, .brawn = 3, .brains = 2,
            .talk = 1, .cool = 3,
            .hp = 10, .hp_max = 10,
            .fate_points = 3, .fate_max = 5
        };
    }

    // Create NPCs (all positions verified against collision map)
    // Rubik: room at row 4, col 9-11 (walkable)
    int rubik = entity_create(ENT_NPC, 72, 32);
    if (rubik >= 0) {
        entities[rubik].base_tile = SPR_RUBIK_BASE;
        entities[rubik].dialogue_id = DLG_RUBIK_INTRO;
        entities[rubik].dir = DIR_DOWN;
    }

    // Peacemaker: hallway row 7, col 18 (walkable)
    int peacemaker = entity_create(ENT_NPC, 144, 56);
    if (peacemaker >= 0) {
        entities[peacemaker].base_tile = SPR_GENERIC_BASE;
        entities[peacemaker].dialogue_id = DLG_PEACEMAKER_HINT;
        entities[peacemaker].dir = DIR_LEFT;
    }

    // Ramis: room at row 12, col 1-3 (walkable)
    int ramis = entity_create(ENT_NPC, 16, 96);
    if (ramis >= 0) {
        entities[ramis].base_tile = SPR_GENERIC_BASE;
        entities[ramis].dialogue_id = DLG_RAMIS_CAR;
        entities[ramis].dir = DIR_RIGHT;
    }

    // Random cop: hallway row 8, col 22 (walkable)
    int cop1 = entity_create(ENT_NPC, 176, 64);
    if (cop1 >= 0) {
        entities[cop1].base_tile = SPR_GENERIC_BASE;
        entities[cop1].dialogue_id = DLG_RANDOM_COP_1;
        entities[cop1].npc_behavior = 1; // Wander
    }
}

void game_init(void) {
    // Initialize subsystems
    sprite_init();
    entity_init();
    text_init();

    // Reset all state machines
    game_state = STATE_TITLE;
    combat_state = COMBAT_INACTIVE;
    dlg_state = DLG_INACTIVE;

    // Enable timer for RNG
    REG_TM0CNT_H = TM_ENABLE | TM_FREQ_1024;

    // Debug: on-screen key display
    debug_input_init();
}

void game_update(void) {
    input_poll();
    debug_input_update();

    GameState prev_state = game_state;

    switch (game_state) {
    case STATE_TITLE:
        if (key_hit(KEY_START)) {
            game_state = STATE_MAP_TRANSITION;
            transition_timer = 0;
        }
        break;

    case STATE_MAP_TRANSITION:
        transition_timer++;
        if (transition_timer == 1) {
            // Initialize the game world on first transition
            setup_chapter_1();
        }
        if (transition_timer > 15) {
            game_state = STATE_EXPLORE;
            // Set up display for exploration
            REG_DISPCNT = DCNT_MODE0 | DCNT_BG0 | DCNT_BG1 | DCNT_OBJ | DCNT_OBJ_1D;
        }
        break;

    case STATE_EXPLORE:
        if (dialogue_active()) {
            dialogue_update();
        } else if (combat_active()) {
            combat_update();
        } else {
            player_update();
            entity_update_all();

            // Update camera to follow active player
            Entity *p = &entities[active_player];
            map_update_camera(p->x + 8, p->y + 8);

            // Check for map transition triggers
            u8 col = map_get_collision(p->x + 8, p->y + 8);
            if (col >= 10) {
                // Collision values 10+ are map transition triggers
                switch (col) {
                case 10: game_change_map(MAP_ID_STREETS, 128, 16); break;
                case 11: game_change_map(MAP_ID_HQ, 128, 224); break;
                case 12: game_change_map(MAP_ID_TACO, 80, 128); break;
                }
            }

            // Check for random encounters on streets
            // (simplified: collision value 3 = encounter zone)
        }
        break;

    case STATE_MENU:
        // TODO: pause menu with stats, Fate points, aspects
        if (key_hit(KEY_START)) {
            game_state = STATE_EXPLORE;
            // Re-enable exploration display
            REG_DISPCNT = DCNT_MODE0 | DCNT_BG0 | DCNT_BG1 | DCNT_OBJ | DCNT_OBJ_1D;
        }
        break;

    default:
        break;
    }

    // START opens menu during exploration (if not in dialogue/combat)
    // Guard: only if we were already in EXPLORE before the switch above
    // (prevents same-frame MENU→EXPLORE→MENU bounce)
    if (game_state == STATE_EXPLORE && prev_state == STATE_EXPLORE &&
        !dialogue_active() && !combat_active()) {
        if (key_hit(KEY_START)) {
            game_state = STATE_MENU;
        }
    }
}

void game_draw(void) {
    switch (game_state) {
    case STATE_TITLE:
        title_draw();
        break;

    case STATE_EXPLORE:
        map_apply_scroll();
        entity_draw_all();
        indicator_update();
        sprite_update_oam();

        if (dialogue_active()) {
            dialogue_draw();
        }
        if (combat_active()) {
            combat_draw();
        }
        break;

    case STATE_MENU:
        // Keep rendering the game world behind the menu
        map_apply_scroll();
        entity_draw_all();
        indicator_update();
        // TODO: draw menu UI overlay here
        sprite_update_oam();
        break;

    case STATE_MAP_TRANSITION:
        // Could fade to black here
        break;

    default:
        break;
    }
}

void game_change_map(int map_id, int spawn_x, int spawn_y) {
    // Clear non-player entities
    for (int i = 0; i < MAX_ENTITIES; i++) {
        if (entities[i].active &&
            entities[i].type != ENT_PLAYER_TREVOR &&
            entities[i].type != ENT_PLAYER_KIP) {
            entities[i].active = 0;
            sprite_hide(i);
        }
    }

    // Load new map
    switch (map_id) {
    case MAP_ID_HQ:      map_load(&map_dfwtf_hq); break;
    case MAP_ID_STREETS:  map_load(&map_dfw_streets); break;
    case MAP_ID_TACO:     map_load(&map_taco_bongo); break;
    }

    // Move players to spawn point
    for (int i = 0; i < MAX_ENTITIES; i++) {
        if (entities[i].active &&
            (entities[i].type == ENT_PLAYER_TREVOR ||
             entities[i].type == ENT_PLAYER_KIP)) {
            entities[i].x = spawn_x + (entities[i].type == ENT_PLAYER_KIP ? 16 : 0);
            entities[i].y = spawn_y;
        }
    }

    // Set up NPCs for this map
    if (map_id == MAP_ID_TACO) {
        int donny = entity_create(ENT_NPC, 64, 48);
        if (donny >= 0) {
            entities[donny].base_tile = SPR_GENERIC_BASE;
            entities[donny].dialogue_id = DLG_DONNY_TACOS;
            entities[donny].dir = DIR_DOWN;
        }
    }
    if (map_id == MAP_ID_STREETS) {
        int informant = entity_create(ENT_NPC, 176, 128);
        if (informant >= 0) {
            entities[informant].base_tile = SPR_GENERIC_BASE;
            entities[informant].dialogue_id = DLG_INFORMANT;
            entities[informant].dir = DIR_LEFT;
        }
    }
}
