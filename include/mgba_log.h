// mgba_log.h — mGBA debug console output
// Writes appear in mGBA's log/debug console when run in the emulator.
// On real hardware these writes are harmless no-ops.
#ifndef MGBA_LOG_H
#define MGBA_LOG_H

#include "gba.h"

#define REG_DEBUG_ENABLE (*(vu16*)0x04FFF780)
#define REG_DEBUG_FLAGS  (*(vu16*)0x04FFF700)
#define REG_DEBUG_STRING ((char*)0x04FFF600)

#define MGBA_LOG_FATAL 0
#define MGBA_LOG_ERROR 1
#define MGBA_LOG_WARN  2
#define MGBA_LOG_INFO  3
#define MGBA_LOG_DEBUG 4

// Call once at startup. Returns 1 if running in mGBA, 0 otherwise.
static inline int mgba_open(void) {
    REG_DEBUG_ENABLE = 0xC0DE;
    return REG_DEBUG_ENABLE == 0x1DEA;
}

// Log a message (max 256 chars) at the given level.
static inline void mgba_log(int level, const char *msg) {
    // Copy message into debug string buffer
    volatile char *dst = (volatile char*)0x04FFF600;
    int i = 0;
    while (msg[i] && i < 255) {
        dst[i] = msg[i];
        i++;
    }
    dst[i] = '\0';
    REG_DEBUG_FLAGS = level | 0x100; // 0x100 = send flag
}

// Simple number-to-string for test output (no sprintf on bare metal)
static inline void mgba_log_num(int level, const char *prefix, int value) {
    char buf[80];
    int i = 0;
    // Copy prefix
    while (prefix[i] && i < 60) { buf[i] = prefix[i]; i++; }

    // Convert number to decimal
    if (value < 0) { buf[i++] = '-'; value = -value; }
    if (value == 0) { buf[i++] = '0'; }
    else {
        char tmp[12];
        int j = 0;
        while (value > 0 && j < 11) {
            tmp[j++] = '0' + (value % 10);
            value /= 10;
        }
        while (j > 0) buf[i++] = tmp[--j];
    }
    buf[i] = '\0';
    mgba_log(level, buf);
}

#endif // MGBA_LOG_H
