# Ocarina Reframed 1.0

Two ways to experience Ocarina of Time 3D:

- **Single Screen:** redesigned file selection, Items, Gear, Map, and Songs on the main screen, with item previews, descriptions, and live controller assignments.
- **Dual Screen:** the original two-screen experience with the modern HUD and free camera.

Both include a controller-neutral diamond.

Four downloads: Single Screen or Dual Screen, each available as **Mod** or **ROM Patch**, with all five regional folders inside.

## Download

[Ocarina Reframed 1.0 release](https://github.com/OTPR26/OOT3DHud/releases/tag/v1.0)

Choose one ZIP, then the folder for North America, Europe, Japan, Taiwan, or Korea:

- `Ocarina-Reframed-1.0-Single-Screen-Mods.zip`
- `Ocarina-Reframed-1.0-Single-Screen-ROM-Patches.zip`
- `Ocarina-Reframed-1.0-Dual-Screen-Mods.zip`
- `Ocarina-Reframed-1.0-Dual-Screen-ROM-Patches.zip`

No game ROMs or saves are included. 

A 4K texture pack is not required but these patches and mods are compatible with Henriko's 4K texture packs.

## Mod installation

1. Extract the ZIP and choose your game's region.
2. Find the user-data folder for your emulator.
3. Move any existing regional mod contents out of `load/mods`.
4. Merge the included regional `load` contents into the user-data folder.
5. Launch the supported ROM normally.

Keep backups outside `load/mods`. Use the directory belonging to the emulator you run; standalone Azahar and RetroArch may use different directories.

## ROM-patch installation

1. Choose your region and check your original ROM against `SOURCE.txt`.
2. Apply `patch.xdelta` using an xdelta3-compatible patcher.
3. Verify the patched ROM's output checksum.
4. Disable or remove any existing HUD-related mod, then launch the patched ROM.

Preserve your original ROM and write to a new output file. Do not force checksum mismatches or combine competing code patches. Other code mods need explicit compatibility validation even when they are not HUD-related.

See [installation details](INSTALL.md) and [release notes](docs/releases/1.0.md).


## Credits and licensing

- Roberto-Nessy / OoT3D_Standalone_Free_Cam — free-camera and loader foundation.
- Project Restoration — HUD inspiration and artwork attribution.
- gamestabled / OoT3D_Randomizer — OoT3D structures and hook references.

Preserve included file-level licenses and notices. See [license scope](LICENSE_SCOPE.md) and [notices](NOTICE.md); source-code licensing does not relicense third-party artwork or game data.
