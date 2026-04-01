#!/usr/bin/env python3
"""gen_all_sprites.py -- Full pipeline: generate all character sprites and import to GBA format.

For each character JSON in levels/characters/:
1. Generate front/back/left/right views via ComfyUI
2. Assemble into a 4x2 sprite sheet (directions x walk frames)
3. Import to GBA 4bpp format via import_sprite.py logic

Usage: gen_all_sprites.py [--seed N] [--characters NAME,NAME,...]
"""

import json, os, struct, sys, time, urllib.request, urllib.error, zlib

URL = os.environ.get("COMFYUI_URL", "http://127.0.0.1:8000")
SEED = 123
SPRITE_DIR = "levels/characters/sprites"
OUT_DIR = "out/gen_sprites"

STYLE = ("pixelsprite, single character, {direction} facing, standing, pixel art, "
         "game character, solid color background, centered, full body, chibi proportions, "
         "big head, 16-bit style, retro RPG")

DIRECTIONS = [
    ("front", "front"),
    ("back",  "back rear behind"),
    ("left",  "left side profile facing left"),
    ("right", "right side profile facing right"),
]

NEGATIVE = ("blurry, 3d, realistic, photo, text, watermark, multiple characters, "
            "crowd, background scenery")


def comfyui_generate(prompt, neg, seed, prefix, width=512, height=512):
    workflow = {
        "3": {"class_type": "KSampler", "inputs": {
            "seed": seed, "steps": 25, "cfg": 7.5,
            "sampler_name": "euler_ancestral", "scheduler": "normal",
            "denoise": 1.0, "model": ["4", 0], "positive": ["6", 0],
            "negative": ["7", 0], "latent_image": ["5", 0]}},
        "4": {"class_type": "CheckpointLoaderSimple", "inputs": {
            "ckpt_name": "allInOnePixelModel_v1.ckpt"}},
        "5": {"class_type": "EmptyLatentImage", "inputs": {
            "width": width, "height": height, "batch_size": 1}},
        "6": {"class_type": "CLIPTextEncode", "inputs": {
            "text": prompt, "clip": ["4", 1]}},
        "7": {"class_type": "CLIPTextEncode", "inputs": {
            "text": neg, "clip": ["4", 1]}},
        "8": {"class_type": "VAEDecode", "inputs": {
            "samples": ["3", 0], "vae": ["4", 2]}},
        "9": {"class_type": "SaveImage", "inputs": {
            "filename_prefix": prefix, "images": ["8", 0]}},
    }
    payload = json.dumps({"prompt": workflow}).encode()
    req = urllib.request.Request(URL + "/prompt", data=payload,
                                 headers={"Content-Type": "application/json"})
    resp = urllib.request.urlopen(req)
    pid = json.loads(resp.read())["prompt_id"]
    while True:
        time.sleep(1)
        h = json.loads(urllib.request.urlopen(URL + "/history/" + pid).read())
        if pid in h:
            for node_out in h[pid]["outputs"].values():
                for img in node_out.get("images", []):
                    return img["filename"]
            return None


def download_comfyui_image(filename, dest_path):
    url = "%s/view?filename=%s&type=output" % (URL, filename)
    urllib.request.urlretrieve(url, dest_path)


