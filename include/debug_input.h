// debug_input.h — On-screen key press display for debugging
#ifndef DEBUG_INPUT_H
#define DEBUG_INPUT_H

// Initialize debug key display sprites (call after sprite_init)
void debug_input_init(void);

// Update debug key display (call each frame after input_poll)
void debug_input_update(void);

#endif // DEBUG_INPUT_H
