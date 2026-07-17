#!/usr/bin/env python3
"""Build the lossless PR HUD atlas with OoT's HD item cells embedded."""

from pathlib import Path

from PIL import Image, ImageDraw


ROOT = Path(__file__).resolve().parents[1]
PR_ATLAS = Path("/tmp/project-restoration-hd-hud-official.png")
ITEM_ATLAS = ROOT / "derived-assets/oot3d-hd-pack/Items/tex1_512x512_AAC267B3D165E6FF_4_mip0.png"
OUTPUT = ROOT / "derived-assets/project-restoration-oot-items-clean.png"


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


def main() -> None:
    hud = Image.open(PR_ATLAS).convert("RGBA")
    items = remove_detached_item_artifacts(
        Image.open(ITEM_ATLAS).convert("RGBA"))
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

    # Solid native-renderer swatches in a confirmed transparent PR region.
    # Sampling their centers lets the GPU draw a perfectly crisp, scalable
    # magic-meter frame without packing low-resolution pixels into code. Keep
    # the separate transparent sample around (1644,48) untouched.
    draw = ImageDraw.Draw(hud)
    draw.rectangle((1456, 36, 1468, 48), fill=(0, 0, 0, 255))
    draw.rectangle((1476, 36, 1488, 48), fill=(235, 235, 220, 255))
    draw.rectangle((1496, 36, 1508, 48), fill=(72, 76, 76, 255))
    draw.rectangle((1516, 36, 1528, 48), fill=(30, 220, 55, 255))
    OUTPUT.parent.mkdir(parents=True, exist_ok=True)
    hud.save(OUTPUT, format="PNG", optimize=False)
    print(OUTPUT)


if __name__ == "__main__":
    main()
