# Release checklist

## Before making a public repository

- [x] Preserve and prominently link Roberto-Nessy's standalone free-camera attribution.
- [x] License original OTPR26 source contributions under GPL-3.0-or-later while preserving all
      inherited file-level notices and separate asset terms.
- [ ] Preserve every file-level copyright and license notice.
- [ ] Keep `artifacts/`, `derived-assets/`, `game-dumps/`, loose `exheader.bin` files, and private
      texture-pack files out of the public repository.
- [ ] Confirm redistribution terms for the Project Restoration-derived release atlas and generated
      legacy framebuffer header.
- [ ] Obtain or document permission for upstream-authored free-camera portions that do not carry an
      explicit file-level license.
- [ ] Confirm the source archive contains no `.git` directory, ROM data, credentials, device paths,
      or experimental IPS builds.
- [ ] Rebuild from the clean source snapshot and compare `code.ips` with the tested hash.

## Tested USA files (private validation only)

```text
code.ips
SHA-256 28ae0db0ed8b3668d1603f04aab7d1579e734868d80cd97a59b37b74fbee3073

exheader.bin
SHA-256 48c9fc32dd18df64eced1570e79ab634b189c9385bedc087126d7e3a3d827530

tex1_256x128_F23CD5DE9DCE99C4_4_mip0.png
SHA-256 b1a065bf7bfdb058c9fb1bc1f9a0d438a7199356b4f350b2d72978c5c0670f22
```

These hashes identify the exact files validated on the private test setup. Their presence here does
not grant permission to redistribute the files.
