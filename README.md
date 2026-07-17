# OoT3D Modern HUD + Free Camera

A presentation and controller-usability patch for the USA 1.0 release of *The Legend of Zelda:
Ocarina of Time 3D*.

The patch keeps the original gameplay rules while adding:

- a native, high-resolution top-screen HUD inspired by Project Restoration;
- ABXY in a modern diamond layout, including the game's live action prompt;
- live B/X/Y and D-pad item icons;
- D-pad access to touchscreen I, touchscreen II, Navi, and the Ocarina;
- live hearts, magic, and rupees on the top screen;
- the standalone C-stick free camera.

## Controls

| Input | Action |
| --- | --- |
| D-pad Left | Touchscreen item I |
| D-pad Down | Touchscreen item II |
| D-pad Up | Navi / View |
| D-pad Right | Ocarina |
| C-stick | Free camera |
| L + R + D-pad Up/Down | Free-camera sensitivity |
| L + R + D-pad Left/Right | Free-camera inversion |

Unassigned I or II slots remain blank. The L+R camera-settings chord takes priority over the gameplay
D-pad bindings.

## Compatibility and status

- Tested with USA OoT3D 1.0 in Azahar Plus.
- Tested alongside a 4K custom-texture pack.
- Free camera, all four D-pad actions, live item icons, action prompt, hearts, magic, and rupees have
  been validated in normal gameplay.
- EUR, JP, Master Quest, real 3DS hardware, and other game revisions are not yet validated.
- The current native-board allocation displays up to 12 hearts. Saves with more than 12 heart
  containers still work, but only the first 12 are drawn by this HUD.

The patch does not intentionally change combat, movement, progression, balance, inventory rules, or
save data.

## Building

Requirements:

- devkitARM / devkitPro with the 3DS rules installed;
- Python 3.

Build the tested USA/Azahar configuration:

```sh
./tools/build.sh
```

The output is `artifacts/USA/code.ips`. The wrapper stages the source in a temporary directory so the
inherited devkitARM Makefile works when the project path contains spaces. `REGION`, `CITRA`,
`DEVKITPRO`, and `DEVKITARM` may be overridden through environment variables.

Run the host-side input-routing tests without a 3DS toolchain:

```sh
make host-test
```

## Installing a private test build in Azahar Plus

For USA OoT3D (`0004000000033500`), the tested files use these paths beneath Azahar Plus's user data
directory:

```text
load/mods/0004000000033500/code.ips
load/mods/0004000000033500/exheader.bin
load/textures/0004000000033500/UI/tex1_256x128_F23CD5DE9DCE99C4_4_mip0.png
```

On the tested AYN setup, the user data directory is `/storage/emulated/0/ROMs/n3ds`.

The exheader and full-resolution adapted texture are deliberately excluded from the source package.
The generated legacy framebuffer header remains in the source snapshot because it affects the tested
binary's link layout, although the release renderer does not display those embedded sprites. Do not
distribute game dumps, extracted executables, or patched game binaries.

## Attribution and redistribution

The loader and free-camera base come from
[Roberto-Nessy/OoT3D_Standalone_Free_Cam](https://github.com/Roberto-Nessy/OoT3D_Standalone_Free_Cam).
OoT3D structures and hook conventions were cross-checked against
[gamestabled/OoT3D_Randomizer](https://github.com/gamestabled/OoT3D_Randomizer).
The HUD design and source art are inspired by Project Restoration's HD HUD.

This combined project is distributed under GPLv3, as selected for the OTPR26 repository. Preserve all
upstream attribution and file-level license notices. See [NOTICE.md](NOTICE.md) and
[RELEASE_CHECKLIST.md](RELEASE_CHECKLIST.md) for the third-party and game-asset boundaries.

Implementation details are in [docs/IMPLEMENTATION.md](docs/IMPLEMENTATION.md) and
[docs/NATIVE_HD_HUD.md](docs/NATIVE_HD_HUD.md).
