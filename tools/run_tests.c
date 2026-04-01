// run_tests.c — headless GBA test runner using libmgba
//
// Loads a test ROM, runs frames until the SRAM done marker appears,
// then reads results from SRAM and reports pass/fail.
//
// SRAM protocol (written by test_main.c):
//   Byte 0-3: magic "TEST"
//   Byte 4:   total test count
//   Byte 5:   pass count
//   Byte 6:   fail count
//   Byte 7:   done flag (0xAA = complete)
//   Byte 8+:  per-test results (0=pass, 1=fail)

#include <mgba/core/core.h>
#include <mgba/core/log.h>
#include <mgba-util/vfs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SRAM_BASE   0x0E000000
#define DONE_MARKER 0xAA
#define MAX_FRAMES  600  // 10 seconds at 60fps

static void no_log(struct mLogger *log, int cat, enum mLogLevel level,
                   const char *fmt, va_list args) {
    (void)log; (void)cat; (void)level; (void)fmt; (void)args;
}

int main(int argc, char *argv[]) {
    const char *rom = argc > 1 ? argv[1] : "ass_game_test.gba";

    struct mLogger logger = { .log = no_log };
    mLogSetDefaultLogger(&logger);

    struct mCore *core = mCoreFind(rom);
    if (!core) {
        fprintf(stderr, "ERROR: Could not identify platform for %s\n", rom);
        return 1;
    }
    core->init(core);

    unsigned w, h;
    core->desiredVideoDimensions(core, &w, &h);
    color_t *vidbuf = calloc(w * h, sizeof(color_t));
    core->setVideoBuffer(core, vidbuf, w);

    if (!mCoreLoadFile(core, rom)) {
        fprintf(stderr, "ERROR: Could not load ROM: %s\n", rom);
        core->deinit(core);
        free(vidbuf);
        return 1;
    }
    mCoreConfigInit(&core->config, "test");
    core->reset(core);

    // Run frames until done marker appears or timeout.
    int done = 0;
    for (int frame = 0; frame < MAX_FRAMES; frame++) {
        core->runFrame(core);
        if (core->busRead8(core, SRAM_BASE + 7) == DONE_MARKER) {
            done = 1;
            break;
        }
    }

    if (!done) {
        fprintf(stderr, "ERROR: Tests did not complete within %d frames\n", MAX_FRAMES);
        core->deinit(core);
        free(vidbuf);
        return 1;
    }

    // Verify magic.
    char magic[5] = {
        core->busRead8(core, SRAM_BASE + 0),
        core->busRead8(core, SRAM_BASE + 1),
        core->busRead8(core, SRAM_BASE + 2),
        core->busRead8(core, SRAM_BASE + 3),
        '\0'
    };
    if (strcmp(magic, "TEST") != 0) {
        fprintf(stderr, "ERROR: Bad SRAM magic: %s\n", magic);
        core->deinit(core);
        free(vidbuf);
        return 1;
    }

    int total  = core->busRead8(core, SRAM_BASE + 4);
    int passed = core->busRead8(core, SRAM_BASE + 5);
    int failed = core->busRead8(core, SRAM_BASE + 6);

    printf("==================================================\n");
    printf("  AS&S Game Test Results\n");
    printf("==================================================\n");
    printf("  Total:  %d\n", total);
    printf("  Passed: %d\n", passed);
    printf("  Failed: %d\n", failed);
    printf("==================================================\n");

    if (failed > 0) {
        printf("\nFailed tests:\n");
        for (int i = 0; i < total; i++) {
            if (core->busRead8(core, SRAM_BASE + 8 + i) != 0) {
                printf("  Test #%d: FAIL\n", i);
            }
        }
    }

    printf("\n%s\n", failed == 0 ? "ALL TESTS PASSED" : "SOME TESTS FAILED");

    core->deinit(core);
    free(vidbuf);
    return failed == 0 ? 0 : 1;
}
