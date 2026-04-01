#!/usr/bin/env python3
"""
gensprites.py — Generate 4bpp sprite data for AS&S characters.
Each character: 4 directions × 2 frames × 4 tiles (16×16 @ 4bpp) = 32 tiles
Each tile: 8×8 pixels @ 4bpp = 32 bytes = 8 u32 words
Total per character: 32 × 8 = 256 u32 words

GBA 4bpp tile layout: each u32 holds 8 pixels, low nibble = leftmost pixel.
For a 16×16 sprite in OBJ 1D mode, the 4 tiles are:
  tile 0: top-left 8×8    tile 1: top-right 8×8
  tile 2: bottom-left 8×8 tile 3: bottom-right 8×8

Actually in 1D mapping for 16×16 square sprite, the tile order is:
  tile+0: top-left, tile+1: bottom-left, tile+2: top-right, tile+3: bottom-right
Wait - GBA stores them as: tile0=TL, tile1=BL, tile2=TR, tile3=BR for 1D mapping.
No - in 1D mode, 16×16 uses tiles [n, n+1, n+2, n+3] in the pattern:
  Row 0-7:  tile n (left half),  tile n+2 (right half)  -- No wait.

Actually for GBA 1D OBJ mapping, a 16×16 4bpp sprite uses 4 tiles:
  tile_id+0 = rows 0-7, cols 0-7 (top-left)
  tile_id+1 = rows 0-7, cols 8-15 (top-right)
  tile_id+2 = rows 8-15, cols 0-7 (bottom-left)
  tile_id+3 = rows 8-15, cols 8-15 (bottom-right)
That's correct for 1D mapping: tiles are sequential.
"""

def img_to_tiles(img):
    """Convert a 16×16 pixel image (list of 16 rows, 16 pixels each) to 4 tiles.
    Each tile is 8 u32 words."""
    tiles = []
    # 4 tiles: TL(0,0), TR(0,8), BL(8,0), BR(8,8) — in 1D order: TL, TR, BL, BR
    for ty in range(2):
        for tx in range(2):
            tile = []
            for row in range(8):
                word = 0
                for col in range(8):
                    px = img[ty*8 + row][tx*8 + col]
                    word |= (px & 0xF) << (col * 4)
                tile.append(word)
            tiles.extend(tile)
    return tiles

def make_trevor(direction, frame):
    """Trevor Steel: broad shoulders, dark suit (7=blue), tie (6=red), skin (4)"""
    img = [[0]*16 for _ in range(16)]
    # Head (rows 1-5, centered)
    for r in range(1, 5):
        for c in range(5, 11):
            img[r][c] = 4  # skin
    # Hair top
    for c in range(5, 11):
        img[0][c] = 5  # tan/dark hair
    # Eyes (row 2)
    if direction != 1:  # not facing up
        img[2][6] = 0; img[2][9] = 0
    # Suit body (rows 5-12)
    for r in range(5, 12):
        for c in range(3, 13):
            img[r][c] = 7  # blue suit
    # Tie
    for r in range(5, 10):
        img[r][7] = 6; img[r][8] = 6
    # Shoulders extra wide
    for r in range(5, 8):
        img[r][2] = 7; img[r][13] = 7
    # Legs (rows 12-15)
    for r in range(12, 15):
        img[r][5] = 2; img[r][6] = 2
        img[r][9] = 2; img[r][10] = 2
    # Walk frame: shift one leg
    if frame == 1:
        img[14][5] = 0; img[14][6] = 0
        img[14][9] = 2; img[14][10] = 2
        img[15][9] = 2; img[15][10] = 2
    return img

