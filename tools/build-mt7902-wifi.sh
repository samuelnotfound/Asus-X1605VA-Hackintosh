#!/bin/bash
set -euo pipefail

if [[ "$(uname -s)" != Darwin ]]; then
    echo "Run this build on macOS with the Xcode command-line tools." >&2
    exit 1
fi

root="$(cd "$(dirname "$0")/.." && pwd)"
port_rev=20aaac5c8e1d97d737a73f1a684ded2ec1d0f940
firmware_mirror_rev=1b0294eb44457b7cb6d94dae0d0cc5bc87e71b89
kernel_sdk_rev=3f750085caa17ec3a7880f11c11bf4f48cd6a164
mkdir -p "$root/build"
work="$(mktemp -d "$root/build/mt7902-wifi.XXXXXX")"
echo "Build directory: $work"

fetch_archive() {
    local archive="$work/$3"
    if [[ -n "${MT7902_ARCHIVES:-}" ]]; then
        archive="$MT7902_ARCHIVES/$3"
    else
        curl --fail --location --silent --show-error --retry 2 \
            "https://codeload.github.com/$1/tar.gz/$2" -o "$archive"
    fi
    tar -xzf "$archive" -C "$work"
}

fetch_archive hbyonly1/itlmt7922-wifi "$port_rev" port.tar.gz
fetch_archive KingAkeem/mt7902 "$firmware_mirror_rev" firmware.tar.gz
fetch_archive acidanthera/MacKernelSDK "$kernel_sdk_rev" sdk.tar.gz

cd "$work/itlmt7922-wifi-$port_rev"
git init --quiet
git apply "$root/patches/mt7902-wifi-connac2.patch"
firmware="$work/mt7902-$firmware_mirror_rev/firmware"
cp "$firmware"/WIFI_*MT7902*.bin "$firmware/LICENCE.mediatek" fw/
cat > fw/SHA256SUMS <<'EOF'
b5958ac72c71fb8405e080f52d378d02b484812b9f1010c8843727056bbfc998  WIFI_RAM_CODE_MT7902_1.bin
d73ba9e982f781221a2b9f10c42031f20a9dce046929fc0d55c791a417efe30a  WIFI_MT7902_patch_mcu_1_1_hdr.bin
EOF

MACKERNELSDK="$work/MacKernelSDK-$kernel_sdk_rev" bash scripts/build.sh
echo "Kext: $PWD/build/itlmt7922.kext"
echo "Menu app: $PWD/build/ItlWiFi7922.app"
