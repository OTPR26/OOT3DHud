#!/bin/sh
set -eu

region="${REGION:-USA}"
citra="${CITRA:-1}"
hud_scale="${HUD_SCALE:-100}"
devkitpro="${DEVKITPRO:-/opt/devkitpro}"
devkitarm="${DEVKITARM:-${devkitpro}/devkitARM}"
project_dir=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
output_dir="${OUTPUT_DIR:-$project_dir/artifacts/$region}"
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
    --exclude game-dumps \
    --exclude test-builds \
    --exclude test-backups \
    --exclude dist \
    --exclude release-v\* \
    --exclude repo-release \
    --exclude website \
    --exclude .pnpm-store \
    --exclude code.ips \
    --exclude '*.elf' \
    "$project_dir/" "$stage_dir/"

DEVKITPRO="$devkitpro" DEVKITARM="$devkitarm" \
    make -C "$stage_dir" all REGION="$region" citra="$citra" \
    HUD_SCALE="$hud_scale"

mkdir -p "$output_dir"
cp "$stage_dir/code.ips" "$output_dir/code.ips"
cp "$stage_dir/$(basename "$stage_dir").elf" \
    "$output_dir/oot3d-modern-hud.elf"

echo "Built $output_dir/code.ips (HUD scale: $hud_scale%)"
