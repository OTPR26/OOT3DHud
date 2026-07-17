# Installing the USA Azahar / Azahar Plus release

This release targets USA OoT3D 1.0, title ID `0004000000033500`.

1. Close OoT3D.
2. Open the Azahar or Azahar Plus user-data directory.
3. Merge the release's `load` folder into that directory.
4. Enable custom textures in Azahar.
5. Start the game normally; do not resume an old emulator save state made before installing the mod.

The resulting files should be:

```text
load/mods/0004000000033500/code.ips
load/mods/0004000000033500/exheader.bin
load/textures/0004000000033500/UI/tex1_256x128_F23CD5DE9DCE99C4_4_mip0.png
```

For the tested AYN configuration, the Azahar Plus user-data directory is
`/storage/emulated/0/ROMs/n3ds`.

Do not combine this release with another code patch for the same title. Existing non-HUD custom
textures may remain installed. If troubleshooting, fully close and restart Azahar after changing
files.

## Expected controls

| Input | Action |
| --- | --- |
| D-pad Left | Touchscreen item I |
| D-pad Down | Touchscreen item II |
| D-pad Up | Navi / View |
| D-pad Right | Ocarina |
| C-stick | Free camera |
| L + R + D-pad Up/Down | Free-camera sensitivity |
| L + R + D-pad Left/Right | Free-camera inversion |

Unassigned I and II slots appear blank by design.

## Attribution

The C-stick free camera is based on
[Roberto-Nessy/OoT3D_Standalone_Free_Cam](https://github.com/Roberto-Nessy/OoT3D_Standalone_Free_Cam).
The modern HUD is inspired by Project Restoration's HD HUD.
