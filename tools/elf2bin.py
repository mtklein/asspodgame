#!/usr/bin/env python3
"""Extract loadable segments from an ELF into a flat binary (like objcopy -O binary).

Uses LMA (load addresses), not VMA, so sections with 'AT >rom' in the
linker script are placed at their ROM offset — not at their runtime EWRAM
address.  This avoids the 96 MB gap that zig objcopy produces.

Usage: python3 elf2bin.py input.elf output.bin
"""
import struct, sys

def elf2bin(elf_path, bin_path):
    with open(elf_path, "rb") as f:
        data = f.read()

    # Parse ELF header (32-bit little-endian)
    if data[:4] != b"\x7fELF":
        raise ValueError("Not an ELF file")
    if data[4] != 1:
        raise ValueError("Not a 32-bit ELF")

    e_phoff = struct.unpack_from("<I", data, 28)[0]
    e_phentsize = struct.unpack_from("<H", data, 42)[0]
    e_phnum = struct.unpack_from("<H", data, 44)[0]

    # Collect PT_LOAD segments
    segments = []
    for i in range(e_phnum):
        off = e_phoff + i * e_phentsize
        p_type, p_offset, p_vaddr, p_paddr, p_filesz, p_memsz = struct.unpack_from(
            "<IIIIII", data, off
        )
        if p_type == 1 and p_filesz > 0:  # PT_LOAD with data
            seg_data = data[p_offset : p_offset + p_filesz]
            segments.append((p_paddr, seg_data))  # Use paddr (LMA)

    if not segments:
        raise ValueError("No loadable segments found")

    # Sort by physical address
    segments.sort(key=lambda s: s[0])

    # Find the base (lowest LMA) and build a flat image
    base = segments[0][0]
    end = max(addr + len(d) for addr, d in segments)
    image = bytearray(end - base)

    for addr, seg_data in segments:
        offset = addr - base
        image[offset : offset + len(seg_data)] = seg_data

    with open(bin_path, "wb") as f:
        f.write(image)

    print(f"elf2bin: {bin_path} — {len(image)} bytes (base 0x{base:08X})")

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print(f"Usage: {sys.argv[0]} input.elf output.bin", file=sys.stderr)
        sys.exit(1)
    elf2bin(sys.argv[1], sys.argv[2])
