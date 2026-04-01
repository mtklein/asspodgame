#!/usr/bin/env python3
"""Convert a tileset PNG to GBA 4bpp tile data.

Usage: bake_tileset.py <png_path> <palette_json_path> <output.c> <output.h>

Reads a PNG tileset image (RGBA, 8-bit) and a palette JSON file (16 entries of
{r,g,b} in GBA RGB15 range 0-31), then outputs a C source/header pair with the
tile data packed in GBA 4bpp format.
"""

import json
import struct
import sys
import zlib


# ---------------------------------------------------------------------------
# Minimal pure-Python PNG reader (RGBA 8-bit, all 5 filter types)
# ---------------------------------------------------------------------------

def _paeth(a, b, c):
    """Paeth predictor used by PNG filter type 4."""
    p = a + b - c
    pa = abs(p - a)
    pb = abs(p - b)
    pc = abs(p - c)
    if pa <= pb and pa <= pc:
        return a
    if pb <= pc:
        return b
    return c


def read_png(path):
    """Read an RGBA 8-bit PNG and return (width, height, pixels).

    *pixels* is a list of (r, g, b, a) tuples in row-major order.
    Handles all five PNG filter types (None, Sub, Up, Average, Paeth).
    """
    with open(path, "rb") as f:
        data = f.read()

    # Validate PNG signature.
    sig = b"\x89PNG\r\n\x1a\n"
    if data[:8] != sig:
        raise ValueError("Not a valid PNG file")

    pos = 8
    width = height = 0
    idat_chunks = []

    while pos < len(data):
        length = struct.unpack(">I", data[pos:pos + 4])[0]
        chunk_type = data[pos + 4:pos + 8]
        chunk_data = data[pos + 8:pos + 8 + length]
        # skip CRC (4 bytes after chunk data)
        pos += 12 + length

        if chunk_type == b"IHDR":
            width = struct.unpack(">I", chunk_data[0:4])[0]
            height = struct.unpack(">I", chunk_data[4:8])[0]
            bit_depth = chunk_data[8]
            color_type = chunk_data[9]
            if bit_depth != 8 or color_type != 6:
                raise ValueError(
                    f"Unsupported PNG format: bit_depth={bit_depth}, "
                    f"color_type={color_type} (need 8-bit RGBA)"
                )
        elif chunk_type == b"IDAT":
            idat_chunks.append(chunk_data)
        elif chunk_type == b"IEND":
            break

    raw = zlib.decompress(b"".join(idat_chunks))

    # Unfilter.  Each row is: 1 filter byte + width*4 payload bytes.
    bpp = 4  # bytes per pixel (RGBA)
    stride = width * bpp
    prev_row = bytearray(stride)  # "previous row" starts as all zeros
    pixels = []

    offset = 0
    for _y in range(height):
        filter_type = raw[offset]
        offset += 1
        cur_row = bytearray(raw[offset:offset + stride])
        offset += stride

        if filter_type == 0:
            pass  # None
        elif filter_type == 1:
            # Sub: each byte adds the byte *bpp* positions to its left.
            for i in range(stride):
                left = cur_row[i - bpp] if i >= bpp else 0
                cur_row[i] = (cur_row[i] + left) & 0xFF
        elif filter_type == 2:
            # Up: each byte adds the byte above it.
            for i in range(stride):
                cur_row[i] = (cur_row[i] + prev_row[i]) & 0xFF
        elif filter_type == 3:
            # Average: each byte adds floor((left + above) / 2).
            for i in range(stride):
                left = cur_row[i - bpp] if i >= bpp else 0
                cur_row[i] = (cur_row[i] + ((left + prev_row[i]) >> 1)) & 0xFF
        elif filter_type == 4:
            # Paeth
            for i in range(stride):
                left = cur_row[i - bpp] if i >= bpp else 0
                up = prev_row[i]
                up_left = prev_row[i - bpp] if i >= bpp else 0
                cur_row[i] = (cur_row[i] + _paeth(left, up, up_left)) & 0xFF
        else:
            raise ValueError(f"Unknown PNG filter type {filter_type}")

        # Extract RGBA tuples from this row.
        for x in range(width):
            base = x * 4
            pixels.append((cur_row[base], cur_row[base + 1],
                           cur_row[base + 2], cur_row[base + 3]))
        prev_row = cur_row

    return width, height, pixels


# ---------------------------------------------------------------------------
# Palette matching
# ---------------------------------------------------------------------------

