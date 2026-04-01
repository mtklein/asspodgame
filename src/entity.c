// entity.c — Entity system implementation
#include "entity.h"
#include "input.h"
#include "map.h"
#include "sprite.h"
#include "dialogue.h"

Entity entities[MAX_ENTITIES];
int active_player = 0;

void entity_init(void) {
    for (int i = 0; i < MAX_ENTITIES; i++) {
        entities[i].type = ENT_NONE;
        entities[i].active = 0;
    }
}

int entity_create(EntityType type, int x, int y) {
    for (int i = 0; i < MAX_ENTITIES; i++) {
        if (!entities[i].active) {
            Entity *e = &entities[i];
            e->type = type;
            e->x = x;
            e->y = y;
            e->dir = DIR_DOWN;
            e->sprite_id = (u16)i;  // Simple 1:1 mapping
            e->base_tile = 0;
            e->anim_frame = 0;
            e->anim_timer = 0;
            e->walk_speed = 1;
            e->active = 1;
            e->dialogue_id = 0;
            e->npc_behavior = 0;

            // Set default sprite size (16x16)
            sprite_show(i);
            sprite_set_size(i, ATTR0_SQUARE, ATTR1_SIZE_16);

            return i;
        }
    }
    return -1; // No free slot
}

// Check collision for a 16x16 entity at pixel position
static int check_collision(int px, int py) {
    // Check all four corners of the entity's bounding box
    // Use a slightly smaller hitbox (inset by 2px) for nicer feel
    int left   = px + 2;
    int right  = px + 13;
    int top    = py + 4;   // Top half is more lenient (head can overlap)
    int bottom = py + 15;

    return !map_is_walkable(left,  top)    ||
           !map_is_walkable(right, top)    ||
           !map_is_walkable(left,  bottom) ||
           !map_is_walkable(right, bottom);
}

int entity_move(int id, int dx, int dy) {
    Entity *e = &entities[id];
    if (!e->active) return 0;

    int new_x = e->x + dx;
    int new_y = e->y + dy;

    // Try both axes
    if (!check_collision(new_x, new_y)) {
        e->x = new_x;
        e->y = new_y;
        return 1;
    }
    // Try just horizontal
    if (dx != 0 && !check_collision(new_x, e->y)) {
        e->x = new_x;
        return 1;
    }
    // Try just vertical
    if (dy != 0 && !check_collision(e->x, new_y)) {
        e->y = new_y;
        return 1;
    }

    return 0; // Blocked
}

static void entity_animate(Entity *e) {
    e->anim_timer++;
    if (e->anim_timer >= 8) {
        e->anim_timer = 0;
        e->anim_frame = (e->anim_frame + 1) % 4;
    }
    // 4 directions × 4 frames = 16 frames per character
    // Each 16x16 sprite uses 4 tiles (2×2 in 4bpp)
    int frame_tiles = e->base_tile + (e->dir * 4 + e->anim_frame) * 4;
    sprite_set_tile(e->sprite_id, frame_tiles, 0);
}

void player_update(void) {
    Entity *player = &entities[active_player];
    if (!player->active) return;

    int dx = key_tri_horz();
    int dy = key_tri_vert();

    // Update facing direction
    if (dx > 0)      player->dir = DIR_RIGHT;
    else if (dx < 0) player->dir = DIR_LEFT;
    else if (dy > 0) player->dir = DIR_DOWN;
    else if (dy < 0) player->dir = DIR_UP;

    int moved = 0;
    if (dx != 0 || dy != 0) {
        moved = entity_move(active_player, dx * player->walk_speed, dy * player->walk_speed);
    }

    // Animate if moving, reset to standing frame if not
    if (moved) {
        entity_animate(player);
    } else {
        player->anim_frame = 0;
        player->anim_timer = 0;
        int frame_tiles = player->base_tile + player->dir * 4 * 4;
        sprite_set_tile(player->sprite_id, frame_tiles, 0);
    }

    // Interaction: press A to talk/interact
    if (key_hit(KEY_A)) {
        // Check for nearby NPC in facing direction
        int check_x = player->x + 8; // Center
        int check_y = player->y + 8;
        switch (player->dir) {
            case DIR_UP:    check_y -= 16; break;
            case DIR_DOWN:  check_y += 16; break;
            case DIR_LEFT:  check_x -= 16; break;
            case DIR_RIGHT: check_x += 16; break;
        }
        int npc = entity_at(check_x, check_y, 12);
        if (npc >= 0 && entities[npc].dialogue_id > 0) {
            dialogue_start(entities[npc].dialogue_id);
        }
    }

    // Switch active character with SELECT
    if (key_hit(KEY_SELECT)) {
        // Find the other player character
        for (int i = 0; i < MAX_ENTITIES; i++) {
            if (i != active_player &&
                (entities[i].type == ENT_PLAYER_TREVOR ||
                 entities[i].type == ENT_PLAYER_KIP) &&
                entities[i].active) {
                active_player = i;
                break;
            }
        }
    }
}

void entity_update_all(void) {
    for (int i = 0; i < MAX_ENTITIES; i++) {
        Entity *e = &entities[i];
        if (!e->active) continue;

        if (e->type == ENT_NPC && e->npc_behavior == 1) {
            // Simple wander: occasionally change direction
            e->anim_timer++;
            if (e->anim_timer >= 60) { // ~1 second
                e->anim_timer = 0;
                // Pseudo-random direction change
                e->dir = (Direction)((e->dir + 1) % 4);
            }
        }
    }
}

void entity_draw_all(void) {
    for (int i = 0; i < MAX_ENTITIES; i++) {
        Entity *e = &entities[i];
        if (!e->active || e->type == ENT_TRIGGER) {
            sprite_hide(e->sprite_id);
            continue;
        }

        // Convert world coords to screen coords
        int sx = e->x - (int)camera_x;
        int sy = e->y - (int)camera_y;

        // Hide if off screen
        if (sx < -16 || sx > 240 || sy < -16 || sy > 160) {
            sprite_hide(e->sprite_id);
        } else {
            sprite_show(e->sprite_id);
            sprite_set_pos(e->sprite_id, sx, sy);
        }
    }
}

int entity_at(int px, int py, int range) {
    for (int i = 0; i < MAX_ENTITIES; i++) {
        if (!entities[i].active) continue;
        if (entities[i].type == ENT_TRIGGER) continue;

        int dx = px - (entities[i].x + 8);
        int dy = py - (entities[i].y + 8);
        if (dx < 0) dx = -dx;
        if (dy < 0) dy = -dy;

        if (dx < range && dy < range) {
            return i;
        }
    }
    return -1;
}
