# Installing Ocarina Reframed 2.0

Choose Single Screen or Dual Screen, then Mods or ROM Patches. Each archive
contains North America Rev 1, Europe Rev 1 English, and Japan Rev 1. Use one
edition and one installation method. No texture pack is required.

## Emulator Mods

1. Extract the ZIP and choose your game's region.
2. Close the game and locate your emulator's user-data folder.
3. Back up the existing regional title folder outside `load/mods`.
4. Copy the included regional `load` contents into the user-data folder.
5. Start the clean original game; do not resume an older emulator save state.

The path is `load/mods/<title-id>/code.ips` alongside `exheader.bin` and `romfs`.
Standalone Azahar and RetroArch can use different user-data folders. Keep saves.

## ROM Patches

1. Choose your region and verify the [clean-ROM checksum](docs/releases/2.0-checksums.md).
2. Apply `patch.xdelta` with an xdelta3-compatible patcher.
3. Write a new output ROM and verify its checksum. Preserve the original.
4. Disable external mod overrides, then launch the patched ROM.

Do not force checksum mismatches or patch an already patched ROM. These ROM
patches are emulator downloads; physical consoles should use the separate Luma
package. Do not combine competing code mods or arbitrary randomized ROMs.

## Experimental Luma3DS

1. Use a clean, working game with the matching region and Rev 1 executable.
2. Back up your saves and the existing matching `luma/titles/<title-id>` folder.
3. Copy the matching region's `luma` folder from the experimental archive to the
   SD card root. Enable game patching in Luma3DS.
4. Start without other code mods, plugins, or a previously patched ROM.

The whole-ROM hash need not match for an external Luma mod; executable revision,
region and installed updates still matter. Hardware compatibility is unconfirmed.
During the first test, check title/load, Options, touch, shortcuts, free camera,
and Rosalina before saving. Restore the previous title folder to undo.
Report results in [issue #12](https://github.com/OTPR26/OOT3DHud/issues/12).

## Controls and settings

Right stick/C-stick: free camera. Select: Items. D-pad Left/Down: assigned
touchscreen items. Up: Navi/View. Right: Ocarina. ZL: minimap visibility.

The Dual Screen Options menu appears on the upper screen and uses controller
navigation: Up/Down selects, Left/Right adjusts, L/R changes columns, and the
Cancel/OK actions discard/apply changes. Native Save provides entry to Options.
Settings use separate SDMC sidecar files; recreated slots with the same name may
inherit them. Game save files remain separate.

To switch editions or delivery methods, close the game, back up the previous mod
folder, and install only the new selection. For ROM patches, always start from
the clean original. Keep your saves and avoid old emulator save states.

See [release notes](docs/releases/2.0.md), [corresponding source](release-source/2.0/README.md),
and the licenses and notices included with each download.
