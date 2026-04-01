// combat.h — Turn-based combat system
#ifndef COMBAT_H
#define COMBAT_H

#include "gba.h"
#include "entity.h"

// Combat states
typedef enum {
    COMBAT_INACTIVE = 0,
    COMBAT_START,       // Transition into combat
    COMBAT_PLAYER_TURN, // Player choosing action
    COMBAT_ENEMY_TURN,  // Enemy acting
    COMBAT_ANIMATE,     // Playing attack animation
    COMBAT_RESULT,      // Showing result text
    COMBAT_VICTORY,
    COMBAT_DEFEAT,
} CombatState;

// An enemy in combat
typedef struct {
    const char *name;
    Stats stats;
    u16 sprite_tile;
    u8  alive;
} CombatEnemy;

// Combat action types
typedef enum {
    ACT_SHOOT,    // Use SHOOT stat — ranged attack
    ACT_PUNCH,    // Use BRAWN stat — melee
    ACT_TALK,     // Use TALK stat — try to de-escalate / intimidate
    ACT_ITEM,     // Use an item
    ACT_FATE,     // Spend a Fate Point for a bonus
} CombatAction;

#define MAX_COMBAT_ENEMIES 4

extern CombatState combat_state;
extern CombatEnemy combat_enemies[MAX_COMBAT_ENEMIES];
extern int num_combat_enemies;

// Start a combat encounter
void combat_start(const CombatEnemy *enemies, int count);

// Update combat (call each frame)
void combat_update(void);

// Draw combat UI (call each frame)
void combat_draw(void);

// Is combat active?
static inline int combat_active(void) {
    return combat_state != COMBAT_INACTIVE;
}

// Fate-style "roll": stat + random(1-4) vs DC
// Returns margin (positive = success, negative = fail)
int fate_roll(int stat, int dc);

#endif // COMBAT_H
