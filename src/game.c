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
static int current_map_id = -1;

static int transition_timer = 0;
static int title_blink = 0;

static const Map* map_by_id(int id) {
    switch (id) {
    case MAP_ID_HQ:      return &map_dfwtf_hq;
    case MAP_ID_STREETS: return &map_dfw_streets;
    case MAP_ID_TACO:    return &map_taco_bongo;
    default:             return &map_dfwtf_hq;
    }
}

static void spawn_entities(int map_id, int include_players) {
    if (map_id < 0 || map_id >= MAP_NUM_MAPS) return;
    const MapSpawnTable *st = &map_spawn_tables[map_id];
    for (int i = 0; i < st->num_spawns; i++) {
        const EntitySpawn *s = &st->spawns[i];
        int is_player = (s->type == ENT_PLAYER_TREVOR || s->type == ENT_PLAYER_KIP);
        if (is_player && !include_players) continue;
        int id = entity_create((EntityType)s->type, s->x, s->y);
        if (id < 0) continue;
        entities[id].dir = (Direction)s->dir;
        entities[id].base_tile = s->base_tile;
        entities[id].walk_speed = s->walk_speed;
        entities[id].dialogue_id = s->dialogue_id;
        entities[id].npc_behavior = s->npc_behavior;
        entities[id].stats = (Stats){
            .shoot = s->shoot, .brawn = s->brawn, .brains = s->brains,
            .talk = s->talk, .cool = s->cool,
            .hp = s->hp, .hp_max = s->hp_max,
            .fate_points = s->fate_points, .fate_max = s->fate_max
        };
        if (s->type == ENT_PLAYER_TREVOR) active_player = id;
    }
}

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

    // Load starting map and spawn all entities (including players)
    current_map_id = MAP_ID_HQ;
    map_load(map_by_id(current_map_id));
    spawn_entities(current_map_id, 1);
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

    switch ((int)game_state) {
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
            if (col >= 10 && current_map_id >= 0 && current_map_id < MAP_NUM_MAPS) {
                const MapTransitionTable *tt = &map_transition_tables[current_map_id];
                for (int i = 0; i < tt->num_transitions; i++) {
                    if (tt->transitions[i].collision_code == col) {
                        game_change_map(tt->transitions[i].target_map_id,
                                        tt->transitions[i].spawn_x,
                                        tt->transitions[i].spawn_y);
                        break;
                    }
                }
            }
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
    switch ((int)game_state) {
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
    current_map_id = map_id;
    map_load(map_by_id(map_id));

    // Move players to spawn point
    for (int i = 0; i < MAX_ENTITIES; i++) {
        if (entities[i].active &&
            (entities[i].type == ENT_PLAYER_TREVOR ||
             entities[i].type == ENT_PLAYER_KIP)) {
            entities[i].x = spawn_x + (entities[i].type == ENT_PLAYER_KIP ? 16 : 0);
            entities[i].y = spawn_y;
        }
    }

    // Spawn NPCs for this map (no players — they already exist)
    spawn_entities(map_id, 0);
}
