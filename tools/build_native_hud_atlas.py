#!/usr/bin/env python3
"""Build the lossless PR HUD atlas with OoT's HD item cells embedded."""

import argparse
from pathlib import Path

from PIL import Image, ImageDraw


HEART_PAIR_BANK_X = 1024
HEART_PAIR_BANK_Y = 512
HEART_PAIR_TILE_WIDTH = 128
HEART_PAIR_TILE_HEIGHT = 64
MAGIC_FRAME_X = 1024
MAGIC_FRAME_Y = 720
MAGIC_FRAME_WIDTH = 440
MAGIC_FRAME_HEIGHT = 36
MAGIC_FILL_X = 1024
MAGIC_FILL_Y = 800
MAGIC_FILL_WIDTH = 848
MAGIC_FILL_HEIGHT = 20
AMMO_BANK_X = 1024
AMMO_BANK_Y = 832
AMMO_TILE_WIDTH = 56
AMMO_TILE_HEIGHT = 64
AMMO_BANK_COLUMNS = 18


def remove_detached_item_artifacts(image: Image.Image) -> Image.Image:
    """Remove only tiny detached alpha islands from HUD sampling cells."""
    cleaned = image.copy()
    width, height = cleaned.size
    pixels = cleaned.load()
    cell_size = width / 12.0
    for cell_y in range(11):  # Item IDs used by the HUD end at 0x7A.
        for cell_x in range(12):
            # Match NativeHud_WriteItemUV exactly, including its proven
            # five-source-pixel horizontal alignment correction.
            x0 = max(0, round(cell_x * cell_size - 5.0))
            x1 = min(width, round((cell_x + 1) * cell_size - 5.0))
            y0 = max(0, round(cell_y * cell_size))
            y1 = min(height, round((cell_y + 1) * cell_size))
            seen: set[tuple[int, int]] = set()
            components: list[list[tuple[int, int]]] = []
            for y in range(y0, y1):
                for x in range(x0, x1):
                    if (x, y) in seen or pixels[x, y][3] == 0:
                        continue
                    stack = [(x, y)]
                    seen.add((x, y))
                    component: list[tuple[int, int]] = []
                    while stack:
                        px, py = stack.pop()
                        component.append((px, py))
                        for dy in (-1, 0, 1):
                            for dx in (-1, 0, 1):
                                nx, ny = px + dx, py + dy
                                if not (x0 <= nx < x1 and y0 <= ny < y1):
                                    continue
                                if (nx, ny) in seen or pixels[nx, ny][3] == 0:
                                    continue
                                seen.add((nx, ny))
                                stack.append((nx, ny))
                    components.append(component)
            if not components:
                continue
            largest = max(len(component) for component in components)
            threshold = max(8, int(largest * 0.20))
            for component in components:
                if len(component) < threshold:
                    for x, y in component:
                        pixels[x, y] = (0, 0, 0, 0)
            item_id = cell_y * 12 + cell_x
            if item_id in (7, 8):
                # The HD dump contains a small gray "15" counter baked into
                # the upper-left of both Ocarina sampling cells.
                for y in range(y0, min(y0 + 17, y1)):
                    for x in range(x0, min(x0 + 17, x1)):
                        pixels[x, y] = (0, 0, 0, 0)
            if item_id in (0x0A, 0x0B, 0x0F, 0x46):
                # Hookshot, Longshot, Lens of Truth, and Hover Boots sit safely
                # above their cell bottoms, but bilinear filtering samples a
                # thin fragment from the next atlas row. Clear only that shared
                # boundary so the artifact cannot follow the icon to another
                # HUD slot.
                boundary = round((cell_y + 1) * cell_size)
                for y in range(max(0, boundary - 1), min(height, boundary + 2)):
                    for x in range(x0, x1):
                        pixels[x, y] = (0, 0, 0, 0)
    # Bilinear sampling can still pull a one-pixel sliver from the neighboring
    # cell even after detached components are removed. Add a narrow transparent
    # gutter at the exact runtime column boundaries; item art is centered well
    # inside these edges.
    item_rows_bottom = min(height, round(11 * cell_size))
    for cell_x in range(1, 12):
        boundary = round(cell_x * cell_size - 5.0)
        for y in range(item_rows_bottom):
            for x in range(max(0, boundary - 1), min(width, boundary + 1)):
                pixels[x, y] = (0, 0, 0, 0)
    return cleaned


