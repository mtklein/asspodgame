// screenshot.c — headless GBA screenshot capture using libmgba
//
// Loads a ROM, optionally simulates key presses, runs N frames,
// writes the framebuffer as a raw PPM to stdout.
//
// Usage: screenshot <rom.gba> [frames] [keys_at_frame ...]
//   screenshot ass_game.gba 120 0:start 30:none
//     -> press START at frame 0, release at frame 30, capture at frame 120
//
// Key names: a b select start right left up down r l none

#include <mgba/core/core.h>
#include <mgba/core/log.h>
#include <mgba-util/vfs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define GBA_KEY_A       0
#define GBA_KEY_B       1
#define GBA_KEY_SELECT  2
#define GBA_KEY_START   3
#define GBA_KEY_RIGHT   4
#define GBA_KEY_LEFT    5
#define GBA_KEY_UP      6
#define GBA_KEY_DOWN    7
#define GBA_KEY_R       8
#define GBA_KEY_L       9

static void no_log(struct mLogger *log, int cat, enum mLogLevel level,
                   const char *fmt, va_list args) {
    (void)log; (void)cat; (void)level; (void)fmt; (void)args;
}

struct KeyEvent {
    int frame;
    int key;  // -1 = release all
};

static int parse_key(const char *name) {
    if (strcmp(name, "a") == 0) return GBA_KEY_A;
    if (strcmp(name, "b") == 0) return GBA_KEY_B;
    if (strcmp(name, "select") == 0) return GBA_KEY_SELECT;
    if (strcmp(name, "start") == 0) return GBA_KEY_START;
    if (strcmp(name, "right") == 0) return GBA_KEY_RIGHT;
    if (strcmp(name, "left") == 0) return GBA_KEY_LEFT;
    if (strcmp(name, "up") == 0) return GBA_KEY_UP;
    if (strcmp(name, "down") == 0) return GBA_KEY_DOWN;
    if (strcmp(name, "r") == 0) return GBA_KEY_R;
    if (strcmp(name, "l") == 0) return GBA_KEY_L;
    if (strcmp(name, "none") == 0) return -1;
    return -1;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <rom.gba> [frames] [frame:key ...]\n", argv[0]);
        return 1;
    }
    const char *rom = argv[1];
    int total_frames = argc > 2 ? atoi(argv[2]) : 120;

    // Parse key events
    struct KeyEvent events[64];
    int num_events = 0;
    for (int i = 3; i < argc && num_events < 64; i++) {
        char *colon = strchr(argv[i], ':');
        if (colon) {
            *colon = '\0';
            events[num_events].frame = atoi(argv[i]);
            events[num_events].key = parse_key(colon + 1);
            num_events++;
        }
    }

    struct mLogger logger = { .log = no_log };
    mLogSetDefaultLogger(&logger);

    struct mCore *core = mCoreFind(rom);
    if (!core) { fprintf(stderr, "ERROR: Can't identify %s\n", rom); return 1; }
    core->init(core);

    unsigned w, h;
    core->desiredVideoDimensions(core, &w, &h);
    color_t *vidbuf = calloc(w * h, sizeof(color_t));
    core->setVideoBuffer(core, vidbuf, w);

    if (!mCoreLoadFile(core, rom)) {
        fprintf(stderr, "ERROR: Can't load %s\n", rom);
        core->deinit(core); free(vidbuf); return 1;
    }
    mCoreConfigInit(&core->config, "screenshot");
    core->reset(core);

    uint16_t keys = 0;
    for (int frame = 0; frame < total_frames; frame++) {
        // Apply key events for this frame
        for (int e = 0; e < num_events; e++) {
            if (events[e].frame == frame) {
                if (events[e].key < 0) {
                    keys = 0;
                } else {
                    keys |= (1 << events[e].key);
                }
            }
        }
        core->setKeys(core, keys);
        core->runFrame(core);
    }

    // Write PPM to stdout (simple, no dependencies)
    // color_t on GBA mGBA is typically 32-bit ABGR or ARGB
    printf("P6\n%u %u\n255\n", w, h);
    for (unsigned i = 0; i < w * h; i++) {
        color_t c = vidbuf[i];
        // mGBA color_t is 32-bit: 0xAARRGGBB or 0xAABBGGRR depending on platform
        // On macOS/little-endian it's typically 0xAABBGGRR in memory
        unsigned char r = (c >> 0) & 0xFF;
        unsigned char g = (c >> 8) & 0xFF;
        unsigned char b = (c >> 16) & 0xFF;
        fwrite(&r, 1, 1, stdout);
        fwrite(&g, 1, 1, stdout);
        fwrite(&b, 1, 1, stdout);
    }

    core->deinit(core);
    free(vidbuf);
    return 0;
}
