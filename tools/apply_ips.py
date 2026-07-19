#!/usr/bin/env python3
"""Apply a standard IPS patch to a binary file."""

from pathlib import Path
import struct
import sys


def apply_ips(source: bytes, patch: bytes) -> bytes:
    if not patch.startswith(b"PATCH"):
        raise ValueError("not an IPS patch")

    output = bytearray(source)
    offset = 5
    while patch[offset : offset + 3] != b"EOF":
        if offset + 5 > len(patch):
            raise ValueError("truncated IPS record")

        target = int.from_bytes(patch[offset : offset + 3], "big")
        size = struct.unpack_from(">H", patch, offset + 3)[0]
        offset += 5

        if size == 0:
            if offset + 3 > len(patch):
                raise ValueError("truncated IPS RLE record")
            size = struct.unpack_from(">H", patch, offset)[0]
            data = patch[offset + 2 : offset + 3] * size
            offset += 3
        else:
            data = patch[offset : offset + size]
            if len(data) != size:
                raise ValueError("truncated IPS data")
            offset += size

        end = target + len(data)
        if end > len(output):
            output.extend(b"\0" * (end - len(output)))
        output[target:end] = data

    offset += 3
    if len(patch) - offset == 3:
        output = output[: int.from_bytes(patch[offset : offset + 3], "big")]
    elif len(patch) != offset:
        raise ValueError("unexpected data after IPS EOF marker")

    return bytes(output)


def main() -> None:
    if len(sys.argv) != 4:
        raise SystemExit(f"usage: {sys.argv[0]} BASE IPS OUTPUT")

    base_path, patch_path, output_path = map(Path, sys.argv[1:])
    output_path.write_bytes(apply_ips(base_path.read_bytes(), patch_path.read_bytes()))


if __name__ == "__main__":
    main()
