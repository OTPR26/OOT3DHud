# Ocarina Reframed 1.1 corresponding source

Choose single or dual, then USA, EUR, JP, TWN, or KOR. BUILD.txt contains the build command and expected code.ips checksum. Licenses and notices are preserved from 1.0.

The North American English Game Over atlas correction is a native RomFS layout change, shared by both editions: in misc/us/english/gameover.qsp, sprite record 100 (40 bytes per record), keep its geometry and change the four little-endian float image coordinates at record offset 20 from (0, 56, 192, 32) to (0, 224, 768, 128). This matches the bundled 1024x1024 hud_all00.ctxb. All other language atlases retain 256x256 coordinates. The corrected layout is included in both USA Mod payloads and embedded in both USA ROM patches.

See the release notes for gameplay verification scope. Dual Screen executable sources are unchanged.
