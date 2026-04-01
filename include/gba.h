// gba.h — GBA hardware register definitions
// Advanced Sagebrush & Shootouts: The Game
#ifndef GBA_H
#define GBA_H

#include <stdint.h>

// --- Type aliases ---
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef int8_t   s8;
typedef int16_t  s16;
typedef int32_t  s32;
typedef volatile u16 vu16;
typedef volatile u32 vu32;

// --- Memory-mapped I/O base addresses ---
#define MEM_IO       0x04000000
#define MEM_VRAM     0x06000000
#define MEM_PAL_BG   0x05000000
#define MEM_PAL_OBJ  0x05000200
#define MEM_OAM      0x07000000
#define MEM_EWRAM    0x02000000

// --- Display control ---
#define REG_DISPCNT  (*(vu16*)(MEM_IO + 0x0000))
#define REG_DISPSTAT (*(vu16*)(MEM_IO + 0x0004))
#define REG_VCOUNT   (*(vu16*)(MEM_IO + 0x0006))

// Display modes
#define DCNT_MODE0   0x0000  // Tile mode 0 (4 BG layers)
#define DCNT_MODE1   0x0001  // Tile mode 1 (2 regular + 1 affine)
#define DCNT_MODE3   0x0003  // Bitmap mode 3 (240x160, 16bpp)
#define DCNT_MODE4   0x0004  // Bitmap mode 4 (240x160, 8bpp, paged)
#define DCNT_OBJ     0x1000  // Enable sprites
#define DCNT_OBJ_1D  0x0040  // 1D sprite mapping
#define DCNT_BG0     0x0100
#define DCNT_BG1     0x0200
#define DCNT_BG2     0x0400
#define DCNT_BG3     0x0800

// --- Background control ---
#define REG_BG0CNT   (*(vu16*)(MEM_IO + 0x0008))
#define REG_BG1CNT   (*(vu16*)(MEM_IO + 0x000A))
#define REG_BG2CNT   (*(vu16*)(MEM_IO + 0x000C))
#define REG_BG3CNT   (*(vu16*)(MEM_IO + 0x000E))

#define REG_BG0HOFS  (*(vu16*)(MEM_IO + 0x0010))
#define REG_BG0VOFS  (*(vu16*)(MEM_IO + 0x0012))
#define REG_BG1HOFS  (*(vu16*)(MEM_IO + 0x0014))
#define REG_BG1VOFS  (*(vu16*)(MEM_IO + 0x0016))
#define REG_BG2HOFS  (*(vu16*)(MEM_IO + 0x0018))
#define REG_BG2VOFS  (*(vu16*)(MEM_IO + 0x001A))
#define REG_BG3HOFS  (*(vu16*)(MEM_IO + 0x001C))
#define REG_BG3VOFS  (*(vu16*)(MEM_IO + 0x001E))

// BG control bits
#define BG_PRIO(n)     ((n) & 3)
#define BG_CBB(n)      (((n) & 3) << 2)   // Character base block
#define BG_SBB(n)      (((n) & 31) << 8)  // Screen base block
#define BG_4BPP        0x0000
#define BG_8BPP        0x0080
#define BG_SIZE_256x256  0x0000
#define BG_SIZE_512x256  0x4000
#define BG_SIZE_256x512  0x8000
#define BG_SIZE_512x512  0xC000

// --- Input ---
#define REG_KEYINPUT (*(vu16*)(MEM_IO + 0x0130))

#define KEY_A        0x0001
#define KEY_B        0x0002
#define KEY_SELECT   0x0004
#define KEY_START    0x0008
#define KEY_RIGHT    0x0010
#define KEY_LEFT     0x0020
#define KEY_UP       0x0040
#define KEY_DOWN     0x0080
#define KEY_R        0x0100
#define KEY_L        0x0200

// --- DMA ---
#define REG_DMA3SAD  (*(vu32*)(MEM_IO + 0x00D4))
#define REG_DMA3DAD  (*(vu32*)(MEM_IO + 0x00D8))
#define REG_DMA3CNT  (*(vu32*)(MEM_IO + 0x00DC))

#define DMA_ENABLE   0x80000000
#define DMA_NOW      0x00000000
#define DMA_16       0x00000000
#define DMA_32       0x04000000

// --- Timer ---
#define REG_TM0CNT_L (*(vu16*)(MEM_IO + 0x0100))
#define REG_TM0CNT_H (*(vu16*)(MEM_IO + 0x0102))

#define TM_ENABLE    0x0080
#define TM_IRQ       0x0040
#define TM_FREQ_1    0x0000
#define TM_FREQ_64   0x0001
#define TM_FREQ_256  0x0002
#define TM_FREQ_1024 0x0003

// --- Interrupt ---
#define REG_IE       (*(vu16*)(MEM_IO + 0x0200))
#define REG_IF       (*(vu16*)(MEM_IO + 0x0202))
#define REG_IME      (*(vu16*)(MEM_IO + 0x0208))

#define IRQ_VBLANK   0x0001

// --- OAM (sprite) definitions ---
typedef struct {
    u16 attr0;
    u16 attr1;
    u16 attr2;
    s16 pad;   // affine data / padding
} OBJ_ATTR;

#define OAM_MEM      ((OBJ_ATTR*)(MEM_OAM))

// attr0
#define ATTR0_Y(y)        ((y) & 0xFF)
#define ATTR0_REG         0x0000
#define ATTR0_AFF         0x0100
#define ATTR0_HIDE        0x0200
#define ATTR0_AFF_DBL     0x0300
#define ATTR0_SQUARE      0x0000
#define ATTR0_WIDE        0x4000
#define ATTR0_TALL        0x8000
#define ATTR0_4BPP        0x0000
#define ATTR0_8BPP        0x2000

// attr1
#define ATTR1_X(x)        ((x) & 0x1FF)
#define ATTR1_SIZE_8      0x0000
#define ATTR1_SIZE_16     0x4000
#define ATTR1_SIZE_32     0x8000
#define ATTR1_SIZE_64     0xC000
#define ATTR1_HFLIP       0x1000
#define ATTR1_VFLIP       0x2000

// attr2
#define ATTR2_TILE(t)     ((t) & 0x3FF)
#define ATTR2_PRIO(p)     (((p) & 3) << 10)
#define ATTR2_PALBANK(p)  (((p) & 15) << 12)

// --- Tile/map memory helpers ---
#define TILE_MEM    ((u32*)(MEM_VRAM))
#define TILE8_MEM   ((u32*)(MEM_VRAM))
#define SE_MEM      ((u16*)(MEM_VRAM))

// Charblock = 16KB, Screenblock = 2KB
#define CBB_ADDR(n)  (MEM_VRAM + (n) * 0x4000)
#define SBB_ADDR(n)  (MEM_VRAM + (n) * 0x0800)

// Screen entry helpers
#define SE_TILE(t)    ((t) & 0x3FF)
#define SE_HFLIP      0x0400
#define SE_VFLIP      0x0800
#define SE_PALBANK(p) (((p) & 15) << 12)

// --- Utility macros ---
#define RGB15(r,g,b) ((r) | ((g)<<5) | ((b)<<10))

static inline void vsync(void) {
    while (REG_VCOUNT >= 160);
    while (REG_VCOUNT <  160);
}

// DMA3 copy (fast, word-aligned)
static inline void dma3_copy(void *dst, const void *src, u32 count, u32 mode) {
    REG_DMA3SAD = (u32)src;
    REG_DMA3DAD = (u32)dst;
    REG_DMA3CNT = count | mode | DMA_ENABLE;
}

#endif // GBA_H
