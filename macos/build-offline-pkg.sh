#!/bin/zsh
# Build an offline graphics installer on an Intel macOS build host.
set -euo pipefail
ROOT=${0:A:h:h}
[[ $(uname -s) == Darwin && $(uname -m) == x86_64 ]] || {
  print -u2 'An Intel macOS build host is required.'
  exit 1
}

zsh "$ROOT/install.sh" --build-only

INPUT="$ROOT/build/deps/NootedGreen/sle_Internal"
DIST="$ROOT/dist"
mkdir -p "$DIST"
STAGE=$(mktemp -d "$DIST/.graphics-pkg.XXXXXXXX")
trap 'rm -rf -- "$STAGE"' EXIT
SCRIPTS="$STAGE/scripts"
mkdir -p "$SCRIPTS"/{le,sle,macos/out,build}

for bundle in AppleIntelTGLGraphics AppleIntelTGLGraphicsFramebuffer; do
  ditto "$INPUT/le/$bundle.kext" "$SCRIPTS/le/$bundle.kext"
done
for bundle in AppleIntelTGLGraphicsMTLDriver AppleIntelTGLGraphicsGLDriver AppleIntelTGLGraphicsVADriver AppleIntelTGLGraphicsVAME AppleIntelGraphicsShared; do
  ditto "$INPUT/sle/$bundle.bundle" "$SCRIPTS/sle/$bundle.bundle"
done
if [[ -d "$INPUT/le/HookCase.kext" ]]; then
  ditto "$INPUT/le/HookCase.kext" "$SCRIPTS/le/HookCase.kext"
else
  ditto "$ROOT/build/HookCase.kext" "$SCRIPTS/build/HookCase.kext"
fi
for bundle in AppleIntelTGLGraphicsMTLDriver xe; do
  ditto "$ROOT/macos/out/$bundle.bundle" "$SCRIPTS/macos/out/$bundle.bundle"
done
cp "$ROOT/macos/install-macos.sh" "$SCRIPTS/macos/"
cp "$ROOT/macos/pkg-scripts/"{preinstall,postinstall} "$SCRIPTS/"
chmod 755 "$SCRIPTS/"{preinstall,postinstall}

# The parent installer places its bundles and registers kexts at runtime.
PACKAGE=ASUS-X1605VA-IrisXe-Offline.pkg
pkgbuild --nopayload --scripts "$SCRIPTS" \
  --identifier io.github.samuelnotfound.x1605va.irisxe --version 1.0 "$STAGE/$PACKAGE"
pkgutil --expand "$STAGE/$PACKAGE" "$STAGE/expanded"
[[ -s "$STAGE/expanded/Scripts" ]]
mv -f "$STAGE/$PACKAGE" "$DIST/$PACKAGE"
print -r -- "$DIST/$PACKAGE"
