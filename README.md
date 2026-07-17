# OoT3D Modern HUD + Free Camera

[![Release](https://img.shields.io/github/v/release/OTPR26/OOT3DHud?display_name=tag)](https://github.com/OTPR26/OOT3DHud/releases/latest)
[![License](https://img.shields.io/github/license/OTPR26/OOT3DHud)](LICENSE)
![Game](https://img.shields.io/badge/OoT3D-USA%201.0-4b8bbe)
![Tested](https://img.shields.io/badge/tested-Azahar%20%2B%20Azahar%20Plus-5aaa46)

A modern, high-resolution top-screen HUD and C-stick free camera for the USA 1.0 release of
*The Legend of Zelda: Ocarina of Time 3D*.

The HUD brings the Project Restoration style to OoT3D: ABXY in a diamond, live item assignments,
the changing action prompt, D-pad shortcuts, hearts, magic, and rupees, all rendered through the
game's native interface. The patch leaves OoT3D's gameplay, progression, inventory rules, and
save format unchanged.

> [!IMPORTANT]
> This first release supports **USA OoT3D 1.0** (`0004000000033500`). EUR, JP, Master Quest, other
> revisions, are still in progress.

## Features

- Native HUD compatible with Azahar custom-texture scaling
- Modern Nintendo-style ABXY diamond in the upper-right
- Original live A action prompt, repositioned without replacing its changing text
- Live B, X, Y, I, and II item icons
- D-pad shortcuts for I, II, Navi/View, and the Ocarina
- Live hearts, magic meter,and rupees
- C-stick free camera using [Roberto-Nessy/OoT3D_Standalone_Free_Cam](https://github.com/Roberto-Nessy/OoT3D_Standalone_Free_Cam)
- Compatible with Henriko's 4K texture pack(https://www.henrikomagnifico.com).



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
| USA OoT3D 1.0 — Azahar on macOS and Windows | Tested |
| Henriko's 4K custom-texture pack | Tested |
| EUR / JP / other game revisions | Not yet validated |
| Master Quest | Not yet validated |
| Original 3DS hardware | Not yet validated |

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
28ae0db0ed8b3668d1603f04aab7d1579e734868d80cd97a59b37b74fbee3073
```

## How it works

Physical D-pad input is translated into OoT3D's original touchscreen samples, so the game retains its
normal item-usability checks and press/hold behavior. The HUD reuses a native top-screen
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
