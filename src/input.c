// input.c — Input polling implementation
#include "input.h"

u16 key_cur  = 0;
u16 key_prev = 0;

void input_poll(void) {
    key_prev = key_cur;
    // GBA keys are active-low, so invert
    key_cur = ~REG_KEYINPUT & 0x03FF;
}
