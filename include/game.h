// game.h — Main game state machine
#ifndef GAME_H
#define GAME_H

#include "gba.h"

// Game states
typedef enum {
    STATE_TITLE,
    STATE_EXPLORE,
    STATE_DIALOGUE,
    STATE_COMBAT,
    STATE_MENU,
    STATE_MAP_TRANSITION,
} GameState;

extern GameState game_state;

// Initialize the game
void game_init(void);

// Main game update (call each frame)
void game_update(void);

// Main game draw (call each frame)
void game_draw(void);

// Transition to a new map
void game_change_map(int map_id, int spawn_x, int spawn_y);

// Map IDs
#define MAP_ID_HQ      0
#define MAP_ID_STREETS  1
#define MAP_ID_TACO     2

#endif // GAME_H
