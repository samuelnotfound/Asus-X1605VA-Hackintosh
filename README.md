# ASUS Vivobook X1605VA Hackintosh

> **Development snapshot.** This EFI is not ready for installation or daily use.

Both ASUS Vivobook X1605VA variants below were tested on **macOS Tahoe 26.7.2** with **OpenCore 1.0.8**.

## Hardware

| Component | Installed hardware |
| --- | --- |
| Motherboard | ASUS X1605VA |
| CPU | Intel Core i5-13500H, 12 cores and 16 threads, or Intel Core i9-13900H, 14 cores and 20 threads |
| Graphics | Intel Iris Xe Graphics (`8086:a7a0`) |
| Display | Internal `CMN1618` panel, 1920 × 1200 at 60 Hz |
| Wi-Fi | MediaTek MT7902 (`14c3:7902`, or Realtek RTL8821CE (`10ec:c821`); |
| Bluetooth | MediaTek MT7902 (`13d3:3579`, USB) |
| Audio | Realtek ALC256 (`10ec:0256`) |
| Trackpad | ASUS Precision Touchpad (`ASUE1306`) via Intel I²C (`8086:51e9`) |
| Webcam | USB 2.0 HD UVC webcam (`3277:0036`) |
| Fingerprint reader | ELAN (`04f3:0c90`, USB) |
| USB controller | Intel xHCI (`8086:51ed`) |



## Graphics acceleration

Intel Xe acceleration requires the macOS graphics patches in addition to the EFI. [Graphics setup](docs/ASUS-GRAPHICS.md)

The parent project maintains the Iris Xe patches. Follow its guide for graphics updates. [Upstream graphics updates](https://github.com/JulioNectar/Hackintosh-XPS13Plus-IrisXe-Tahoe/blob/main/docs/INSTALL.md#updating-an-existing-install) · [Verify acceleration after reboot](docs/INSTALL.md#after-the-first-accelerated-boot)

## Hardware status

| Feature | Status |
| --- | --- |
| Speakers | Working with AppleALC recompiled for the X1605VA. [AppleALC fork](https://github.com/samuelnotfound/AppleALC) |
| Internal microphone | Working |
| Trackpad in macOS | Working with the patched VoodooI2C beta and VoodooI2CHID. [VoodooI2C fork](https://github.com/samuelnotfound/VoodooI2C) |
| Trackpad in the OpenCore picker | Working with the custom ASUS UEFI driver. [Picker driver source](src/AsusX1605VATouchpadDxe) |
| Battery and FN keys | Working |
| Screen brightness | Adjustable with the XeShim utility. |
| Wi-Fi | Unsupported on MediaTek. Working on Realtek |
| Bluetooth | Working on MediaTek. Realtek Bluetooth is untested |
| Sleep and wake | Untested |


[Hardware notes](docs/ASUS-HARDWARE.md) · [Display tools](docs/INSTALL.md#after-the-first-accelerated-boot)

## Custom driver sources

The submodules pin the ASUS-specific driver revisions used for this setup.

| Source | Branch | Pinned commit |
| --- | --- | --- |
| [AppleALC](Sources/AppleALC) | x1605va-layout26-only | bb776a8 |
| [VoodooI2C](Sources/VoodooI2C) | master | 5a5bd0d |

To populate the source folders after cloning, run the following command.

    git submodule update --init --recursive

Git records the exact commits. Later fork updates take effect after updating the pins.

The small [F9 mic-mute LED kext](Extras/AsusMicMuteLED) version 0.1.1 is included and enabled for staged testing in the MediaTek config. Run its descriptor probe before requesting an LED write. Hardware verification is pending. Automatic synchronization with microphone mute remains to be implemented.

## Credits

- ASUS EFI baseline. [Heitorpoggi’s Vivobook project](https://github.com/Heitorpoggi/ASUS-VivoBook-16X-Hackintosh-)
- Testing the i9-13900H configuration. [MrSenaru](https://github.com/MrSenaru)
- OpenCore and bundled kexts including Lilu, WhateverGreen, VirtualSMC, AppleALC, VoodooPS2, NVMeFix, and RestrictEvents. [Acidanthera and contributors](https://github.com/acidanthera)
- Iris Xe graphics patches and tooling. [JulioNectar’s patch guide](https://github.com/JulioNectar/Hackintosh-XPS13Plus-IrisXe-Tahoe/blob/main/docs/INSTALL.md) · [NootedGreen](https://github.com/sgiammori/NootedGreen)
- Bundled VoodooI2C and USBToolBox kexts. [VoodooI2C contributors](https://github.com/VoodooI2C/VoodooI2C) · [USBToolBox contributors](https://github.com/USBToolBox/kext)
- Realtek Wi-Fi driver. [RTL88WiFi source at the bundled revision](https://github.com/X1REN41L/RTL88WiFi/tree/8cebe79209d05e8760e17603af6a344941920381) · [Driver license](licenses/RTL88WiFi-GPL-2.0.txt) · [Driver notices](licenses/RTL88WiFi-NOTICE.md)
- OpenAI Codex assisted with pair programming on this project.

Thanks to the authors of all bundled kexts. Additional credits for the inherited patches and tools are listed in the parent project. [Parent project credits](https://github.com/JulioNectar/Hackintosh-XPS13Plus-IrisXe-Tahoe#credits)

[Repository license](LICENSE) · [Component notices](NOTICE.md) · [Português](README.pt-BR.md)
