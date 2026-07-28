# Installing Ocarina Reframed

Choose the ZIP matching your copy of OoT3D:

| Package | Tested game version | Title ID |
| --- | --- | --- |
| USA | USA 1.0 | `0004000000033500` |
| EUR | Europe Rev 1 | `0004000000033600` |
| JP | Japan Rev 1 | `0004000000033400` |
| TWN | Taiwan Rev 1 | `000400000008F900` |
| KOR | Korea Rev 1 | `000400000008F800` |

## Azahar or Citra Mod ZIP

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

USA and European packages also include their high-resolution message font under
`load/mods/<title-id>/romfs/message/`.

For the tested AYN configuration, the Azahar user-data directory is
`/storage/emulated/0/ROMs/n3ds`.

Do not combine this release with another code patch for the same title. Existing non-HUD custom
textures may remain installed. If troubleshooting, fully close and restart Azahar after changing
files.

The Mod ZIP is for emulators that support its custom-texture folder. Do not install it through
Luma3DS: applying its code without the matching emulator texture could leave HUD graphics missing or
malformed.

## ROM Patch ZIP

1. Extract the regional ROM Patch ZIP.
2. Open [Rom Patcher JS](https://www.marcrobledo.com/RomPatcher.js/).
3. Select a clean, legally obtained, decrypted `.3ds` or `.cci` dump matching the package's region
   and revision.
4. Select the included `.xdelta` patch, apply it, and save the resulting ROM.
5. Disable any separate Ocarina Reframed Mod ZIP installation before launching the patched ROM.

The patched ROM contains its required code and HUD resources and does not require emulator custom
textures. Encrypted, previously modified, or mismatched dumps will not patch correctly.

### Original 3DS with Luma3DS

Use the regional ROM Patch ZIP, not the Mod ZIP. Original-hardware operation has not yet been
validated by this project; a compatible method of launching the legally dumped, patched ROM is
required. Back up the original ROM and save data before testing.

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
| Hold L + R + ZR | Cycle HUD size: 75%, 100%, 125%, Off |
| L + R + D-pad Up/Down | Free-camera sensitivity |
| L + R + D-pad Left/Right | Free-camera inversion |

Unassigned I and II slots appear blank by design.

## Attribution

The C-stick free camera is based on
[Roberto-Nessy/OoT3D_Standalone_Free_Cam](https://github.com/Roberto-Nessy/OoT3D_Standalone_Free_Cam).
The modern HUD is inspired by Project Restoration's HD HUD.
