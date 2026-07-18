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
SHA-256 c54e29a163ede7452f54b94e45f2ef1d42b0afe81de9a41f17287d9e0a570c15

exheader.bin
SHA-256 48c9fc32dd18df64eced1570e79ab634b189c9385bedc087126d7e3a3d827530

tex1_256x128_F23CD5DE9DCE99C4_4_mip0.png
SHA-256 a0f29f85f29f87a678c49bebc114f3b90557fd86f4d047020fa285403f1c55a7
```

These hashes identify the exact files validated on the private test setup. Their presence here does
not grant permission to redistribute the files.
