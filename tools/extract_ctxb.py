#!/usr/bin/env python3
"""Convert Grezzo CTXB textures to PNG for local HUD development.

The CTXB layout and ETC1 decoding logic are adapted from Scarlet (MIT):
Copyright (c) 2016 xdaniel (Daniel R.) / DigitalZero Domain.
Run this with a Python environment that provides Pillow.
"""

from __future__ import annotations

import argparse
import struct
from pathlib import Path

from PIL import Image


MODIFIERS = (
    (2, 8, -2, -8),
    (5, 17, -5, -17),
    (9, 29, -9, -29),
    (13, 42, -13, -42),
    (18, 60, -18, -60),
    (24, 80, -24, -80),
    (33, 106, -33, -106),
    (47, 183, -47, -183),
)


def clamp(value: int) -> int:
    return max(0, min(255, value))


def decode_etc1_block(block: int) -> list[tuple[int, int, int]]:
    table1 = (block >> 37) & 7
    table2 = (block >> 34) & 7
    differential = (block >> 33) & 1
    flip = (block >> 32) & 1

    if not differential:
        r1n, r2n = (block >> 60) & 15, (block >> 56) & 15
        g1n, g2n = (block >> 52) & 15, (block >> 48) & 15
        b1n, b2n = (block >> 44) & 15, (block >> 40) & 15
        r1, r2 = r1n * 17, r2n * 17
        g1, g2 = g1n * 17, g2n * 17
        b1, b2 = b1n * 17, b2n * 17
    else:
        def signed3(value: int) -> int:
            return value - 8 if value >= 4 else value

        r1n, g1n, b1n = (block >> 59) & 31, (block >> 51) & 31, (block >> 43) & 31
        r2n = r1n + signed3((block >> 56) & 7)
        g2n = g1n + signed3((block >> 48) & 7)
        b2n = b1n + signed3((block >> 40) & 7)
        r1, r2 = (r1n << 3) | (r1n >> 2), (r2n << 3) | (r2n >> 2)
        g1, g2 = (g1n << 3) | (g1n >> 2), (g2n << 3) | (g2n >> 2)
        b1, b2 = (b1n << 3) | (b1n >> 2), (b2n << 3) | (b2n >> 2)

    pixels: list[tuple[int, int, int]] = []
    for py in range(4):
        for px in range(4):
            bit = px * 4 + py
            index = ((block >> bit) & 1) | (((block >> (bit + 16)) & 1) << 1)
            first = (flip and py < 2) or (not flip and px < 2)
            base = (r1, g1, b1) if first else (r2, g2, b2)
            modifier = MODIFIERS[table1 if first else table2][index]
            pixels.append(tuple(clamp(channel + modifier) for channel in base))
    return pixels


def decode_etc1(data: bytes, width: int, height: int, alpha4: bool) -> Image.Image:
    image = Image.new("RGBA", (width, height))
    cursor = 0
    for tile_y in range(0, height, 8):
        for tile_x in range(0, width, 8):
            for block_y in range(0, 8, 4):
                for block_x in range(0, 8, 4):
                    alpha = struct.unpack_from("<Q", data, cursor)[0] if alpha4 else (2**64 - 1)
                    cursor += 8 if alpha4 else 0
                    block = struct.unpack_from("<Q", data, cursor)[0]
                    cursor += 8
                    colors = decode_etc1_block(block)
                    for py in range(4):
                        for px in range(4):
                            x, y = tile_x + block_x + px, tile_y + block_y + py
                            if x >= width or y >= height:
                                continue
                            nibble = (alpha >> ((px * 4 + py) * 4)) & 15
                            image.putpixel((x, y), (*colors[py * 4 + px], nibble * 17))
    return image


def morton8(x: int, y: int) -> int:
    """Return the PICA200 8x8 tiled pixel index."""
    return (
        (x & 1)
        | ((y & 1) << 1)
        | ((x & 2) << 1)
        | ((y & 2) << 2)
        | ((x & 4) << 2)
        | ((y & 4) << 3)
    )


def decode_rgba4(data: bytes, width: int, height: int) -> Image.Image:
    image = Image.new("RGBA", (width, height))
    tiles_per_row = width // 8
    for y in range(height):
        for x in range(width):
            tile = (y // 8) * tiles_per_row + (x // 8)
            offset = (tile * 64 + morton8(x & 7, y & 7)) * 2
            value = struct.unpack_from("<H", data, offset)[0]
            image.putpixel(
                (x, y),
                tuple(((value >> shift) & 15) * 17 for shift in (0, 4, 8, 12)),
            )
    return image


def convert(source: Path, output: Path) -> None:
    data = source.read_bytes()
    if data[:4] != b"ctxb":
        raise ValueError(f"{source}: not a CTXB texture")
    tex_offset, data_offset = struct.unpack_from("<II", data, 16)
    if data[tex_offset : tex_offset + 4] != b"tex ":
        raise ValueError(f"{source}: missing TEX chunk")
    texture_count = struct.unpack_from("<I", data, tex_offset + 8)[0]
    output.mkdir(parents=True, exist_ok=True)

    for index in range(texture_count):
        entry = tex_offset + 12 + index * 0x24
        length, mipmaps, flags, width, height, pixel_format, data_type, relative = struct.unpack_from(
            "<IHHHHHHI", data, entry
        )
        name = data[entry + 20 : entry + 36].split(b"\0", 1)[0].decode("ascii") or f"texture_{index}"
        raw = data[data_offset + relative : data_offset + relative + length]
        print(
            f"{index:02d} {name:16s} {width}x{height} len={length} "
            f"mips={mipmaps} flags={flags:04x} format={pixel_format:04x}/{data_type:04x}"
        )
        if pixel_format not in (0x6752, 0x675A, 0x675B):
            print("   skipped: converter currently supports RGBA4, ETC1, and ETC1A4")
            continue
        if pixel_format == 0x6752:
            image = decode_rgba4(raw, width, height)
        else:
            image = decode_etc1(raw, width, height, pixel_format == 0x675B)
        image.save(output / f"{index:02d}-{name}.png")


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("source", type=Path)
    parser.add_argument("output", type=Path)
    args = parser.parse_args()
    convert(args.source, args.output)


if __name__ == "__main__":
    main()