def load_palette(path):
    """Load palette JSON and return list of (r8, g8, b8) tuples and the raw
    RGB15 entries."""
    with open(path, "r") as f:
        entries = json.load(f)
    if len(entries) != 16:
        raise ValueError(f"Expected 16 palette entries, got {len(entries)}")
    rgb8 = []
    rgb15 = []
    for e in entries:
        r5, g5, b5 = e["r"], e["g"], e["b"]
        rgb15.append((r5, g5, b5))
        rgb8.append((r5 * 255 // 31, g5 * 255 // 31, b5 * 255 // 31))
    return rgb8, rgb15


def match_pixel(r, g, b, a, palette_rgb8, cache):
    """Return the palette index closest to (r, g, b, a).

    Transparent pixels (alpha == 0) always map to index 0.
    """
    if a == 0:
        return 0
    key = (r, g, b)
    if key in cache:
        return cache[key]
    best_idx = 0
    best_dist = float("inf")
    for idx, (pr, pg, pb) in enumerate(palette_rgb8):
        dr = r - pr
        dg = g - pg
        db = b - pb
        dist = dr * dr + dg * dg + db * db
        if dist < best_dist:
            best_dist = dist
            best_idx = idx
            if dist == 0:
                break
    cache[key] = best_idx
    return best_idx


# ---------------------------------------------------------------------------
# Tile packing
# ---------------------------------------------------------------------------

def pack_tiles(width, height, pixels, palette_rgb8):
    """Pack pixel data into GBA 4bpp tile words.

    The image is treated as an 8-column grid of 8x8 tiles.  Returns a flat
    list of u32 words (8 words per tile, tiles in row-major order).
    """
    tiles_x = 8
    tiles_y = height // 8
    num_tiles = tiles_x * tiles_y

    cache = {}
    words = []

    for t in range(num_tiles):
        tx = t % tiles_x
        ty = t // tiles_x
        for row in range(8):
            word = 0
            for col in range(8):
                px = tx * 8 + col
                py = ty * 8 + row
                r, g, b, a = pixels[py * width + px]
                idx = match_pixel(r, g, b, a, palette_rgb8, cache)
                word |= (idx & 0xF) << (col * 4)
            words.append(word)

    return words, num_tiles


# ---------------------------------------------------------------------------
# Code generation
# ---------------------------------------------------------------------------

def write_header(path, num_tiles):
    with open(path, "w") as f:
        f.write("#ifndef GEN_TILESET_H\n")
        f.write("#define GEN_TILESET_H\n")
        f.write('#include "gba.h"\n')
        f.write(f"#define TILESET_URBAN_TILES {num_tiles}\n")
        f.write(f"#define TILESET_URBAN_SIZE (TILESET_URBAN_TILES * 32)\n")
        f.write(f"extern const u32 tileset_urban[TILESET_URBAN_TILES * 8];\n")
        f.write(f"extern const u16 tileset_urban_pal[16];\n")
        f.write("#endif\n")


def write_source(path, words, rgb15, num_tiles):
    with open(path, "w") as f:
        f.write('#include "gen_tileset.h"\n\n')

        # Palette
        f.write("const u16 tileset_urban_pal[16] = {\n")
        for i, (r, g, b) in enumerate(rgb15):
            comma = "," if i < 15 else ""
            f.write(f"    RGB15({r:2d},{g:2d},{b:2d}){comma}\n")
        f.write("};\n\n")

        # Tile data
        f.write(f"const u32 tileset_urban[TILESET_URBAN_TILES * 8] = {{\n")
        for i, w in enumerate(words):
            if i % 16 == 0:
                f.write("    ")
            f.write(f"0x{w:08X}")
            if i < len(words) - 1:
                f.write(",")
                if i % 16 == 15:
                    f.write("\n")
                else:
                    f.write(" ")
            else:
                f.write("\n")
        f.write("};\n")


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

def main():
    if len(sys.argv) != 5:
        print(f"Usage: {sys.argv[0]} <png> <palette.json> <output.c> <output.h>",
              file=sys.stderr)
        sys.exit(1)

    png_path = sys.argv[1]
    pal_path = sys.argv[2]
    out_c = sys.argv[3]
    out_h = sys.argv[4]

    palette_rgb8, palette_rgb15 = load_palette(pal_path)
    width, height, pixels = read_png(png_path)

    if width != 64:
        raise ValueError(f"Expected 64px wide tileset (8 columns of 8px tiles), got {width}")
    if height % 8 != 0:
        raise ValueError(f"Tileset height must be a multiple of 8, got {height}")

    words, num_tiles = pack_tiles(width, height, pixels, palette_rgb8)

    write_header(out_h, num_tiles)
    write_source(out_c, words, palette_rgb15, num_tiles)


if __name__ == "__main__":
    main()
