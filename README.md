# OoT3D Modern HUD + Free Camera

[![Release](https://img.shields.io/github/v/release/OTPR26/OOT3DHud?display_name=tag)](https://github.com/OTPR26/OOT3DHud/releases/latest)
[![License](https://img.shields.io/github/license/OTPR26/OOT3DHud)](LICENSE)
![Game](https://img.shields.io/badge/OoT3D-USA%20%7C%20EUR%20%7C%20JP-4b8bbe)
![Tested](https://img.shields.io/badge/tested-Azahar-5aaa46)

**Website:** [ocarinareframed.com](https://ocarinareframed.com)

A modern, high-resolution top-screen HUD and C-stick free camera for the USA, European, and
Japanese releases of *The Legend of Zelda: Ocarina of Time 3D*.

The HUD brings the Project Restoration style to OoT3D: ABXY in a diamond, live item assignments,
the changing action prompt, D-pad shortcuts, hearts, magic, and rupees, all rendered through the
game's native interface. The patch leaves OoT3D's gameplay, progression, inventory rules, and
save format unchanged.

> [!IMPORTANT]
> Download the package matching your game region. The tested versions are **USA 1.0**,
> **Europe Rev 1**, and **Japan Rev 1**.

## Features

- Native HUD compatible with Azahar custom-texture scaling
- Modern ABXY diamond in the upper-right
- Original live A action prompt, repositioned without replacing its changing text
- Live B, X, Y, I, and II item icons
- Live ammo counts for consumable X, Y, I, and II items
- D-pad shortcuts for I, II, Navi/View, and the Ocarina
- ZL minimap hide/show control, preserving access after the D-pad remap
- Select opens the native Items screen
- Live hearts, magic meter, and rupees
- C-stick free camera using [Roberto-Nessy/OoT3D_Standalone_Free_Cam](https://github.com/Roberto-Nessy/OoT3D_Standalone_Free_Cam)
- Compatible with [Henriko Magnifico's OoT3D 4K texture pack](https://www.henrikomagnifico.com/zelda-ocarina-of-time-3d-4k).



## Controls

| Input | Action |
| --- | --- |
| D-pad Left | Touchscreen item I |
| D-pad Down | Touchscreen item II |
| D-pad Up | Navi / View |
| D-pad Right | Ocarina |
| ZL | Hide/show the minimap |
| Select | Open the Items screen |
| C-stick | Free camera |
| L + R + D-pad Up/Down | Adjust camera sensitivity |
| L + R + D-pad Left/Right | Change camera inversion |

Unassigned I or II slots remain blank. The L+R camera-settings chord takes priority over gameplay
D-pad actions.

## Download and install

Download the ZIP matching your game region from
[Releases](https://github.com/OTPR26/OOT3DHud/releases/latest), then merge its `load` folder into
your Azahar or Citra user-data directory. See [INSTALL.md](INSTALL.md) for complete instructions.

## Compatibility

| Environment | Status |
| --- | --- |
| USA 1.0 | Tested |
| Europe Rev 1 | Tested |
| Japan Rev 1 | Tested |
| Azahar/Citra — macOS, Windows, Android | Supported |
| Henriko's 4K custom-texture pack | Tested |
| Other regions | In Progress |
| Original 3DS + Lumas3DS | Not yet validated |

## Building from source

Requirements:

- devkitARM / devkitPro with the 3DS rules installed
- Python 3

Build a regional configuration:

```sh
REGION=USA ./tools/build.sh
REGION=EUR ./tools/build.sh
REGION=JP ./tools/build.sh
```

The resulting patches are written to `artifacts/<region>/code.ips`.

Run the host-side D-pad routing tests without a 3DS toolchain:

```sh
make host-test
```

The tested `code.ips` SHA-256 values are:

```text
USA  397c97bc0a53372dd74c22ce8acdba35f971012b189a9b7a5c2ae9318f73d3e5
EUR  8b7df0358b079d5dc08dab3740c105e4c7470c90a62467faf6d85593ffd7b88a
JP   bed48705b758ddd74d5540b17b8750a0b65aae7b061e5f3f194e185b3c639289
```

## How it works

Physical D-pad input is translated into OoT3D's original touchscreen samples, so the game retains its
normal item-usability checks and press/hold behavior. ZL reproduces the original minimap hide/show
gestures through OoT3D's sampled controller state. Select opens the native Items screen through its
original touchscreen action. The HUD reuses a native top-screen
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

Original source contributions authored for this repository are licensed under
[GPL-3.0-or-later](LICENSE), unless a file says otherwise. Third-party source retains its original
notices and terms; artwork, game-derived data, trademarks, and other non-code assets are not covered
by that GPL grant. See [LICENSE_SCOPE.md](LICENSE_SCOPE.md) and [NOTICE.md](NOTICE.md) for details.
