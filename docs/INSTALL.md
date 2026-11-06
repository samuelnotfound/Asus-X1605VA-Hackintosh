# ASUS Vivobook X1605VA install notes

Start here once macOS Tahoe is installed and boots through your ASUS OpenCore EFI. [Upstream installation guide](https://github.com/JulioNectar/Hackintosh-XPS13Plus-IrisXe-Tahoe/blob/main/docs/INSTALL.md)

## Install or update the macOS graphics patches

Open Terminal in macOS. If you need Xcode Command Line Tools, run `xcode-select --install` and wait for it to finish. Check your macOS build with `sw_vers -buildVersion` before applying the patches. [Graphics compatibility notes](ASUS-GRAPHICS.md)

### First install

Clone the parent graphics project to an APFS volume and enter its checkout. This example uses your home directory.

```sh
cd ~
git clone https://github.com/JulioNectar/Hackintosh-XPS13Plus-IrisXe-Tahoe.git
cd Hackintosh-XPS13Plus-IrisXe-Tahoe
```

### Build and install

Run the following from inside the parent graphics checkout. Use your ASUS OpenCore EFI to boot the laptop.

1. **Build the graphics patches.** This downloads the dependencies and generates the patched graphics components, `build/NootedGreen.kext`, and `build/HookCase.kext`.

   ```sh
   zsh install.sh --build-only
   ```

   The parent script checks the macOS build even in build-only mode. If you have verified compatibility for your build and the script stops at that check, retry with `XE_FORCE=1 zsh install.sh --build-only`.

2. **Install the macOS graphics components.** This command installs `HookCase.kext` in `/Library/Extensions`. **Do not install HookCase in the OpenCore EFI.**

   ```sh
   sudo zsh macos/install-macos.sh \
     build/deps/NootedGreen/sle_Internal/sle \
     build/deps/NootedGreen/sle_Internal/le
   ```

3. **Copy `NootedGreen.kext` to the EFI only.** Mount the EFI partition you boot from, back up its existing `NootedGreen.kext`, then replace `EFI/OC/Kexts/NootedGreen.kext` on that partition with the bundle from the repo’s `build/` folder. Both ASUS profiles already enable it in `Kernel > Add`.
4. Approve the kernel extensions if macOS prompts, then reboot through that EFI and [verify acceleration](#after-the-first-accelerated-boot).

### Updating an existing graphics install

Follow the parent project’s update instructions for the latest graphics patches. Keep your ASUS EFI profile and place the updated `NootedGreen.kext` in `EFI/OC/Kexts/` on the EFI you boot from. [Upstream graphics updates](https://github.com/JulioNectar/Hackintosh-XPS13Plus-IrisXe-Tahoe/blob/main/docs/INSTALL.md#updating-an-existing-install)

## After the first accelerated boot

From the parent graphics checkout in macOS, build the display utilities with the Command Line Tools.

```sh
mkdir -p tools/build
clang -O2 -o tools/build/xe-res tools/xe-res.c \
  -framework CoreGraphics -framework CoreFoundation
clang -O2 -fobjc-arc -DXECTL_NO_MAIN -o tools/build/xe-brilho \
  tools/xe-brilho.m tools/xectl.c -framework AppKit -framework IOKit
```

Run `kmutil showloaded | grep NootedGreen` to confirm the EFI kext loaded and `system_profiler SPDisplaysDataType` to check Metal support. Run `tools/build/xe-res` to list display modes. For example, `tools/build/xe-res w1680` selects a matching HiDPI mode if available.

With a compatible XeShim loaded, `tools/build/xe-brilho 70` requests 70% screen brightness. To build XeShim from source, run `zsh tools/build-kext.sh XeShim`.

## Moving OpenCore to the internal disk

1. Use `diskutil list internal` to identify the internal EFI System Partition. Mount that partition with `sudo diskutil mount diskXsY` using its actual identifier.
2. Back up the existing internal EFI. Copy `EFI/OC` from the USB that successfully boots this laptop to the internal EFI partition. Preserve existing Windows and other bootloader directories. If the USB uses a verified OpenCore fallback loader and the target fallback path is available, copy that loader as well.
3. In ASUS UEFI, select the internal OpenCore boot entry. If the firmware offers a file picker to add an entry, point it to `\EFI\OC\OpenCore.efi`. Test the internal entry while keeping the USB available for recovery.
4. If starting from the public config instead of your working USB, populate your own SMBIOS identity in `PlatformInfo > Generic` before booting.
