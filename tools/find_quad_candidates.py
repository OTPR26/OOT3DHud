#!/usr/bin/env python3
"""Find likely 4-vertex top-screen UI rectangles in an ARM code image."""

import argparse
import struct
from pathlib import Path


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("image", type=Path)
    parser.add_argument("--base", type=lambda value: int(value, 0), default=0x100000)
    args = parser.parse_args()
    data = args.image.read_bytes()

    for offset in range(0, len(data) - 48, 4):
        values = struct.unpack_from("<12f", data, offset)
        xs = values[0::3]
        ys = values[1::3]
        zs = values[2::3]
        if not all(abs(z) < 0.001 for z in zs):
            continue
        if not (0.0 <= min(xs) <= 420.0 and 0.0 <= max(xs) <= 420.0):
            continue
        if not (0.0 <= min(ys) <= 260.0 and 0.0 <= max(ys) <= 260.0):
            continue
        if len({round(x, 3) for x in xs}) != 2 or len({round(y, 3) for y in ys}) != 2:
            continue
        if max(xs) - min(xs) < 4.0 or max(ys) - min(ys) < 4.0:
            continue
        print(f"{args.base + offset:08x} x={min(xs):g}..{max(xs):g} y={min(ys):g}..{max(ys):g}")


if __name__ == "__main__":
    main()
