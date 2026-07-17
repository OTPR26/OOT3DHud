#!/bin/sh
set -eu

region="${REGION:-USA}"
citra="${CITRA:-1}"
plus_controls_only="${PLUS_CONTROLS_ONLY:-}"
plus_minimal_hud="${PLUS_MINIMAL_HUD:-}"
plus_hearts_only="${PLUS_HEARTS_ONLY:-}"
plus_rupees_stage="${PLUS_RUPEES_STAGE:-}"
# This is the final tested native-HUD configuration. The historical variable
# name is retained internally to avoid disturbing verified conditional layout.
plus_native_magic_stage="${PLUS_NATIVE_MAGIC_STAGE:-1}"
devkitpro="${DEVKITPRO:-/opt/devkitpro}"
devkitarm="${DEVKITARM:-${devkitpro}/devkitARM}"
project_dir=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
stage_dir=$(mktemp -d "${TMPDIR:-/tmp}/oot3d-modern-hud.XXXXXX")

cleanup() {
    rm -rf "$stage_dir"
}
trap cleanup EXIT INT TERM

if [ ! -x "$devkitarm/bin/arm-none-eabi-gcc" ]; then
    echo "devkitARM was not found at: $devkitarm" >&2
    exit 1
fi

rsync -a \
    --exclude .git \
    --exclude build \
    --exclude artifacts \
    --exclude code.ips \
    --exclude '*.elf' \
    "$project_dir/" "$stage_dir/"

DEVKITPRO="$devkitpro" DEVKITARM="$devkitarm" \
    make -C "$stage_dir" all REGION="$region" citra="$citra" \
    plus_controls_only="$plus_controls_only" plus_minimal_hud="$plus_minimal_hud" \
    plus_hearts_only="$plus_hearts_only" plus_rupees_stage="$plus_rupees_stage" \
    plus_native_magic_stage="$plus_native_magic_stage"

mkdir -p "$project_dir/artifacts/$region"
cp "$stage_dir/code.ips" "$project_dir/artifacts/$region/code.ips"
cp "$stage_dir/$(basename "$stage_dir").elf" \
    "$project_dir/artifacts/$region/oot3d-modern-hud.elf"

echo "Built artifacts/$region/code.ips"
