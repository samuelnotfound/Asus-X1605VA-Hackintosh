# ASUS Vivobook X1605VA Hackintosh

> **Development snapshot.** This EFI is not ready for installation or daily use.

Both ASUS Vivobook X1605VA variants below were tested on **macOS Tahoe 26.7.2** with **OpenCore 1.0.8**. Both use Intel Iris Xe graphics (`8086:A7A0`).

## Choose an EFI profile

| Profile | Tested CPU | Wi-Fi adapter | Wi-Fi support |
| --- | --- | --- | --- |
| [`config-realtek.plist`](EFI/OC/config-realtek.plist) | Intel Core i9-13900H | Realtek RTL8821CE (`10ec:c821`) | RTL88WiFi |
| [`config-mediatek.plist`](EFI/OC/config-mediatek.plist) | Intel Core i5-13500H | MediaTek MT7902 (`14c3:7902`) | Currently unsupported |

Copy `EFI/` to your boot drive’s EFI partition. Choose the profile that matches your Wi-Fi adapter and save a copy as `EFI/OC/config.plist` on that drive. [Setup notes](docs/INSTALL.md)

## Graphics acceleration

Iris Xe acceleration requires the macOS graphics patches in addition to the EFI. [Graphics setup](docs/ASUS-GRAPHICS.md)

To update the graphics patches, follow the ASUS install guide. [Install or update the graphics patches](docs/INSTALL.md#install-or-update-the-macos-graphics-patches) · [Verify acceleration after reboot](docs/INSTALL.md#after-the-first-accelerated-boot)

## Hardware status

| Feature | Status |
| --- | --- |
| Speakers | Working with AppleALC recompiled for the X1605VA. [AppleALC fork](https://github.com/samuelnotfound/AppleALC) |
| Internal microphone | Working |
| Trackpad | Unresponsive |
| USB mouse, battery and keyboard backlight | Working |
| Screen brightness | Adjustable with the XeShim utility. Brightness Fn keys are unresolved |
| Bluetooth | Working on MediaTek. Realtek Bluetooth is untested |
| Sleep and wake | Untested |

[Hardware notes](docs/ASUS-HARDWARE.md) · [Display tools](docs/INSTALL.md#after-the-first-accelerated-boot)

## Credits

- ASUS EFI baseline. [Heitorpoggi’s Vivobook project](https://github.com/Heitorpoggi/ASUS-VivoBook-16X-Hackintosh-)
- OpenCore and bundled kexts including Lilu, WhateverGreen, VirtualSMC, AppleALC, VoodooPS2, NVMeFix, and RestrictEvents. [Acidanthera and contributors](https://github.com/acidanthera)
- Iris Xe graphics patches and tooling. [JulioNectar’s patch guide](https://github.com/JulioNectar/Hackintosh-XPS13Plus-IrisXe-Tahoe/blob/main/docs/INSTALL.md) · [NootedGreen](https://github.com/sgiammori/NootedGreen)
- Bundled VoodooI2C and USBToolBox kexts. [VoodooI2C contributors](https://github.com/VoodooI2C/VoodooI2C) · [USBToolBox contributors](https://github.com/USBToolBox/kext)
- Realtek Wi-Fi driver. [RTL88WiFi source at the bundled revision](https://github.com/X1REN41L/RTL88WiFi/tree/8cebe79209d05e8760e17603af6a344941920381) · [Driver license](licenses/RTL88WiFi-GPL-2.0.txt) · [Driver notices](licenses/RTL88WiFi-NOTICE.md)
- OpenAI Codex assisted with pair programming on this project.

Thanks to the authors of all bundled kexts. Additional credits for the inherited patches and tools are listed in the parent project. [Parent project credits](https://github.com/JulioNectar/Hackintosh-XPS13Plus-IrisXe-Tahoe#credits)

[Repository license](LICENSE) · [Component notices](NOTICE.md) · [Português](README.pt-BR.md)