def add_heart_pair_bank(hud: Image.Image) -> None:
    """Add every sequential two-heart state used by the 20-heart HUD."""
    # PR's five heart states are 64x64 physical cells at logical x=480:
    # full, 3/4, 1/2, 1/4, and dark-empty. Reorder them into the runtime's
    # empty-to-full progression without resampling any source pixels.
    source_by_progress = [
        hud.crop((1920, 256, 1984, 320)),  # empty
        hud.crop((1920, 192, 1984, 256)),  # quarter
        hud.crop((1920, 128, 1984, 192)),  # half
        hud.crop((1920, 64, 1984, 128)),   # three-quarter
        hud.crop((1920, 0, 1984, 64)),     # full
    ]

    variants: list[tuple[int | None, int | None]] = [
        (None, None),  # no heart containers in this pair
        (0, None), (1, None), (2, None), (3, None), (4, None),
        (0, 0), (1, 0), (2, 0), (3, 0), (4, 0),
        (4, 1), (4, 2), (4, 3), (4, 4),
    ]
    bank_width = HEART_PAIR_TILE_WIDTH * 5
    bank_height = HEART_PAIR_TILE_HEIGHT * 3
    hud.paste((0, 0, 0, 0),
              (HEART_PAIR_BANK_X, HEART_PAIR_BANK_Y,
               HEART_PAIR_BANK_X + bank_width,
               HEART_PAIR_BANK_Y + bank_height))
    for index, (left_state, right_state) in enumerate(variants):
        x = HEART_PAIR_BANK_X + (index % 5) * HEART_PAIR_TILE_WIDTH
        y = HEART_PAIR_BANK_Y + (index // 5) * HEART_PAIR_TILE_HEIGHT
        if left_state is not None:
            hud.alpha_composite(source_by_progress[left_state], (x, y))
        if right_state is not None:
            hud.alpha_composite(source_by_progress[right_state], (x + 64, y))


def add_magic_meter_bank(hud: Image.Image) -> None:
    """Add fixed-geometry normal/double frames and a sliding live-fill strip."""
    draw = ImageDraw.Draw(hud)
    transparent = (0, 0, 0, 0)
    black = (0, 0, 0, 255)
    border = (235, 235, 220, 255)
    track = (72, 76, 76, 255)
    green = (30, 220, 55, 255)

    # Both frames occupy a 110x9 logical tile. The normal frame uses its
    # first 74 logical pixels and leaves the remainder transparent; the
    # double frame spans the same 110-pixel width as the heart rows.
    for row, outer_width in enumerate((74, 110)):
        x = MAGIC_FRAME_X
        y = MAGIC_FRAME_Y + row * MAGIC_FRAME_HEIGHT
        hud.paste(transparent, (x, y, x + MAGIC_FRAME_WIDTH,
                                y + MAGIC_FRAME_HEIGHT))
        draw.rectangle((x, y, x + outer_width * 4 - 1,
                        y + MAGIC_FRAME_HEIGHT - 1), fill=black)
        draw.rectangle((x + 4, y + 4, x + (outer_width - 1) * 4 - 1,
                        y + 32 - 1), fill=border)
        draw.rectangle((x + 8, y + 8, x + (outer_width - 2) * 4 - 1,
                        y + 28 - 1), fill=track)

    # A fixed 106-pixel fill quad samples a 212-pixel logical strip. Moving
    # its 106-pixel UV window right replaces green pixels with transparency,
    # so every live width can be represented without changing GPU geometry.
    hud.paste(transparent, (MAGIC_FILL_X, MAGIC_FILL_Y,
                            MAGIC_FILL_X + MAGIC_FILL_WIDTH,
                            MAGIC_FILL_Y + MAGIC_FILL_HEIGHT))
    draw.rectangle((MAGIC_FILL_X, MAGIC_FILL_Y,
                    MAGIC_FILL_X + MAGIC_FILL_WIDTH // 2 - 1,
                    MAGIC_FILL_Y + MAGIC_FILL_HEIGHT - 1), fill=green)


def add_ammo_count_bank(hud: Image.Image) -> None:
    """Add lossless live-count tiles for every attainable ammo value."""
    # PR's native 0-9 strip occupies 8x16 logical cells. Trim two transparent
    # source pixels from each side so two digits fit in a compact 56x64 tile;
    # no scaling, filtering, or format conversion is involved.
    digits = [hud.crop((digit * 32 + 2, 288,
                        digit * 32 + 30, 352))
              for digit in range(10)]
    hud.paste((0, 0, 0, 0),
              (AMMO_BANK_X, AMMO_BANK_Y,
               AMMO_BANK_X + AMMO_BANK_COLUMNS * AMMO_TILE_WIDTH,
               AMMO_BANK_Y + 3 * AMMO_TILE_HEIGHT))
    for amount in range(51):
        x = AMMO_BANK_X + (amount % AMMO_BANK_COLUMNS) * AMMO_TILE_WIDTH
        y = AMMO_BANK_Y + (amount // AMMO_BANK_COLUMNS) * AMMO_TILE_HEIGHT
        if amount >= 10:
            hud.alpha_composite(digits[amount // 10], (x, y))
        hud.alpha_composite(digits[amount % 10], (x + 28, y))


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("project_restoration_atlas", type=Path)
    parser.add_argument("item_atlas", type=Path)
    parser.add_argument("output", type=Path)
    args = parser.parse_args()

    hud = Image.open(args.project_restoration_atlas).convert("RGBA")
    items = remove_detached_item_artifacts(
        Image.open(args.item_atlas).convert("RGBA"))
    if hud.size != (2048, 1024):
        raise ValueError(f"unexpected PR atlas size: {hud.size}")
    if items.size != (512, 512):
        raise ValueError(f"unexpected item atlas size: {items.size}")

    # PR's D-pad crop contains a detached horizontal drop shadow in its final
    # few logical rows. It becomes conspicuous over bright sky, so clear only
    # that isolated strip while leaving the cross artwork untouched.
    hud.paste((0, 0, 0, 0), (160, 258, 268, 272))

    # This lower-middle region contains action-text assets not used by our HUD.
    # Clear it before compositing so PR artwork cannot show through transparent
    # pixels around the OoT item silhouettes. Keep both source images at their
    # original resolution; no resampling or lossy encoding is performed.
    hud.paste((0, 0, 0, 0), (512, 512, 1024, 1024))
    hud.alpha_composite(items, (512, 512))

    # Ten native pair quads can display all twenty live hearts while keeping
    # rupees and magic inside the board's proven low-index range.
    add_heart_pair_bank(hud)
    add_magic_meter_bank(hud)
    add_ammo_count_bank(hud)

    # Solid native-renderer swatches in a confirmed transparent PR region.
    # Sampling their centers lets the GPU draw a perfectly crisp, scalable
    # magic-meter frame without packing low-resolution pixels into code. Keep
    # the separate transparent sample around (1644,48) untouched.
    draw = ImageDraw.Draw(hud)
    draw.rectangle((1456, 36, 1468, 48), fill=(0, 0, 0, 255))
    draw.rectangle((1476, 36, 1488, 48), fill=(235, 235, 220, 255))
    draw.rectangle((1496, 36, 1508, 48), fill=(72, 76, 76, 255))
    draw.rectangle((1516, 36, 1528, 48), fill=(30, 220, 55, 255))
    # Dedicated transparent texel used to collapse inactive count quads.
    hud.paste((0, 0, 0, 0), (2036, 1012, 2048, 1024))
    args.output.parent.mkdir(parents=True, exist_ok=True)
    hud.save(args.output, format="PNG", optimize=False)
    print(args.output)


if __name__ == "__main__":
    main()
