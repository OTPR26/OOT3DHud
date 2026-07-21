#!/usr/bin/env python3
"""Prepare a regional OoT3D exheader for the expanded Reframed code image.

The Chinese/Taiwanese and Korean releases use a different title-ID family and
different memory layout from the western/Japanese builds.  Their exheaders
must therefore be derived from the matching game, never from the USA file.
"""

from __future__ import annotations

import argparse
import struct
from pathlib import Path


TEXT_MAX_PAGES_OFFSET = 0x14
TEXT_SIZE_OFFSET = 0x18
BSS_SIZE_OFFSET = 0x3C
FLAGS_OFFSET = 0x0D
EXPANDED_BSS_SIZE = 0x19B000
ACI_KERNEL_CAPS_OFFSET = 0x370
SIGNED_ACI_KERNEL_CAPS_OFFSET = 0x770
IR_SERVICE_OFFSETS = (0x328, 0x728)
USA_SYSCALL_MASK_0 = bytes.fromhex("fefffbf0")
PROCESS_CONTROL_CAP = bytes.fromhex("000001f4")


def prepare(source: Path, destination: Path) -> None:
    data = bytearray(source.read_bytes())
    if len(data) != 0x800 or data[:8] != b"CtrApp\0\0":
        raise ValueError(
            f"{source}: expected a decrypted 0x800-byte CtrApp exheader"
        )

    text_max_pages = struct.unpack_from("<I", data, TEXT_MAX_PAGES_OFFSET)[0]
    struct.pack_into("<I", data, TEXT_SIZE_OFFSET, text_max_pages * 0x1000)
    struct.pack_into("<I", data, BSS_SIZE_OFFSET, EXPANDED_BSS_SIZE)
    data[FLAGS_OFFSET] |= 0x02

    # The regional games omit the process-control SVCs used by loader_main.
    # Add the same syscall mask and process-control descriptor as the working
    # western mod, while retaining the CJK-only static mapping descriptor.
    if data[ACI_KERNEL_CAPS_OFFSET:ACI_KERNEL_CAPS_OFFSET + 4] != bytes.fromhex(
        "4e9ffaf0"
    ):
        raise ValueError(f"{source}: unexpected regional syscall mask")
    data[ACI_KERNEL_CAPS_OFFSET:ACI_KERNEL_CAPS_OFFSET + 4] = USA_SYSCALL_MASK_0
    regional_caps = data[ACI_KERNEL_CAPS_OFFSET + 12:ACI_KERNEL_CAPS_OFFSET + 48]
    data[ACI_KERNEL_CAPS_OFFSET + 12:ACI_KERNEL_CAPS_OFFSET + 52] = (
        PROCESS_CONTROL_CAP + regional_caps
    )

    if data[
        SIGNED_ACI_KERNEL_CAPS_OFFSET:SIGNED_ACI_KERNEL_CAPS_OFFSET + 4
    ] != bytes.fromhex("4e9ffaf0"):
        raise ValueError(f"{source}: unexpected signed regional syscall mask")
    data[
        SIGNED_ACI_KERNEL_CAPS_OFFSET:SIGNED_ACI_KERNEL_CAPS_OFFSET + 4
    ] = USA_SYSCALL_MASK_0

    # Preserve ldr:ro in slot 26 and repurpose the original ir:USER slot for
    # the New 3DS IR service consumed by the standalone free-camera code.
    for offset in IR_SERVICE_OFFSETS:
        if data[offset:offset + 8] != b"ir:USER\0":
            raise ValueError(f"{source}: unexpected IR service slot")
        data[offset:offset + 8] = b"ir:rst\0\0"

    destination.parent.mkdir(parents=True, exist_ok=True)
    destination.write_bytes(data)
    print(f"Created {destination} from {source}")


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("region", choices=("TWN", "KOR"))
    parser.add_argument(
        "--source",
        required=True,
        type=Path,
        help="matching decrypted regional exheader (starts with CtrApp)",
    )
    parser.add_argument("--output", type=Path)
    args = parser.parse_args()

    root = Path(__file__).resolve().parent.parent
    output = args.output or root / "artifacts" / args.region / "exheader.bin"
    prepare(args.source, output)


if __name__ == "__main__":
    main()
