# OoT3D Modern HUD + Free Camera

[![Release](https://img.shields.io/github/v/release/OTPR26/OOT3DHud?display_name=tag)](https://github.com/OTPR26/OOT3DHud/releases/latest)
[![License](https://img.shields.io/github/license/OTPR26/OOT3DHud)](LICENSE)
![Game](https://img.shields.io/badge/OoT3D-USA%201.0-4b8bbe)
![Tested](https://img.shields.io/badge/tested-Azahar%20Plus-5aaa46)

A modern, high-resolution top-screen HUD and C-stick free camera for the USA 1.0 release of
*The Legend of Zelda: Ocarina of Time 3D*.

The HUD brings the Project Restoration style to OoT3D: ABXY in a diamond, live item assignments,
the changing action prompt, D-pad shortcuts, hearts, magic, and rupees—all rendered through the
game's native GPU interface. The patch leaves OoT3D's gameplay, progression, inventory rules, and
save format unchanged.

> [!IMPORTANT]
> This first release supports **USA OoT3D 1.0** (`0004000000033500`). EUR, JP, Master Quest, other
> revisions, and original 3DS hardware have not yet been validated.

## Features

- Native high-resolution HUD compatible with Azahar custom-texture scaling
- Modern Nintendo-style ABXY diamond in the upper-right
- Original live A action prompt, repositioned without replacing its changing text
- Live B, X, Y, I, and II item icons
- D-pad shortcuts for I, II, Navi/View, and the Ocarina
- Live hearts, magic meter, rupee icon, and rupee total
- Standalone C-stick free camera with sensitivity and inversion controls
- Compatible with an existing 4K OoT3D texture pack
- No intentional gameplay, balance, progression, inventory, or save-data changes

## Download and install

Download the latest **USA Azahar Plus** ZIP from
[Releases](https://github.com/OTPR26/OOT3DHud/releases/latest), then merge its `load` folder into
your Azahar user-data directory.

The release installs these files:

```text
load/mods/0004000000033500/code.ips
load/mods/0004000000033500/exheader.bin
load/textures/0004000000033500/UI/tex1_256x128_F23CD5DE9DCE99C4_4_mip0.png
```

Enable **Custom Textures** in Azahar and restart the emulator after changing files. On the tested AYN
setup, the Azahar Plus user-data directory is `/storage/emulated/0/ROMs/n3ds`.

Do not combine this release with another `code.ips` for the same title. The HUD, input remapping, and
free camera are already combined into the single included patch.

See [INSTALL.md](INSTALL.md) for the full installation and troubleshooting notes.

## Controls

| Input | Action |
| --- | --- |
| D-pad Left | Touchscreen item I |
| D-pad Down | Touchscreen item II |
| D-pad Up | Navi / View |
| D-pad Right | Ocarina |
| C-stick | Free camera |
| L + R + D-pad Up/Down | Adjust camera sensitivity |
| L + R + D-pad Left/Right | Change camera inversion |

Unassigned I or II slots remain blank. The L+R camera-settings chord takes priority over gameplay
D-pad actions.

## Compatibility

| Environment | Status |
| --- | --- |
| USA OoT3D 1.0 — Azahar Plus on Android | Tested |
| USA OoT3D 1.0 — Azahar on macOS | HUD and save loading tested |
| Existing 4K custom-texture pack | Tested |
| EUR / JP / other game revisions | Not yet validated |
| Master Quest | Not yet validated |
| Original 3DS hardware | Not yet validated |

### Current limitation

The reliable native-board allocation currently displays up to **12 hearts**. Saves with more heart
containers remain valid, but this HUD draws only the first 12.

## Building from source

Requirements:

- devkitARM / devkitPro with the 3DS rules installed
- Python 3

Build the tested USA/Azahar configuration:

```sh
./tools/build.sh
```

The resulting patch is `artifacts/USA/code.ips`.

Run the host-side D-pad routing tests without a 3DS toolchain:

```sh
make host-test
```

The tested `code.ips` SHA-256 is:

```text
820e1cda6d4744c91d033aaef6ad4965c400543800586d393b59098f452051f5
```

## How it works

Physical D-pad input is translated into OoT3D's original touchscreen samples, so the game retains its
normal item-usability checks and press/hold behavior. The HUD reuses a native stereoscopic top-screen
board and updates its position and UV buffers from live, read-only game state.

See [Implementation notes](docs/IMPLEMENTATION.md) and
[Native HD HUD notes](docs/NATIVE_HD_HUD.md) for technical details.

## Credits

- [Roberto-Nessy/OoT3D_Standalone_Free_Cam](https://github.com/Roberto-Nessy/OoT3D_Standalone_Free_Cam)
  — original standalone C-stick free camera and loader base
- [Project Restoration](https://restoration.zora.re/hd-hud)
  — HD HUD design and visual inspiration
- [gamestabled/OoT3D_Randomizer](https://github.com/gamestabled/OoT3D_Randomizer)
  — OoT3D structures and established hook conventions

This project is distributed under [GPLv3](LICENSE). Preserve all upstream attribution and file-level
license notices. Nintendo game dumps, extracted executables, and patched game binaries are not
included. See [NOTICE.md](NOTICE.md) for third-party and asset details.
