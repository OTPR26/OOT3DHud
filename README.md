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

Download the package matching your game region from
[Releases](https://github.com/OTPR26/OOT3DHud/releases/latest). Each region has a normal emulator
mod and a self-contained ROM patch.

### Azahar, Citra, or other emulators

1. Download the regional **Mod ZIP**.
2. Extract the ZIP and merge its `load` folder into the Azahar or Citra user-data directory.
3. Enable **Use Custom Textures** in the emulator's graphics settings.
4. Restart the emulator and launch the game normally.

Existing unrelated texture packs can remain installed, but do not combine Ocarina Reframed with
another `code.ips` patch for the same title. See [INSTALL.md](INSTALL.md) for the regional title IDs
and complete folder paths.

### ROM patch

1. Download and extract the regional **ROM Patch ZIP**.
2. Open [Rom Patcher JS](https://www.marcrobledo.com/RomPatcher.js/).
3. Select a clean, legally obtained, decrypted `.3ds` or `.cci` dump matching the region and
   revision listed in the included README.
4. Select the included `.xdelta` file and apply the patch.
5. Save the resulting ROM, add it to Azahar or Citra, and launch it normally.

Xdelta patches require the exact original ROM specified by the package. Encrypted, previously
modified, or mismatched dumps will not work. Disable the separate Ocarina Reframed mod-folder
installation when using a patched ROM so the mod is not applied twice. Back up save data before
testing any modified ROM.

### Original 3DS with Luma3DS

Original 3DS support is still in progress. Luma3DS can load the included `code.ips` and
`exheader.bin`, but the current HD HUD atlas uses Azahar/Citra custom-texture replacement and cannot
yet be loaded by Luma3DS. 3DS users may instead prefer to use the rom patching directions above.

## Compatibility

| Environment | Status |
| --- | --- |
| USA Rev 0/1 | Supported |
| Europe Rev 1 | Supported |
| Japan Rev 1 | Supported |
| Azahar/Citra — macOS, Windows, Android | Supported |
| Henriko's 4K texture pack | Supported |
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
USA  b1ec0136f09232bea190d643fe5b4d29a6e5bfa3efe15abc50439cd8e1d5c565
EUR  7766c0da9778a49b0311f1603ffe070234a47eff109d96d46c5040e709d3f191
JP   3541d2881c5c9eab5b88c151909a3741d6943012505c29463b1cbf15f5f44000
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
