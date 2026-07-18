#!/bin/sh
set -eu

version="${VERSION:-0.2.0}"
project_dir=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
dist_dir="$project_dir/dist"
work_dir=$(mktemp -d "${TMPDIR:-/tmp}/oot3d-modern-hud-package.XXXXXX")
source_name="OoT3D-Modern-HUD-Free-Cam-v${version}-source"
release_name="OoT3D-Modern-HUD-Free-Cam-v${version}-USA-Azahar"
title_id="0004000000033500"

cleanup() {
    rm -rf "$work_dir"
}
trap cleanup EXIT INT TERM

mkdir -p "$dist_dir"
rm -f "$dist_dir/$source_name.zip" "$dist_dir/$release_name.zip"

mkdir -p "$work_dir/$source_name"
rsync -a \
    --exclude .git \
    --exclude build \
    --exclude artifacts \
    --exclude dist \
    --exclude derived-assets \
    --exclude game-dumps \
    --exclude code.ips \
    --exclude exheader.bin \
    --exclude '*.elf' \
    --exclude '*.map' \
    --exclude '.DS_Store' \
    "$project_dir/" "$work_dir/$source_name/"

mkdir -p \
    "$work_dir/$release_name/load/mods/$title_id" \
    "$work_dir/$release_name/load/textures/$title_id/UI"
cp "$project_dir/artifacts/USA/code.ips" \
    "$work_dir/$release_name/load/mods/$title_id/code.ips"
cp "$project_dir/artifacts/USA/exheader.bin" \
    "$work_dir/$release_name/load/mods/$title_id/exheader.bin"
cp "$project_dir/artifacts/USA/tex1_256x128_F23CD5DE9DCE99C4_4_mip0.png" \
    "$work_dir/$release_name/load/textures/$title_id/UI/tex1_256x128_F23CD5DE9DCE99C4_4_mip0.png"
cp "$project_dir/INSTALL.md" "$work_dir/$release_name/README.md"
cp "$project_dir/LICENSE" "$work_dir/$release_name/LICENSE"
cp "$project_dir/LICENSE_SCOPE.md" "$work_dir/$release_name/LICENSE_SCOPE.md"
cp "$project_dir/NOTICE.md" "$work_dir/$release_name/NOTICE.md"

(
    cd "$work_dir/$release_name"
    find load -type f -print | LC_ALL=C sort | while IFS= read -r file; do
        shasum -a 256 "$file"
    done > SHA256SUMS.txt
)

(
    cd "$work_dir"
    zip -q -r "$dist_dir/$source_name.zip" "$source_name"
    zip -q -r "$dist_dir/$release_name.zip" "$release_name"
)

(
    cd "$dist_dir"
    shasum -a 256 "$source_name.zip" "$release_name.zip" > SHA256SUMS.txt
)

echo "Created:"
echo "  dist/$source_name.zip"
echo "  dist/$release_name.zip"
echo "  dist/SHA256SUMS.txt"
