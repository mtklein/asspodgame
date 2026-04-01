#!/usr/bin/env python3
"""Export tileset_urban from game_data.c as a PNG image for Tiled."""
import re, struct, sys, zlib

def parse_tileset_and_palette(src_path):
    with open(src_path) as f:
        src = f.read()

    def extract_hex_array(name, src):
        pattern = rf'{re.escape(name)}\s*\[[^\]]*\]\s*=\s*\{{([^;]+)\}};'
        m = re.search(pattern, src, re.DOTALL)
        if not m:
            raise ValueError(f"Could not find array {name}")
        body = m.group(1)
        # Strip C comments
        body = re.sub(r'//[^\n]*', '', body)
        vals = []
        for tok in re.findall(r'0[xX][0-9a-fA-F]+', body):
            vals.append(int(tok, 16))
        return vals

    tile_words = extract_hex_array('tileset_urban', src)  # 512 u32
    # Palette is parsed from RGB15() macro calls, not the hex array

    # Parse palette — the source uses RGB15(r,g,b) macro calls
    pal_pattern = r'RGB15\(\s*(\d+)\s*,\s*(\d+)\s*,\s*(\d+)\s*\)'
    pal_matches = re.findall(pal_pattern, src[src.index('tileset_urban_pal'):])
    palette = []
    for r, g, b in pal_matches[:16]:
        r8 = int(r) * 255 // 31
        g8 = int(g) * 255 // 31
        b8 = int(b) * 255 // 31
        palette.append((r8, g8, b8))

    return tile_words, palette

def decode_tiles(tile_words, palette, columns=8):
    num_tiles = len(tile_words) // 8
    rows = (num_tiles + columns - 1) // columns
    img_w = columns * 8
    img_h = rows * 8
    pixels = [(0, 0, 0, 0)] * (img_w * img_h)

    for t in range(num_tiles):
        tx = (t % columns) * 8
        ty = (t // columns) * 8
        for row in range(8):
            word = tile_words[t * 8 + row]
            for px in range(8):
                idx = (word >> (px * 4)) & 0xF
                if idx == 0:
                    rgba = (0, 0, 0, 0)  # transparent
                else:
                    r, g, b = palette[idx] if idx < len(palette) else (255, 0, 255)
                    rgba = (r, g, b, 255)
                pixels[(ty + row) * img_w + (tx + px)] = rgba

    return img_w, img_h, pixels

def write_png(path, width, height, pixels):
    def chunk(ctype, data):
        c = ctype + data
        return struct.pack('>I', len(data)) + c + struct.pack('>I', zlib.crc32(c) & 0xFFFFFFFF)

    raw = b''
    for y in range(height):
        raw += b'\x00'  # filter: none
        for x in range(width):
            r, g, b, a = pixels[y * width + x]
            raw += struct.pack('BBBB', r, g, b, a)

    sig = b'\x89PNG\r\n\x1a\n'
    ihdr = struct.pack('>IIBBBBB', width, height, 8, 6, 0, 0, 0)
    idat = zlib.compress(raw)

    with open(path, 'wb') as f:
        f.write(sig)
        f.write(chunk(b'IHDR', ihdr))
        f.write(chunk(b'IDAT', idat))
        f.write(chunk(b'IEND', b''))

if __name__ == '__main__':
    src = sys.argv[1] if len(sys.argv) > 1 else 'src/game_data.c'
    out = sys.argv[2] if len(sys.argv) > 2 else 'levels/tilesets/tileset_urban.png'
    tile_words, palette = parse_tileset_and_palette(src)
    w, h, pixels = decode_tiles(tile_words, palette)
    write_png(out, w, h, pixels)
    print(f"Wrote {w}x{h} PNG to {out}")
