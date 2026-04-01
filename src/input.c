// input.c — Input polling implementation
#include "input.h"

u16 key_cur  = 0;
u16 key_prev = 0;
u8  input_test_mode = 0;

void input_poll(void) {
    if (input_test_mode) {
        // In test mode, key_cur/key_prev are set externally by sim_key()
        return;
    }
    key_prev = key_cur;
    // GBA keys are active-low, so invert
    key_cur = ~REG_KEYINPUT & 0x03FF;
}
