# Ocarina Reframed 1.0 — Emulator Mod

This README is included in both Mod ZIPs; the ZIP name identifies your edition.
Single Screen moves the menu experience onto the main screen. Dual Screen
keeps both screens with the modern HUD and free camera.

## Before you start

- Supply your own supported copy of Ocarina of Time 3D. No ROM is included.
- Choose **one edition** and **one delivery method**. Use this mod with an
  original supported ROM, not a ROM already patched with Ocarina Reframed.
- Close the game. Back up saves and any existing regional mod folder.
- A 4K texture pack is **not required**. This package includes its native HUD
  and menu resources; custom textures need not be enabled for the base mod.

## Install

1. Extract the ZIP and choose your game's region.
2. Find the user-data folder for your emulator.
3. Move any existing regional mod contents out of `load/mods`.
4. Merge the included regional `load` contents into the user-data folder.
5. Launch the supported ROM normally.

Keep the removed regional folder as a backup outside `load/mods`; leave other
games' folders untouched. Do not merge competing `code.ips`, `exheader.bin`,
or menu-resource files. RetroArch's core may use a different user-data folder
from standalone Azahar. Preserve the title-ID and `romfs` subfolders:

```text
<user-data>/load/mods/<title-id>/code.ips
<user-data>/load/mods/<title-id>/exheader.bin
<user-data>/load/mods/<title-id>/romfs/...
```

Do not resume an emulator save state created before installation or while
using a different edition.

On the tested AYN Azahar Plus setup, the user-data directory is `ROMs/n3ds`.
That path is specific to that setup, not a universal Android default.

| Regional folder | Title ID |
| --- | --- |
| North-America | `0004000000033500` |
| Europe | `0004000000033600` |
| Japan | `0004000000033400` |
| Taiwan | `000400000008F900` |
| Korea | `000400000008F800` |

Use `COMPATIBILITY.json` for the exact tested source and regional scope. The
title ID identifies the region, not every compatible revision.

## Controls

These are emulated 3DS inputs; map your physical controller accordingly.

| Input | Action |
| --- | --- |
| Right stick / C-stick | Free camera |
| Select | Open Items |
| D-pad Left | First touchscreen item slot |
| D-pad Down | Second touchscreen item slot |
| D-pad Up | Navi / View |
| D-pad Right | Ocarina |
| ZL | Toggle minimap visibility |

Blank circles are intentional. They remove printed button letters; they do
not remap your emulator's controller bindings. Unassigned item slots stay blank.

## Optional texture packs

Install only the texture files in the appropriate `load/textures` directory.
If your pack bundles a separate code mod, do **not** install that bundled
`load/mods` directory over Ocarina Reframed. Enable custom textures only when
using external texture replacements. The final regional builds have not all been retested with optional packs;
earlier tests are not a blanket compatibility guarantee.

## Switching editions or removing the mod

Close the game. Move the current title-ID mod folder out of `load/mods`, then
install the other edition's complete regional folder. To uninstall, leave it
out and restore any backup you made. Do not delete the emulator's save folders.

## Troubleshooting and test scope

Missing HUD or unchanged menus: check the running emulator's user directory,
region/title ID, extracted folder depth, and conflicting mods; then fully restart.
For Single Screen, the original secondary display is not the redesigned view.

All five regions have scoped AYN Azahar Plus mod/ROM checks without textures.
Windows/RetroArch, optional packs, other revisions/languages and Randomizer
combinations are not established by those checks. Never assume multiple code
patches can be combined by copying them into one folder.

Preserve the supplied notices and licenses. Consult `SHA256SUMS.txt` to verify package contents.

## Corresponding source

The `source` folder includes the exact source for each regional code patch.
See each region’s `BUILD.txt` for build flags and the expected binary checksum.

---

# Ocarina Reframed 1.0 — ROM Patch

This README is included in both ROM-Patch ZIPs; the ZIP name identifies your
edition. Single Screen moves the menu experience onto the main screen. Dual
Screen keeps both screens with the modern HUD and free camera.

## Before you start

- Supply your own supported **clean, decrypted ROM**. No game ROM is included.
- Back up your original ROM and saves. Always create a separate patched output.
- A 4K texture pack is **not required**. The patch incorporates the necessary
  native resources into your copy of the game.
- Use one edition. Do not apply a second edition over an already-patched ROM.

## Patch and launch

1. Choose your region and check your original ROM against `SOURCE.txt`.
2. Apply `patch.xdelta` using an xdelta3-compatible patcher.
3. Verify the patched ROM's output checksum.
4. Disable or remove any existing HUD-related mod, then launch the patched ROM.

Extract the ZIP first. Your original ROM's SHA-256 must match **Source SHA-256**
in `SOURCE.txt` exactly; matching the region or filename alone is not enough.
Use a new output filename and confirm it matches **Output SHA-256**. If patching
reports a checksum mismatch, stop; do not force it.

Close the emulator before moving mod files. Keep a backup outside `load/mods`.
Do not apply Ocarina Reframed's external mod again on top of its patched ROM.
Other code mods also require explicit compatibility validation, even if they
are not HUD-related. Start normally rather than resuming an old save state.

Optional command-line equivalent:

```sh
xdelta3 -d -s "original.3ds" "patch.xdelta" "Ocarina-Reframed.3ds"
```

To calculate SHA-256:

```sh
# macOS
shasum -a 256 "original.3ds"
# Linux
sha256sum "original.3ds"
```

```powershell
# Windows PowerShell
Get-FileHash "original.3ds" -Algorithm SHA256
```

North America, Europe and Japan patches target the tested Rev 1 source files.
Taiwan and Korea target the exact regional sources identified by their hashes.
Encrypted files, other revisions, and different ROM containers may not match.

## Optional texture packs

The game works without one. To use a pack, install its regional texture files
and enable your emulator's custom textures. Do not install the pack's bundled
code mod over this patched ROM. The final regional builds have not all been retested with optional packs.

## Controllers, saves, and edition changes

Map physical inputs to the emulator's 3DS controls: right stick/C-stick for
free camera; Select for Items; D-pad Left/Down for assigned touchscreen items,
Up for Navi/View, Right for Ocarina; ZL for minimap visibility. Blank HUD
circles are intentional and do not change your controller mapping.

Keep your saves backed up. Do not delete save directories when switching.
To change editions, patch your untouched original ROM with the other edition's
patch. To uninstall, run the original ROM again.

## Randomizer and other mods

These are **clean-ROM patches**, not patches for arbitrary randomized games.
Do not force them onto a Randomizer output. A specific compatible patching
workflow must be validated before support is claimed. The ROM-patch option
alone does not make unrelated code or resource changes compatible.

## Test scope

All five regions have scoped AYN Azahar Plus mod/ROM checks without textures.
European checks used English and Taiwan checks used Simplified Chinese.
These results do not validate every revision, language, platform, texture pack,
HUD scale, or gameplay scenario. See `COMPATIBILITY.json` for exact scope.

Preserve the supplied notices and licenses. `SHA256SUMS.txt` verifies ZIP contents; `SOURCE.txt` verifies ROMs.

## Corresponding source

The `source` folder includes the exact source for each regional code patch.
See each region’s `BUILD.txt` for build flags and the expected binary checksum.
