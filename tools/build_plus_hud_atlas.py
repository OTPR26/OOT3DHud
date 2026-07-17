#!/usr/bin/env python3
"""Merge the minimal PR HUD cells into Azahar Plus's working 4K UI atlas."""

from pathlib import Path

from PIL import Image


ROOT = Path(__file__).resolve().parents[1]
PLUS_UI = Path("/tmp/plus-ui-D352.png")
PR_ATLAS = Path("/tmp/project-restoration-hd-hud-official.png")
ITEMS = ROOT / "derived-assets/oot3d-hd-pack/Items/tex1_512x512_AAC267B3D165E6FF_4_mip0.png"
OUTPUT = ROOT / "derived-assets/azahar-plus-4k-modern-hud.png"


def main() -> None:
    plus = Image.open(PLUS_UI).convert("RGBA")
    pr = Image.open(PR_ATLAS).convert("RGBA")
    items = Image.open(ITEMS).convert("RGBA")
    if plus.size != (1024, 512):
        raise ValueError(f"unexpected Plus UI atlas: {plus.size}")
    if pr.size != (2048, 1024) or items.size != (512, 512):
        raise ValueError("unexpected PR or OoT item atlas dimensions")

    # Keep every Plus UI element at its existing normalized coordinate. The
    # output doubles the pack's 4x atlas to an 8x replacement without reducing
    # any source asset, then substitutes only cells sampled by our 13 quads.
    output = plus.resize((2048, 1024), Image.Resampling.LANCZOS)
    for box in (
        (192, 0, 340, 152),      # PR blank face-button base
        (160, 156, 268, 272),    # PR D-pad cross
        (352, 0, 464, 104),      # PR Navi marker
    ):
        output.alpha_composite(pr.crop(box), (box[0], box[1]))

    # The runtime UV mapper expects the untouched 512x512 OoT item sheet here.
    output.paste((0, 0, 0, 0), (512, 512, 1024, 1024))
    output.alpha_composite(items, (512, 512))
    OUTPUT.parent.mkdir(parents=True, exist_ok=True)
    output.save(OUTPUT, format="PNG", optimize=False)
    print(OUTPUT)


if __name__ == "__main__":
    main()
