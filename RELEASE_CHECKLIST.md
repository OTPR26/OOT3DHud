# Release checklist

## Before making a public repository

- [x] Preserve and prominently link Roberto-Nessy's standalone free-camera attribution.
- [x] Preserve the repository's GPLv3 license and all inherited file-level notices.
- [ ] Preserve every file-level copyright and license notice.
- [ ] Keep `artifacts/`, `derived-assets/`, `game-dumps/`, loose `exheader.bin` files, and private
      texture-pack files out of the public repository.
- [ ] Confirm redistribution terms for the Project Restoration-derived release atlas and generated
      legacy framebuffer header.
- [ ] Confirm the source archive contains no `.git` directory, ROM data, credentials, device paths,
      or experimental IPS builds.
- [ ] Rebuild from the clean source snapshot and compare `code.ips` with the tested hash.

## Tested USA files (private validation only)

```text
code.ips
SHA-256 820e1cda6d4744c91d033aaef6ad4965c400543800586d393b59098f452051f5

exheader.bin
SHA-256 48c9fc32dd18df64eced1570e79ab634b189c9385bedc087126d7e3a3d827530

tex1_256x128_F23CD5DE9DCE99C4_4_mip0.png
SHA-256 c0e2593bd23afdd29900191041229127d3a0cf7ea3d0b4b3185b3420bd18e966
```

These hashes identify the exact files validated on the private test setup. Their presence here does
not grant permission to redistribute the files.
