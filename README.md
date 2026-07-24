# Ocarina Reframed: OoT3D Single-Screen HUD + Free Camera

[![Release](https://img.shields.io/github/v/release/OTPR26/OOT3DHud?display_name=tag)](https://github.com/OTPR26/OOT3DHud/releases/latest)
[![License](https://img.shields.io/github/license/OTPR26/OOT3DHud)](LICENSE)
![Game](https://img.shields.io/badge/OoT3D-USA%20%7C%20EUR%20%7C%20JP%20%7C%20TWN%20%7C%20KOR-4b8bbe)
![Tested](https://img.shields.io/badge/tested-Azahar-5aaa46)

[Website](https://ocarinareframed.com) · [GameBanana](https://gamebanana.com/mods/695604) · [Latest release](https://github.com/OTPR26/OOT3DHud/releases/latest)

A modern, high-resolution top-screen HUD and C-stick free camera for the USA, European, Japanese,
Taiwanese, and Korean releases of *The Legend of Zelda: Ocarina of Time 3D*.

The HUD brings the Project Restoration style to OoT3D: ABXY in a diamond, live item assignments,
the changing action prompt, D-pad shortcuts, hearts, magic, and rupees, all rendered through the
game's native interface. The patch leaves OoT3D's gameplay, progression, inventory rules, and
save format unchanged.

> [!IMPORTANT]
> Download the package matching your game region. The tested versions are **USA 1.0**,
> **Europe Rev 1**, **Japan Rev 1**, **Taiwan Rev 1**, and **Korea Rev 1**.

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
- Runtime HUD scaling with 75%, 100%, 125%, and Off settings
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
| Hold L + R + ZR | Cycle HUD size: 75%, 100%, 125%, Off |
| L + R + D-pad Up/Down | Adjust camera sensitivity |
| L + R + D-pad Left/Right | Change camera inversion |

Unassigned I or II slots remain blank. Hold L+R+ZR without pressing another button to change the
HUD size. The L+R camera-settings chord takes priority over gameplay D-pad actions.

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
| Taiwan Rev 1 | Supported |
| Korea Rev 1 | Supported |
| Azahar/Citra — macOS, Windows, Android | Supported |
| Henriko's 4K texture pack | Supported |
| Original 3DS + Luma3DS | Not yet validated - Should work with rom patching |

## Building from source

Requirements:

- devkitARM / devkitPro with the 3DS rules installed
- Python 3

Build a regional configuration:

```sh
REGION=USA ./tools/build.sh
REGION=EUR ./tools/build.sh
REGION=JP ./tools/build.sh
REGION=TWN ./tools/build.sh
REGION=KOR ./tools/build.sh
```

The resulting patches are written to `artifacts/<region>/code.ips`.

Run the host-side D-pad routing tests without a 3DS toolchain:

```sh
make host-test
```

The tested `code.ips` SHA-256 values are:

```text
USA  6e36868f77c4bf564816d81656df14bb0db05926466e0750f19f9a22f43a5d0e
EUR  845c440613e9496deba585ca180cdbf162aa176b2ec042ebba381c5b0aac58b0
JP   e2f378e42fbbdb7fc2f5939be91da6147d66ed9a75e0d1d0df1519f5f735cc0a
TWN  9281f0f52ceb08b9126d24042723ed4933b387e7a49c4073eb11ff954e6c092f
KOR  ae7c5dbdcfc4b43e8ec4a81267deed2570678e554bb66545ff42eeb4940753a2
```

## Controls

ABXY Switch-style diamond 
D-pad items (Fairy/Look Up, Ocarina Right, Items I & II Down and Left)
Select-to-Items access
ZL minimap toggle
Single screen hearts, magic and rupees
C-stick free camera (Sensitivity: Press L + R + D-Pad Up/Down to increase or decrease the camera speed.Invert Axes: Press L + R + D-Pad Left/Right to toggle axes inversion (Neither, just X, just Y, or Both).
 
Adjust HUD scaling or turn the HUD off - L+R+ZR rotates through 75%, 100%, 125%, Off (defaults to 100%)

Each HUD element reflects the game’s live gameplay.

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