def read_png(path):
    with open(path, "rb") as f:
        sig = f.read(8)
        w = h = 0
        idat = []
        while True:
            hdr = f.read(8)
            if len(hdr) < 8:
                break
            length = struct.unpack(">I", hdr[:4])[0]
            ct = hdr[4:8]
            data = f.read(length)
            f.read(4)
            if ct == b"IHDR":
                w, h = struct.unpack(">II", data[:8])
                color_type = data[9]
            elif ct == b"IDAT":
                idat.append(data)
            elif ct == b"IEND":
                break
    raw = zlib.decompress(b"".join(idat))
    bpp = 4 if color_type == 6 else 3
    stride = 1 + w * bpp
    pixels = []
    prev = bytes(w * bpp)
    for y in range(h):
        rs = y * stride
        ft = raw[rs]
        row = bytearray(raw[rs + 1:rs + stride])
        for i in range(len(row)):
            a = row[i - bpp] if i >= bpp else 0
            b = prev[i]
            c = prev[i - bpp] if i >= bpp else 0
            if ft == 1: row[i] = (row[i] + a) & 0xFF
            elif ft == 2: row[i] = (row[i] + b) & 0xFF
            elif ft == 3: row[i] = (row[i] + (a + b) // 2) & 0xFF
            elif ft == 4:
                p = a + b - c
                pa, pb, pc = abs(p - a), abs(p - b), abs(p - c)
                pr = a if (pa <= pb and pa <= pc) else (b if pb <= pc else c)
                row[i] = (row[i] + pr) & 0xFF
        prev = bytes(row)
        for x in range(w):
            off = x * bpp
            r, g, bl = row[off], row[off+1], row[off+2]
            al = row[off+3] if bpp == 4 else 255
            pixels.append((r, g, bl, al))
    return w, h, pixels


def write_png(path, width, height, pixels):
    def chunk(ct, data):
        c = ct + data
        return struct.pack(">I", len(data)) + c + struct.pack(">I", zlib.crc32(c) & 0xFFFFFFFF)
    raw = b""
    for y in range(height):
        raw += b"\x00"
        for x in range(width):
            r, g, b, a = pixels[y * width + x]
            raw += struct.pack("BBBB", r, g, b, a)
    with open(path, "wb") as f:
        f.write(b"\x89PNG\r\n\x1a\n")
        f.write(chunk(b"IHDR", struct.pack(">IIBBBBB", width, height, 8, 6, 0, 0, 0)))
        f.write(chunk(b"IDAT", zlib.compress(raw)))
        f.write(chunk(b"IEND", b""))


def scale_box(pixels, sw, sh, dx, dy, dw, dh, tw, th):
    out = []
    for y in range(th):
        sy = dy + y * dh // th
        for x in range(tw):
            sx = dx + x * dw // tw
            out.append(pixels[sy * sw + sx])
    return out


def build_char_prompt(char_data, direction_desc):
    app = char_data.get("appearance", {})
    parts = [STYLE.format(direction=direction_desc)]
    summary = app.get("summary", "")
    if summary:
        sentences = summary.split(". ")
        parts.append(". ".join(sentences[:2]))
    build = app.get("build", "")
    if build:
        parts.append(build)
    outfit = app.get("outfit", "")
    if outfit:
        parts.append(outfit)
    hair = app.get("hair", "")
    if hair:
        parts.append(hair + " hair")
    return ", ".join(parts)


# GBA palette and packing (from import_sprite.py)
PAL5 = [
    (0,0,0),(31,31,31),(12,12,15),(22,22,22),(28,20,14),(20,15,8),
    (28,4,4),(4,4,28),(15,15,0),(28,28,0),(0,18,0),(28,0,28),
    (0,28,0),(18,0,0),(0,8,18),(15,15,15),
]
PAL8 = [(r*255//31, g*255//31, b*255//31) for r, g, b in PAL5]

def nearest_pal(r, g, b, a):
    if a < 128:
        return 0
    best, bestd = 0, 999999
    for i in range(1, 16):
        pr, pg, pb = PAL8[i]
        d = (r-pr)**2 + (g-pg)**2 + (b-pb)**2
        if d < bestd:
            bestd = d
            best = i
    return best

def pack_4bpp(indices16x16):
    words = []
    for ty in range(2):
        for tx in range(2):
            for row in range(8):
                w = 0
                for px in range(8):
                    idx = indices16x16[(ty*8+row)*16 + (tx*8+px)]
                    w |= (idx & 0xF) << (px * 4)
                words.append(w)
    return words


def process_character(char_data, seed):
    cid = char_data["id"]
    name = char_data.get("name", cid)
    print("\n=== %s ===" % name)

    os.makedirs(SPRITE_DIR, exist_ok=True)
    os.makedirs(OUT_DIR, exist_ok=True)

    direction_images = {}  # dir_name -> (w, h, pixels)

    for dir_name, dir_desc in DIRECTIONS:
        png_path = os.path.join(SPRITE_DIR, "%s_%s.png" % (cid, dir_name))

        if dir_name == "front" and os.path.exists(png_path):
            print("  %s: using existing %s" % (dir_name, png_path))
        else:
            prompt = build_char_prompt(char_data, dir_desc)
            neg = NEGATIVE
            if dir_name != "front":
                neg += ", front facing, looking at camera"
            prefix = "%s_%s" % (cid, dir_name)
            print("  %s: generating..." % dir_name)
            fname = comfyui_generate(prompt, neg, seed, prefix)
            if not fname:
                print("  %s: FAILED" % dir_name)
                return False
            download_comfyui_image(fname, png_path)
            print("  %s: saved %s" % (dir_name, png_path))

        w, h, px = read_png(png_path)
        direction_images[dir_name] = (w, h, px)

    # Assemble 4x2 sprite sheet: columns = down,up,left,right; rows = frame0,frame1
    # Each cell is 16x16 in the output; we scale from the center crop of each 512x512
    print("  Assembling sprite sheet...")
    sheet_w, sheet_h = 4 * 16, 2 * 16
    sheet = [(0, 0, 0, 0)] * (sheet_w * sheet_h)

    dir_order = ["front", "back", "left", "right"]
    for col, dn in enumerate(dir_order):
        sw, sh, spx = direction_images[dn]
        # Center crop to square, then scale to 16x16
        if sw > sh:
            crop_x, crop_y, crop_sz = (sw - sh) // 2, 0, sh
        else:
            crop_x, crop_y, crop_sz = 0, (sh - sw) // 2, sw
        cell = scale_box(spx, sw, sh, crop_x, crop_y, crop_sz, crop_sz, 16, 16)
        # Place in both rows (same frame for both walk frames)
        for row in range(2):
            for y in range(16):
                for x in range(16):
                    px_idx = y * 16 + x
                    sheet_idx = (row * 16 + y) * sheet_w + (col * 16 + x)
                    sheet[sheet_idx] = cell[px_idx]

    sheet_path = os.path.join(OUT_DIR, "%s_sheet.png" % cid)
    write_png(sheet_path, sheet_w, sheet_h, sheet)
    print("  Sheet: %s" % sheet_path)

    # Convert to GBA 4bpp
    all_words = []
    for col in range(4):  # directions
        for row in range(2):  # frames
            indices = []
            for y in range(16):
                for x in range(16):
                    r, g, b, a = sheet[(row*16+y) * sheet_w + (col*16+x)]
                    indices.append(nearest_pal(r, g, b, a))
            all_words.extend(pack_4bpp(indices))

    assert len(all_words) == 256

    # Write C files
    c_name = "spr_%s" % cid
    h_path = os.path.join(OUT_DIR, "%s.h" % c_name)
    c_path = os.path.join(OUT_DIR, "%s.c" % c_name)

    with open(h_path, "w") as f:
        guard = "GEN_SPR_%s_H" % cid.upper()
        f.write("#ifndef %s\n#define %s\n" % (guard, guard))
        f.write('#include "gba.h"\n')
        f.write("extern const u32 %s[256];\n" % c_name)
        f.write("#endif\n")

    with open(c_path, "w") as f:
        f.write('#include "%s.h"\n\n' % c_name)
        f.write("const u32 %s[256] = {\n" % c_name)
        for i in range(0, 256, 8):
            chunk = all_words[i:i+8]
            f.write("    " + ",".join("0x%08X" % w for w in chunk) + ",\n")
        f.write("};\n")

    print("  GBA data: %s, %s" % (c_path, h_path))
    return True


def main():
    seed = SEED
    char_filter = None

    i = 1
    while i < len(sys.argv):
        if sys.argv[i] == "--seed" and i + 1 < len(sys.argv):
            seed = int(sys.argv[i+1]); i += 2
        elif sys.argv[i] == "--characters" and i + 1 < len(sys.argv):
            char_filter = sys.argv[i+1].split(","); i += 2
        else:
            i += 1

    # Check ComfyUI
    try:
        urllib.request.urlopen(URL + "/system_stats")
    except urllib.error.URLError:
        print("ERROR: ComfyUI not running at %s" % URL, file=sys.stderr)
        sys.exit(1)

    char_dir = "levels/characters"
    for fname in sorted(os.listdir(char_dir)):
        if not fname.endswith(".json"):
            continue
        path = os.path.join(char_dir, fname)
        with open(path) as f:
            char_data = json.load(f)
        cid = char_data.get("id", "")
        if char_filter and cid not in char_filter:
            continue
        process_character(char_data, seed)

    print("\n=== All done! Generated sprites in %s ===" % OUT_DIR)


if __name__ == "__main__":
    main()
