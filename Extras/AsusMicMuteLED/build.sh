#!/bin/bash
set -euo pipefail
cd "$(dirname "$0")"
SDK=$(xcrun --sdk macosx --show-sdk-path)
K="$SDK/System/Library/Frameworks/Kernel.framework/Headers"
mkdir -p out/AsusMicMuteLED.kext/Contents/MacOS out/obj
FLAGS=(-arch x86_64 -mmacosx-version-min=11.0 -mkernel -nostdinc -fno-builtin -fno-common -fno-stack-protector -DKERNEL -DKERNEL_PRIVATE -DDRIVER_PRIVATE -DAPPLE -DNeXT -isystem "$(xcrun clang -print-resource-dir)/include" -I"$K" -O2 -Wall)
xcrun clang++ "${FLAGS[@]}" -fapple-kext -std=gnu++17 -nostdinc++ -fno-exceptions -fno-rtti -fno-use-cxa-atexit -c AsusMicMuteLED.cpp -o out/obj/AsusMicMuteLED.o
xcrun clang "${FLAGS[@]}" -c kmod_info.c -o out/obj/kmod_info.o
xcrun clang++ -arch x86_64 -mmacosx-version-min=11.0 -nostdlib -Xlinker -kext -Xlinker -export_dynamic out/obj/*.o -L"$SDK/usr/lib" -lkmodc++ -lkmod -lcc_kext -o out/AsusMicMuteLED.kext/Contents/MacOS/AsusMicMuteLED
cp Info.plist out/AsusMicMuteLED.kext/Contents/
xcrun clang -arch x86_64 -mmacosx-version-min=11.0 -O2 -Wall micled.c -framework IOKit -framework CoreFoundation -o out/micled
codesign --force --sign - out/AsusMicMuteLED.kext
codesign --verify --strict out/AsusMicMuteLED.kext
xcrun otool -hv out/AsusMicMuteLED.kext/Contents/MacOS/AsusMicMuteLED | grep KEXTBUNDLE