def make_kip(direction, frame):
    """Kip Simpkins: spiky hair (11=magenta), casual clothes (6=red jacket), skin (4)"""
    img = [[0]*16 for _ in range(16)]
    # Spiky hair (rows 0-2)
    spikes = [6,7,8,9]
    for c in spikes:
        img[0][c] = 11
    for c in range(4, 12):
        img[1][c] = 11
    for c in range(5, 11):
        img[2][c] = 11
    # Extra spikes
    img[0][4] = 11; img[0][11] = 11
    # Head (rows 2-5)
    for r in range(2, 6):
        for c in range(5, 11):
            img[r][c] = 4  # skin
    # Eyes
    if direction != 1:
        img[3][6] = 0; img[3][9] = 0
    # Red jacket body (rows 6-12)
    for r in range(6, 12):
        for c in range(4, 12):
            img[r][c] = 6
    # Jeans (rows 12-15)
    for r in range(12, 15):
        img[r][5] = 7; img[r][6] = 7
        img[r][9] = 7; img[r][10] = 7
    if frame == 1:
        img[14][5] = 0; img[14][6] = 0
        img[15][9] = 7; img[15][10] = 7
    return img

def make_rubik(direction, frame):
    """Jim Rubik: round/stocky, grey suit (3), skin (4), bald-ish"""
    img = [[0]*16 for _ in range(16)]
    # Round head (rows 1-6)
    for r in range(1, 6):
        for c in range(4, 12):
            img[r][c] = 4
    # Balding top
    for c in range(5, 11):
        img[0][c] = 5
    # Eyes
    if direction != 1:
        img[3][6] = 0; img[3][9] = 0
    # Stocky grey body (rows 6-13, wider)
    for r in range(6, 13):
        for c in range(2, 14):
            img[r][c] = 3  # grey
    # Legs
    for r in range(13, 15):
        img[r][5] = 2; img[r][6] = 2
        img[r][9] = 2; img[r][10] = 2
    if frame == 1:
        img[14][5] = 0; img[14][10] = 0
    return img

def make_generic(direction, frame):
    """Generic NPC: simple humanoid, brown outfit (5), skin (4)"""
    img = [[0]*16 for _ in range(16)]
    # Head
    for r in range(2, 6):
        for c in range(5, 11):
            img[r][c] = 4
    for c in range(6, 10):
        img[1][c] = 4
    # Eyes
    if direction != 1:
        img[3][6] = 0; img[3][9] = 0
    # Brown body
    for r in range(6, 12):
        for c in range(4, 12):
            img[r][c] = 5
    # Legs
    for r in range(12, 15):
        img[r][6] = 2; img[r][7] = 2
        img[r][8] = 2; img[r][9] = 2
    if frame == 1:
        img[14][6] = 0; img[14][7] = 0
    return img

def mirror_h(img):
    """Flip image horizontally"""
    return [row[::-1] for row in img]

def gen_character(make_fn, name):
    """Generate all 32 tiles (4 dirs × 2 frames × 4 tiles) for a character."""
    all_tiles = []
    # Direction order: DOWN=0, UP=1, LEFT=2, RIGHT=3
    for direction in range(4):
        for frame in range(2):
            if direction == 3:  # RIGHT = mirror of LEFT
                img = mirror_h(make_fn(2, frame))
            else:
                img = make_fn(direction, frame)
            all_tiles.extend(img_to_tiles(img))
    return all_tiles

def print_array(name, data):
    print(f"const u32 {name}[SPR_WORDS_PER_CHAR] = {{")
    for i, val in enumerate(data):
        if i % 8 == 0:
            tile = i // 8
            direction = tile // (2*4)
            frame = (tile // 4) % 2
            sub = tile % 4
            dirs = ["DOWN","UP","LEFT","RIGHT"]
            subs = ["TL","TR","BL","BR"]
            if sub == 0:
                print(f"    // {dirs[direction]} frame {frame}")
            print("    ", end="")
        print(f"0x{val:08X},", end="")
        if i % 8 == 7:
            print()
        else:
            print(" ", end="")
    print("};")
    print()

if __name__ == "__main__":
    print("// sprite_data.h — Auto-generated by gensprites.py")
    print("// DO NOT EDIT BY HAND")
    print('#include "game_data.h"')
    print()

    trevor = gen_character(make_trevor, "trevor")
    kip = gen_character(make_kip, "kip")
    rubik = gen_character(make_rubik, "rubik")
    generic = gen_character(make_generic, "generic")

    print_array("spr_trevor", trevor)
    print_array("spr_kip", kip)
    print_array("spr_npc_rubik", rubik)
    print_array("spr_npc_generic", generic)
