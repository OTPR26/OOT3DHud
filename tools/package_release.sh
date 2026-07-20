#!/bin/sh
set -eu

: "${VERSION:?Set VERSION to the release version, for example VERSION=0.4.0}"
version="$VERSION"
region="${REGION:-USA}"
project_dir=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
dist_dir="$project_dir/dist"
work_dir=$(mktemp -d "${TMPDIR:-/tmp}/oot3d-modern-hud-package.XXXXXX")
case "$region" in
    USA)
        title_id="0004000000033500"
        ;;
    EUR)
        title_id="0004000000033600"
        ;;
    JP)
        title_id="0004000000033400"
        ;;
    *)
        echo "Unsupported region: $region (expected USA, EUR, or JP)" >&2
        exit 1
        ;;
esac
release_name="Ocarina-Reframed-v${version}-${region}-Mod"

cleanup() {
    rm -rf "$work_dir"
}
trap cleanup EXIT INT TERM

mkdir -p "$dist_dir"
rm -f "$dist_dir/$release_name.zip"

mkdir -p \
    "$work_dir/$release_name/load/mods/$title_id" \
    "$work_dir/$release_name/load/textures/$title_id/UI"
cp "$project_dir/artifacts/$region/code.ips" \
    "$work_dir/$release_name/load/mods/$title_id/code.ips"
cp "$project_dir/artifacts/$region/exheader.bin" \
    "$work_dir/$release_name/load/mods/$title_id/exheader.bin"
cp "$project_dir/artifacts/USA/tex1_256x128_F23CD5DE9DCE99C4_4_mip0.png" \
    "$work_dir/$release_name/load/textures/$title_id/UI/tex1_256x128_F23CD5DE9DCE99C4_4_mip0.png"
cp "$project_dir/INSTALL.md" "$work_dir/$release_name/README.md"
cp "$project_dir/NOTICE.md" "$work_dir/$release_name/NOTICE.md"

(
    cd "$work_dir"
    zip -q -r "$dist_dir/$release_name.zip" "$release_name"
)

echo "Created:"
echo "  dist/$release_name.zip"
