# Installing Ocarina Reframed

Choose the ZIP matching your copy of OoT3D:

| Package | Tested game version | Title ID |
| --- | --- | --- |
| USA | USA 1.0 | `0004000000033500` |
| EUR | Europe Rev 1 | `0004000000033600` |
| JP | Japan Rev 1 | `0004000000033400` |

1. Close OoT3D.
2. Open the Azahar or Citra user-data directory.
3. Merge the release's `load` folder into that directory.
4. Enable custom textures in Azahar.
5. Start the game normally; do not resume an old emulator save state made before installing the mod.

The resulting files use the title ID for your region:

```text
load/mods/<title-id>/code.ips
load/mods/<title-id>/exheader.bin
load/textures/<title-id>/UI/tex1_256x128_F23CD5DE9DCE99C4_4_mip0.png
```

For the tested AYN configuration, the Azahar user-data directory is
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
| ZL | Hide/show the minimap |
| Select | Open the Items screen |
| C-stick | Free camera |
| L + R + D-pad Up/Down | Free-camera sensitivity |
| L + R + D-pad Left/Right | Free-camera inversion |

Unassigned I and II slots appear blank by design.

## Attribution

The C-stick free camera is based on
[Roberto-Nessy/OoT3D_Standalone_Free_Cam](https://github.com/Roberto-Nessy/OoT3D_Standalone_Free_Cam).
The modern HUD is inspired by Project Restoration's HD HUD.
