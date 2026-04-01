#ifndef PORTRAIT_DATA_H
#define PORTRAIT_DATA_H
#include "gba.h"

#define PORTRAIT_TILE_BASE 160
#define PORTRAIT_OAM_SLOT  33
#define PORTRAIT_TILES     16  // 32x32 = 4x4 tiles of 8x8
#define PORTRAIT_BYTES     (PORTRAIT_TILES * 32)

typedef struct {
    const char *speaker;
    const u32 *data;
} PortraitEntry;

extern const PortraitEntry portrait_table[];

// Find portrait data for a speaker name. Returns NULL if not found.
const u32* portrait_find(const char *speaker);

#endif
