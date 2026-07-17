#!/usr/bin/env python3
"""Extract Grezzo GAR v2 archives used by Project Restoration.

Format details are based on Scarlet's MIT-licensed GARv2 reader:
Copyright (c) 2016 xdaniel (Daniel R.) / DigitalZero Domain.

This tool deliberately extracts only user-supplied archives. Do not commit
the resulting game assets; game-dumps/ is ignored by this repository.
"""

from __future__ import annotations

import argparse
import struct
from pathlib import Path


def u16(data: bytes, offset: int) -> int:
    return struct.unpack_from("<H", data, offset)[0]


def u32(data: bytes, offset: int) -> int:
    return struct.unpack_from("<I", data, offset)[0]


def cstring(data: bytes, offset: int) -> str:
    end = data.index(b"\0", offset)
    return data[offset:end].decode("utf-8")


def extract_gar(source: Path, output: Path) -> None:
    data = source.read_bytes()
    if data[:4] != b"GAR\x02":
        raise ValueError(f"{source}: not a GAR v2 archive")

    archive_size = u32(data, 4)
    num_files = u16(data, 10)
    file_info_offset = u32(data, 16)
    file_index_offset = u32(data, 20)
    if archive_size != len(data):
        raise ValueError(f"{source}: header size {archive_size} != {len(data)}")

    output.mkdir(parents=True, exist_ok=True)
    for index in range(num_files):
        info = file_info_offset + index * 12
        file_size = u32(data, info)
        full_path_offset = u32(data, info + 8)
        file_offset = u32(data, file_index_offset + index * 4)
        filename = Path(cstring(data, full_path_offset)).name
        payload = data[file_offset : file_offset + file_size]
        if len(payload) != file_size:
            raise ValueError(f"{source}: truncated entry {filename}")
        destination = output / filename
        destination.write_bytes(payload)
        print(f"{index:02d} {file_offset:08x} {file_size:8d} {destination}")


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("source", type=Path)
    parser.add_argument("output", type=Path)
    args = parser.parse_args()
    extract_gar(args.source, args.output)


if __name__ == "__main__":
    main()
