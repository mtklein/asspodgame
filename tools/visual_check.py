#!/usr/bin/env python3
"""visual_check.py -- Pixel-level assertions on headless screenshots.

Usage: visual_check.py <screenshot_dir>

Each check reads a PNG and verifies specific pixel properties.
Exits 0 if all pass, 1 if any fail.
"""

import struct, sys, zlib, os


def read_png(path):
    with open(path, "rb") as f:
        sig = f.read(8); w = h = 0; idat = []; color_type = 6
        while True:
            hdr = f.read(8)
            if len(hdr) < 8: break
            length = struct.unpack(">I", hdr[:4])[0]; ct = hdr[4:8]
            data = f.read(length); f.read(4)
            if ct == b"IHDR": w, h = struct.unpack(">II", data[:8]); color_type = data[9]
            elif ct == b"IDAT": idat.append(data)
            elif ct == b"IEND": break
    raw = zlib.decompress(b"".join(idat)); bpp = 4 if color_type == 6 else 3
    stride = 1 + w * bpp; pixels = []; prev = bytes(w * bpp)
    for y in range(h):
        rs = y * stride; ft = raw[rs]; row = bytearray(raw[rs + 1:rs + stride])
        for i in range(len(row)):
            a = row[i - bpp] if i >= bpp else 0; b = prev[i]
            c = prev[i - bpp] if i >= bpp else 0
            if ft == 1: row[i] = (row[i] + a) & 0xFF
            elif ft == 2: row[i] = (row[i] + b) & 0xFF
            elif ft == 3: row[i] = (row[i] + (a + b) // 2) & 0xFF
            elif ft == 4:
                p = a + b - c; pa, pb, pc = abs(p - a), abs(p - b), abs(p - c)
                pr = a if (pa <= pb and pa <= pc) else (b if pb <= pc else c)
                row[i] = (row[i] + pr) & 0xFF
        prev = bytes(row)
        for x in range(w):
            off = x * bpp
            pixels.append((row[off], row[off+1], row[off+2],
                           row[off+3] if bpp == 4 else 255))
    return w, h, pixels


def is_black(r, g, b):
    return r < 10 and g < 10 and b < 10


def is_nonblack(r, g, b):
    return r > 15 or g > 15 or b > 15


def count_nonblack_in_rect(pixels, w, x0, y0, x1, y1):
    n = 0
    for y in range(y0, y1):
        for x in range(x0, x1):
            r, g, b, a = pixels[y * w + x]
            if is_nonblack(r, g, b):
                n += 1
    return n


def count_unique_colors_in_rect(pixels, w, x0, y0, x1, y1):
    colors = set()
    for y in range(y0, y1):
        for x in range(x0, x1):
            r, g, b, a = pixels[y * w + x]
            colors.add((r // 32, g // 32, b // 32))
    return len(colors)


def rect_has_blue(pixels, w, x0, y0, x1, y1):
    for y in range(y0, y1):
        for x in range(x0, x1):
            r, g, b, a = pixels[y * w + x]
            if b > r + 20 and b > g + 20 and b > 40:
                return True
    return False


passed = 0
failed = 0


def check(name, condition, detail=""):
    global passed, failed
    if condition:
        passed += 1
        print("  PASS: %s" % name)
    else:
        failed += 1
        print("  FAIL: %s %s" % (name, ("(%s)" % detail) if detail else ""))


def run_checks(d):
    # --- 01_hq.png: HQ overview ---
    path = os.path.join(d, "01_hq.png")
    if os.path.exists(path):
        print("01_hq.png:")
        w, h, px = read_png(path)
        check("screen is 240x160", w == 240 and h == 160)

        # Sprites should be visible (non-black pixels in the play area)
        play_pixels = count_nonblack_in_rect(px, w, 0, 0, 240, 128)
        check("play area has content", play_pixels > 5000,
              "%d non-black pixels" % play_pixels)

        # Multiple distinct colors = map tiles + sprites rendering
        colors = count_unique_colors_in_rect(px, w, 0, 0, 240, 128)
        check("play area has color variety", colors > 5,
              "%d unique color groups" % colors)

        # Bottom 32px should be mostly black (below map)
        bottom_pixels = count_nonblack_in_rect(px, w, 0, 128, 240, 150)
        check("below map is dark", bottom_pixels < 500,
              "%d non-black pixels" % bottom_pixels)

        # Debug display at bottom-left should have some content
        debug_pixels = count_nonblack_in_rect(px, w, 0, 148, 100, 160)
        check("debug display visible", debug_pixels > 10,
              "%d pixels" % debug_pixels)

    # --- 02_indicator.png: (A) indicator visible ---
    path = os.path.join(d, "02_indicator.png")
    if os.path.exists(path):
        print("02_indicator.png:")
        w, h, px = read_png(path)
        # The indicator is a yellow 8x8 sprite above an NPC
        # Rubik is near (72,32) with camera at ~(0,0)
        # Indicator at roughly (76, 22) on screen
        # Look for yellow pixels (high R+G, low B) in the area above Rubik
        yellow_count = 0
        for y in range(15, 35):
            for x in range(65, 90):
                r, g, b, a = px[y * w + x]
                if r > 100 and g > 100 and b < 80:
                    yellow_count += 1
        check("(A) indicator has yellow pixels", yellow_count > 3,
              "%d yellow pixels" % yellow_count)

    # --- 03_dialogue.png: Dialogue box ---
    path = os.path.join(d, "03_dialogue.png")
    if os.path.exists(path):
        print("03_dialogue.png:")
        w, h, px = read_png(path)

        # Dialogue box occupies roughly y=120-160, x=8-232
        # It should have blue pixels (the FF gradient)
        check("dialogue box has blue fill",
              rect_has_blue(px, w, 10, 120, 230, 158))

        # Text area should NOT be transparent (no map tiles visible)
        # The text area is roughly y=128-152, x=16-224
        # Check that the area is predominantly blue, not map colors
        blue_count = 0
        total = 0
        for y in range(128, 152):
            for x in range(16, 224):
                r, g, b, a = px[y * w + x]
                total += 1
                if b > r and b > g:
                    blue_count += 1
        pct = blue_count * 100 // total if total else 0
        check("dialogue text area is opaque (mostly blue)", pct > 50,
              "%d%% blue" % pct)

        # Portrait should be visible (non-black 32x32 area at left side)
        # Portrait at roughly x=8, y=80
        portrait_pixels = count_nonblack_in_rect(px, w, 0, 70, 44, 110)
        check("portrait sprite visible", portrait_pixels > 100,
              "%d non-black pixels" % portrait_pixels)

        # White text should be present in the box
        white_count = 0
        for y in range(126, 155):
            for x in range(16, 224):
                r, g, b, a = px[y * w + x]
                if r > 200 and g > 200 and b > 200:
                    white_count += 1
        check("white text in dialogue box", white_count > 20,
              "%d white pixels" % white_count)

    # --- 04_scroll_south.png: Scrolled view ---
    path = os.path.join(d, "04_scroll_south.png")
    if os.path.exists(path):
        print("04_scroll_south.png:")
        w, h, px = read_png(path)
        # After scrolling south, should see the hallway area
        # Play area should still have content
        play_pixels = count_nonblack_in_rect(px, w, 0, 0, 240, 128)
        check("scrolled view has content", play_pixels > 3000,
              "%d non-black pixels" % play_pixels)

        # At least one sprite should be visible (the player)
        colors = count_unique_colors_in_rect(px, w, 100, 50, 150, 100)
        check("player sprite area has color", colors > 3,
              "%d color groups" % colors)


def run_golden_checks(d, golden_dir):
    """Compare each screenshot against its golden reference."""
    if not os.path.isdir(golden_dir):
        print("(no golden dir at %s, skipping golden checks)" % golden_dir)
        return

    for fname in sorted(os.listdir(golden_dir)):
        if not fname.endswith(".png"):
            continue
        golden_path = os.path.join(golden_dir, fname)
        test_path = os.path.join(d, fname)
        if not os.path.exists(test_path):
            check("golden %s exists" % fname, False, "test screenshot missing")
            continue

        gw, gh, gpx = read_png(golden_path)
        tw, th, tpx = read_png(test_path)

        if gw != tw or gh != th:
            check("golden %s size match" % fname, False,
                  "golden=%dx%d test=%dx%d" % (gw, gh, tw, th))
            continue

        diff_pixels = 0
        for i in range(gw * gh):
            gr, gg, gb, _ = gpx[i]
            tr, tg, tb, _ = tpx[i]
            if abs(gr - tr) + abs(gg - tg) + abs(gb - tb) > 10:
                diff_pixels += 1

        pct = diff_pixels * 100 // (gw * gh)
        check("golden %s matches" % fname, pct == 0,
              "%d pixels differ (%d%%)" % (diff_pixels, pct))


def main():
    d = sys.argv[1] if len(sys.argv) > 1 else "out/visual_tests"
    golden = sys.argv[2] if len(sys.argv) > 2 else "tests/golden"
    if not os.path.isdir(d):
        print("ERROR: %s not found" % d, file=sys.stderr)
        sys.exit(1)

    run_checks(d)
    run_golden_checks(d, golden)

    print()
    print("Visual checks: %d passed, %d failed" % (passed, failed))
    sys.exit(0 if failed == 0 else 1)


if __name__ == "__main__":
    main()
