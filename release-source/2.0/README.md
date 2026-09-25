# Ocarina Reframed 2.0 corresponding source

Choose single or dual, then USA, EUR, or JP. BUILD.txt records the build command
and expected code.ips hash. The Dual Screen sources also build the experimental
Luma payload with CITRA=0. Its hardware exheader reserves the existing initialized
data plus page-aligned BSS as initialized data, with BSS set to zero; title IDs,
text addresses and capabilities stay regional. Emulator headers retain their
emulator memory layout.

Dual Screen retains the prior HUD and native inventory, with new Options and
title/file-selection presentation. Its controller adapter uses the game's native
touch output without writing to HID shared memory. Japanese Options uses Noto
Sans CJK JP; the SIL OFL notice accompanies those sources and release archives.

File-level licenses and attributions are preserved. See each region's LICENSE,
NOTICE.md and LICENSE_SCOPE.md. These terms do not relicense artwork or game data.
The source snapshots do not contain ROMs or game executables.

The additional host test in tests/test_dual_port_input.c compiles the actual
Dual Screen input implementation with test bindings for native state. From the
2.0 source directory, for example:

```sh
cc -std=c11 -Wall -Wextra -Werror -Wno-int-to-pointer-cast -Wno-pointer-to-int-cast -DVersion_USA -Idual/USA/include -Idual/USA/src tests/test_dual_port_input.c dual/USA/src/controls.c -o /tmp/reframed-dual-input-test
/tmp/reframed-dual-input-test
```

Replace USA with EUR or JP for the other builds. The test suppresses target-layout
static assertions on the host; it does not validate physical-console rendering.
