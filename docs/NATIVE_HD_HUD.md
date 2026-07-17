# Native HD HUD

## Why the renderer is native

An early prototype wrote ARGB2222 sprites into OoT3D's fixed 400x240 framebuffer. Upscaling that
result could never preserve HD detail. The release instead uses OoT3D's GPU board renderer, allowing
Azahar Plus to substitute a 2048x1024 RGBA texture before rendering at the configured internal
resolution.

## Texture and board path

The implementation reuses the normally hidden stereoscopic motion-control board and points it at the
2:1 `cam_interface00` slot. For the tested USA build, Azahar Plus identifies the replacement as:

```text
tex1_256x128_F23CD5DE9DCE99C4_4_mip0.png
```

The board's external position, UV, and index buffers are retained. `native_hud.c` updates the live
quads after OoT3D's normal gameplay update. Native V coordinates use the opposite origin from PNG
pixel coordinates.

## Live elements

- A action prompt: the original changing action art is repositioned and resized.
- B/X/Y: reads current button items.
- D-pad Left/Down: reads the active I/II assignments; an unassigned slot is transparent.
- D-pad Up/Right: Navi/View and Ocarina.
- Hearts: reads health capacity and current health, including empty-heart state.
- Magic: reads acquired state, capacity, and current amount.
- Rupees: reads the current wallet value and draws native digits without a black panel.

All HUD state access is read-only.

## Current allocation limit

Only the low-index portion of the reused board proved reliable with the target game and emulator.
Twelve quads are reserved for hearts, with the remaining reliable quads assigned to rupees and magic.
The current release therefore displays at most 12 hearts, although OoT3D itself supports 20.

## Asset policy

- Keep custom texture art outside `code.ips`.
- Preserve full RGBA resolution; do not convert release art to framebuffer sprites.
- Keep private source art, game extracts, and full-resolution generated atlases out of the source
  snapshot. The small legacy generated header is retained only for reproducible link layout.
- Keep D-pad routing and the free camera independent from texture loading.

The atlas-building tools operate only on user-supplied, legally obtained source textures. The source
package does not contain those full-resolution inputs or the generated replacement atlas.

## Confirmed USA addresses

These addresses are for the legally extracted USA 1.0 executable with text base `0x00100000`.

| Address | Role |
| --- | --- |
| `0x00481194` | Loads the localized menu texture set |
| `0x00480E98` | Builds a native HUD board and registers it |
| `0x00348F34` | Constructs a GPU board from vertex/index buffers |
| `0x00348A64` | Assigns a texture and initial board coordinates |
| `0x0034897C` | Registers a board with the renderer |
| `0x002E11D0` | Retrieves a loaded menu texture by slot |
| `0x00469590` | Constructs the motion-control top-screen quad |
| `0x004FC648` | Motion-control quad visibility flag |
