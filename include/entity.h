// entity.h — Game entity system (player, NPCs)
#ifndef ENTITY_H
#define ENTITY_H

#include "gba.h"

// Direction facing
typedef enum {
    DIR_DOWN  = 0,
    DIR_UP    = 1,
    DIR_LEFT  = 2,
    DIR_RIGHT = 3
} Direction;

// Entity type
typedef enum {
    ENT_NONE = 0,
    ENT_PLAYER_TREVOR,
    ENT_PLAYER_KIP,
    ENT_NPC,
    ENT_ITEM,
    ENT_TRIGGER,    // Invisible trigger zone
} EntityType;

// Character stats (Fate-inspired)
typedef struct {
    s8 shoot;   // Ranged combat
    s8 brawn;   // Melee / physical
    s8 brains;  // Investigation
    s8 talk;    // Social
    s8 cool;    // Composure / dodge
    s8 hp;
    s8 hp_max;
    s8 fate_points;
    s8 fate_max;
} Stats;

#define MAX_ENTITIES 32

typedef struct {
    EntityType type;
    s32 x, y;          // Position in pixels (world coords)
    Direction dir;
    u16 sprite_id;     // OAM slot
    u16 base_tile;     // First tile of animation set
    u8  anim_frame;
    u8  anim_timer;
    u8  walk_speed;    // Pixels per frame
    u8  active;        // Is this entity in use?

    // NPC data
    u16 dialogue_id;   // Index into dialogue table
    u8  npc_behavior;  // 0=stationary, 1=wander, 2=patrol

    // Stats (only for player characters)
    Stats stats;
} Entity;

extern Entity entities[MAX_ENTITIES];
extern int active_player;  // Index of currently controlled player entity

// Initialize entity system
void entity_init(void);

// Create an entity, returns index or -1
int entity_create(EntityType type, int x, int y);

// Update all entities (movement, animation)
void entity_update_all(void);

// Draw all entities (update sprite positions relative to camera)
void entity_draw_all(void);

// Move entity with collision checking; returns 1 if moved
int entity_move(int id, int dx, int dy);

// Get entity at world position (for interaction)
int entity_at(int px, int py, int range);

// Player-specific update (reads input)
void player_update(void);

#endif // ENTITY_H
