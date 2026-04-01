// input.h — Input handling with edge detection
#ifndef INPUT_H
#define INPUT_H

#include "gba.h"

// Current and previous key states
extern u16 key_cur;
extern u16 key_prev;

// When nonzero, input_poll() skips the hardware read (for testing)
extern u8 input_test_mode;

// Call once per frame at start of update
void input_poll(void);

// Key held this frame
static inline int key_held(u16 key)    { return  (key_cur & key) != 0; }
// Key just pressed this frame (edge)
static inline int key_hit(u16 key)     { return  (key_cur & ~key_prev & key) != 0; }
// Key just released this frame
static inline int key_released(u16 key){ return (~key_cur &  key_prev & key) != 0; }

// Directional helpers
static inline int key_tri_horz(void) {
    return key_held(KEY_RIGHT) - key_held(KEY_LEFT);
}
static inline int key_tri_vert(void) {
    return key_held(KEY_DOWN) - key_held(KEY_UP);
}

#endif // INPUT_H
