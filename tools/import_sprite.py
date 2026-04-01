#!/usr/bin/env python3
"""import_sprite.py -- Convert a reference PNG into GBA 4bpp sprite data.

Usage: import_sprite.py <input.png> <palette_index> <output.c> <output.h> [--name NAME]

The input PNG should be a sprite sheet laid out as a 4-column x 2-row grid:
  Column 0-3: directions DOWN, UP, LEFT, RIGHT
  Row 0-1: walk frames 0, 1

Each cell is a square (the script auto-detects cell size from image dimensions).
The script scales each cell to 16x16 and quantizes to the character sprite palette.

palette_index: 0 = character sprite palette (spr_pal_characters from game_data.c)
"""

import json, os, struct, sys, zlib


def read_png(path):
    with open(path, "rb") as f:
        sig = f.read(8)
        if sig != b"\x89PNG\r\n\x1a\n":
            raise ValueError("Not a PNG file")

        width = height = bit_depth = color_type = 0
        idat_chunks = []

        while True:
            header = f.read(8)
            if len(header) < 8:
                break
            length = struct.unpack(">I", header[:4])[0]
            chunk_type = header[4:8]
            data = f.read(length)
            f.read(4)  # CRC

            if chunk_type == b"IHDR":
                width, height, bit_depth, color_type = struct.unpack(">IIBB", data[:10])
            elif chunk_type == b"IDAT":
                idat_chunks.append(data)
            elif chunk_type == b"IEND":
                break

    raw = zlib.decompress(b"".join(idat_chunks))
    bpp = 4 if color_type == 6 else 3  # RGBA or RGB
    stride = 1 + width * bpp
    pixels = []
    prev_row = bytes(width * bpp)

    for y in range(height):
        row_start = y * stride
        filter_type = raw[row_start]
        row_data = bytearray(raw[row_start + 1 : row_start + stride])

        for i in range(len(row_data)):
            a = row_data[i - bpp] if i >= bpp else 0
            b = prev_row[i]
            c = prev_row[i - bpp] if i >= bpp else 0

            if filter_type == 0:
                pass
            elif filter_type == 1:
                row_data[i] = (row_data[i] + a) & 0xFF
            elif filter_type == 2:
                row_data[i] = (row_data[i] + b) & 0xFF
            elif filter_type == 3:
                row_data[i] = (row_data[i] + (a + b) // 2) & 0xFF
            elif filter_type == 4:
                p = a + b - c
                pa, pb, pc = abs(p - a), abs(p - b), abs(p - c)
                if pa <= pb and pa <= pc:
                    pr = a
                elif pb <= pc:
                    pr = b
                else:
                    pr = c
                row_data[i] = (row_data[i] + pr) & 0xFF

        prev_row = bytes(row_data)

        for x in range(width):
            off = x * bpp
            r, g, b = row_data[off], row_data[off + 1], row_data[off + 2]
            a_val = row_data[off + 3] if bpp == 4 else 255
            pixels.append((r, g, b, a_val))

    return width, height, pixels


# Character sprite palette (spr_pal_characters) in RGB5
PALETTES = {
    0: [
        (0, 0, 0), (31, 31, 31), (12, 12, 15), (22, 22, 22),
        (28, 20, 14), (20, 15, 8), (28, 4, 4), (4, 4, 28),
        (15, 15, 0), (28, 28, 0), (0, 18, 0), (28, 0, 28),
        (0, 28, 0), (18, 0, 0), (0, 8, 18), (15, 15, 15),
    ],
}


def rgb5_to_rgb8(r5, g5, b5):
    return (r5 * 255 // 31, g5 * 255 // 31, b5 * 255 // 31)


def nearest_palette_index(r, g, b, a, palette_rgb8):
    if a < 128:
        return 0  # transparent
    best_idx = 0
    best_dist = 999999
    # Skip index 0 (transparent) for opaque pixels
    for i in range(1, len(palette_rgb8)):
        pr, pg, pb = palette_rgb8[i]
        d = (r - pr) ** 2 + (g - pg) ** 2 + (b - pb) ** 2
        if d < best_dist:
            best_dist = d
            best_idx = i
    return best_idx


def scale_nearest(pixels, src_w, src_h, dst_w, dst_h):
    out = []
    for y in range(dst_h):
        sy = y * src_h // dst_h
        for x in range(dst_w):
            sx = x * src_w // dst_w
            out.append(pixels[sy * src_w + sx])
    return out


def extract_cell(pixels, img_w, cx, cy, cell_w, cell_h):
    cell = []
    for y in range(cell_h):
        for x in range(cell_w):
            px = cx * cell_w + x
            py = cy * cell_h + y
            cell.append(pixels[py * img_w + px])
    return cell


def pack_tile_4bpp(indices_16x16):
    """Pack a 16x16 sprite into 4 8x8 tiles (2x2 arrangement, 4bpp)."""
    words = []
    # GBA 16x16 sprite = 4 tiles in 1D mapping: top-left, top-right, bottom-left, bottom-right
    for tile_y in range(2):
        for tile_x in range(2):
            for row in range(8):
                word = 0
                for px in range(8):
                    idx = indices_16x16[(tile_y * 8 + row) * 16 + (tile_x * 8 + px)]
                    word |= (idx & 0xF) << (px * 4)
                words.append(word)
    return words


def main():
    if len(sys.argv) < 5:
        print("Usage: import_sprite.py <input.png> <palette_index> <output.c> <output.h> [--name NAME]",
              file=sys.stderr)
        sys.exit(1)

    png_path = sys.argv[1]
    pal_idx = int(sys.argv[2])
    out_c = sys.argv[3]
    out_h = sys.argv[4]

    name = os.path.splitext(os.path.basename(png_path))[0]
    for i in range(5, len(sys.argv) - 1):
        if sys.argv[i] == "--name":
            name = sys.argv[i + 1]

    # Load palette
    pal5 = PALETTES[pal_idx]
    pal8 = [rgb5_to_rgb8(r, g, b) for r, g, b in pal5]

    # Read image
    img_w, img_h, pixels = read_png(png_path)

    # Expect 4 columns x 2 rows grid
    cell_w = img_w // 4
    cell_h = img_h // 2

    # Direction order: DOWN, UP, LEFT, RIGHT (columns 0-3)
    # Frame order: frame 0 (row 0), frame 1 (row 1)
    all_words = []
    for direction in range(4):
        for frame in range(2):
            cell = extract_cell(pixels, img_w, direction, frame, cell_w, cell_h)
            scaled = scale_nearest(cell, cell_w, cell_h, 16, 16)
            indices = [nearest_palette_index(r, g, b, a, pal8) for r, g, b, a in scaled]
            words = pack_tile_4bpp(indices)
            all_words.extend(words)

    # Should be 32 tiles * 8 words = 256 words
    assert len(all_words) == 256, f"Expected 256 words, got {len(all_words)}"

    # Write header
    c_name = "spr_" + name
    h_guard = "GEN_SPR_%s_H" % name.upper()

    with open(out_h, "w") as f:
        f.write("#ifndef %s\n" % h_guard)
        f.write("#define %s\n" % h_guard)
        f.write('#include "gba.h"\n')
        f.write("extern const u32 %s[256];\n" % c_name)
        f.write("#endif\n")

    # Write source
    with open(out_c, "w") as f:
        f.write('#include "%s"\n\n' % os.path.basename(out_h))
        f.write("const u32 %s[256] = {\n" % c_name)
        for i in range(0, len(all_words), 8):
            chunk = all_words[i:i + 8]
            f.write("    " + ",".join("0x%08X" % w for w in chunk) + ",\n")
        f.write("};\n")

    print("Wrote %s: %d tiles from %dx%d image" % (c_name, len(all_words) // 8, img_w, img_h))


if __name__ == "__main__":
    main()
