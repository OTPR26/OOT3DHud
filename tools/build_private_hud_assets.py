#!/usr/bin/env python3
"""Build the legacy framebuffer prototype header from a Project Restoration atlas.

The generated header is intentionally ignored by git. It must only be built
from assets the user has supplied from their own installation. This tool
downscales and packs art as ARGB2222, so it must not be used for the final
native HD HUD path.
"""

from __future__ import annotations

import argparse
from pathlib import Path

from PIL import Image, ImageEnhance


def scaled_box(box: tuple[int, int, int, int], scale: int) -> tuple[int, int, int, int]:
    return tuple(value * scale for value in box)


def crop_resize(atlas: Image.Image, box: tuple[int, int, int, int], size: tuple[int, int],
                scale: int) -> Image.Image:
    return atlas.crop(scaled_box(box, scale)).resize(size, Image.Resampling.LANCZOS)


def blank_button(atlas: Image.Image, scale: int) -> Image.Image:
    # The circle is vertically symmetric. Mirroring its clean lower arc over
    # the colored A/B glyph produces the unlabeled X/Y base without painting
    # or redistributing a new texture.
    source = atlas.crop(scaled_box((9, 0, 46, 38), scale))
    for y in range(9 * scale):
        source.paste(source.crop((0, 38 * scale - y - 1, 37 * scale, 38 * scale - y)), (0, y))
    return source.resize((20, 20), Image.Resampling.LANCZOS)


def tint_magic(image: Image.Image) -> Image.Image:
    rgba = image.convert("RGBA")
    pixels = []
    for r, g, b, a in rgba.getdata():
        light = (r * 3 + g * 6 + b) // 10
        pixels.append((light // 5, min(255, light + 55), light // 3, a))
    rgba.putdata(pixels)
    return rgba


def pack_rgba2222(image: Image.Image) -> list[int]:
    return [((a >> 6) << 6) | ((r >> 6) << 4) | ((g >> 6) << 2) | (b >> 6)
            for r, g, b, a in image.convert("RGBA").getdata()]


def emit_array(name: str, image: Image.Image) -> str:
    values = pack_rgba2222(image)
    lines = [f"#define {name}_WIDTH {image.width}", f"#define {name}_HEIGHT {image.height}",
             f"static const u8 {name}[{len(values)}] = {{"]
    for offset in range(0, len(values), 16):
        lines.append("    " + ", ".join(f"0x{value:02X}" for value in values[offset:offset + 16]) + ",")
    lines.append("};")
    return "\n".join(lines)


def emit_item_atlas(atlas_path: Path) -> str:
    atlas = Image.open(atlas_path).convert("RGBA")
    if atlas.width != atlas.height or atlas.width % 16:
        raise ValueError(f"expected a square 16-column OoT3D item atlas, got {atlas.size}")

    cell = atlas.width // 16
    icon_width = 14
    icon_height = 14
    item_count = 0x3B  # Equippable items through the three combined bow/arrow icons.
    values: list[int] = []
    for item in range(item_count):
        x = (item % 16) * cell
        y = (item // 16) * cell
        icon = atlas.crop((x, y, x + cell, y + cell)).resize(
            (icon_width, icon_height), Image.Resampling.LANCZOS)
        values.extend(pack_rgba2222(icon))

    lines = ["#define HUD_HAS_OOT_ITEM_ASSETS 1",
             f"#define OOT_ITEM_ICON_WIDTH {icon_width}",
             f"#define OOT_ITEM_ICON_HEIGHT {icon_height}",
             f"#define OOT_ITEM_ICON_COUNT {item_count}",
             f"static const u8 OOT_ITEM_ICONS[{len(values)}] = {{"]
    for offset in range(0, len(values), 16):
        lines.append("    " + ", ".join(f"0x{value:02X}" for value in values[offset:offset + 16]) + ",")
    lines.append("};")
    return "\n".join(lines)


def build(atlas_path: Path, output: Path, test_level: int, oot_items: Path | None) -> None:
    atlas = Image.open(atlas_path).convert("RGBA")
    if atlas.width % 512 or atlas.height != atlas.width // 2:
        raise ValueError(f"expected a 512x256 Project Restoration HUD atlas or integer upscale, got {atlas.size}")
    scale = atlas.width // 512

    sprites = {
        "PR_BUTTON_A": crop_resize(atlas, (9, 0, 46, 38), (20, 20), scale),
        "PR_BUTTON_B": crop_resize(atlas, (48, 0, 85, 38), (20, 20), scale),
        "PR_BUTTON_BLANK": blank_button(atlas, scale),
        "PR_DPAD": crop_resize(atlas, (40, 39, 67, 68), (22, 24), scale),
        "PR_HEART_FULL": crop_resize(atlas, (480, 0, 496, 16), (11, 11), scale),
        "PR_HEART_EMPTY": crop_resize(atlas, (496, 64, 512, 80), (11, 11), scale),
        "PR_RUPEE_GREEN": crop_resize(atlas, (456, 60, 474, 78), (10, 12), scale),
        "PR_MAGIC_BAR": tint_magic(crop_resize(atlas, (90, 56, 154, 64), (96, 5), scale)),
        "PR_LABEL_I": crop_resize(atlas, (483, 81, 494, 96), (7, 10), scale),
        "PR_LABEL_II": crop_resize(atlas, (497, 81, 512, 96), (11, 10), scale),
        "PR_NAVI": crop_resize(atlas, (88, 0, 116, 26), (14, 14), scale),
    }

    output.parent.mkdir(parents=True, exist_ok=True)
    parts = ["#pragma once", "", f"// Generated from a {atlas.width}x{atlas.height} Project Restoration atlas.",
             "#define HUD_HAS_PROJECT_RESTORATION_ASSETS 1",
             f"#define HUD_PROJECT_RESTORATION_TEST_LEVEL {test_level}", ""]
    parts.extend(emit_array(name, image) + "\n" for name, image in sprites.items())
    if oot_items:
        parts.append(emit_item_atlas(oot_items) + "\n")
    output.write_text("\n".join(parts), encoding="utf-8")
    print(f"Generated {output} ({output.stat().st_size} bytes)")


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("atlas", type=Path)
    parser.add_argument("output", type=Path)
    parser.add_argument("--level", type=int, choices=range(1, 5), default=4)
    parser.add_argument("--oot-items", type=Path,
                        help="private OoT3D HD item atlas used for live equipped-item art")
    args = parser.parse_args()
    build(args.atlas, args.output, args.level, args.oot_items)


if __name__ == "__main__":
    main()
