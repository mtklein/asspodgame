// main.c — Entry point for Advanced Sagebrush & Shootouts: The Game
// A GBA top-down RPG based on the AS&S podcast
//
// "In the DFW, crime is a real tough nut, and the regular police
//  can't crack it... now it's up to the DFWTF to save everybody's bacon."

#include "gba.h"
#include "game.h"

int main(void) {
    // Initialize the game
    game_init();

    // Main loop
    while (1) {
        // Wait for vblank (sync to 60fps)
        vsync();

        // Update game logic
        game_update();

        // Render
        game_draw();
    }
}
