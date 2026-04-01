#!/usr/bin/env python3
"""Convert PPM (P6) from stdin to PNG file."""
import struct, sys, zlib

data = sys.stdin.buffer.read()
# Parse P6 header
assert data[:2] == b'P6'
parts = data[2:].split(None, 3)  # width, height, maxval, pixel_data
w, h, maxval = int(parts[0]), int(parts[1]), int(parts[2])
pixels = parts[3]

def chunk(ct, d):
    c = ct + d
    return struct.pack('>I', len(d)) + c + struct.pack('>I', zlib.crc32(c) & 0xFFFFFFFF)

raw = b''
for y in range(h):
    raw += b'\x00'
    for x in range(w):
        off = (y * w + x) * 3
        raw += pixels[off:off+3] + b'\xff'

out = sys.argv[1] if len(sys.argv) > 1 else '/dev/stdout'
with open(out, 'wb') as f:
    f.write(b'\x89PNG\r\n\x1a\n')
    f.write(chunk(b'IHDR', struct.pack('>IIBBBBB', w, h, 8, 6, 0, 0, 0)))
    f.write(chunk(b'IDAT', zlib.compress(raw)))
    f.write(chunk(b'IEND', b''))
