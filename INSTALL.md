# Installing Ocarina Reframed 1.1

Choose Single Screen or Dual Screen, then Mod or ROM Patch. Each main ZIP has
folders for North America, Europe, Japan, Taiwan, and Korea. Use one edition
and one delivery method. A texture pack is not required.

## Mod

1. Extract the ZIP and choose your game's region.
2. Close the game and find the user-data folder for your emulator.
3. Move the existing mod folder for this game out of load/mods; keep a backup.
4. Merge the included regional load contents into the user-data folder.
5. Launch the supported original ROM normally.

The expected path is load/mods/<title-id>/code.ips alongside exheader.bin and
romfs. Standalone Azahar and RetroArch may use different user-data folders.
Do not move or delete saves. Do not resume an old emulator save state.

## ROM Patch

1. Choose your region and [verify the source-ROM checksum](docs/releases/1.1-checksums.md).
2. Apply patch.xdelta using an xdelta3-compatible patcher.
3. Save to a new ROM file; retain your original. Verify the output checksum.
4. Disable or remove existing HUD-related mods, then launch the patched ROM.

North America, Europe and Japan patches use the tested Rev 1 sources. Taiwan
and Korea use the exact source hashes listed above. Matching a filename or
region alone is not enough. Do not force checksum mismatches. The legacy
North America download now contains Single Screen 1.1 and requires the same
Rev 1 input; the older patch accepted a different source ROM.

## Controls

Right stick/C-stick: free camera. Select: Items. D-pad Left/Down: assigned
touchscreen items. Up: Navi/View. Right: Ocarina. ZL: minimap visibility.
Blank HUD circles are intentional and do not remap your physical controller.

## Compatibility and additional information

Do not combine competing code.ips files or apply the external mod over its
patched ROM. Other code mods need explicit compatibility checks even when not
HUD-related. These patches are not validated for arbitrary Randomizer outputs.
Use only a texture pack's textures, not its bundled code mod.

See [release notes](docs/releases/1.1.md), [ROM checksums](docs/releases/1.1-checksums.md),
[download checksums](docs/releases/1.1-download-checksums.txt),
[corresponding source](release-source/1.1/README.md), [license scope](LICENSE_SCOPE.md)
and [notices](NOTICE.md). The ZIPs contain only installable files and one README.
