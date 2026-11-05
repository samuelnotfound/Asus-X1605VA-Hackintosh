# ASUS Vivobook X1605VA Hackintosh

Both ASUS Vivobook X1605VA variants below were tested on **macOS Tahoe 26.7.2** with **OpenCore 1.0.8**. Both use Intel Iris Xe graphics (`8086:A7A0`).

## Choose an EFI profile

| Profile | Tested CPU | Wi-Fi adapter | Wi-Fi support |
| --- | --- | --- | --- |
| [`config-realtek.plist`](EFI/OC/config-realtek.plist) | Intel Core i9-13900H | Realtek RTL8821CE (`10ec:c821`) | RTL88WiFi |
| [`config-mediatek.plist`](EFI/OC/config-mediatek.plist) | Intel Core i5-13500H | MediaTek MT7902 (`14c3:7902`) | Currently unsupported |

Copy `EFI/` to your boot drive’s EFI partition. Choose the profile that matches your Wi-Fi adapter and save a copy as `EFI/OC/config.plist` on that drive. [Setup notes](docs/INSTALL.md)

## Graphics acceleration

Iris Xe acceleration requires the macOS graphics patches in addition to the EFI. GitHub Actions can build an offline `.pkg` that installs those patches without internet access or Xcode Command Line Tools on the target laptop. [Graphics setup](docs/ASUS-GRAPHICS.md) · [Build workflow](https://github.com/samuelnotfound/Asus-X1605VA-Hackintosh/actions/workflows/offline-graphics-pkg.yml)

## Hardware status

| Feature | Status |
| --- | --- |
| Speakers | Working with AppleALC recompiled for the X1605VA. [AppleALC fork](https://github.com/samuelnotfound/AppleALC) |
| Internal microphone | Appears as an input, but recording stalls |
| Trackpad | Unresponsive |
| USB mouse, battery and keyboard backlight | Working |
| Screen brightness | Adjustable with the XeShim utility. Brightness Fn keys are unresolved |
| Bluetooth | MediaTek stability needs retesting. Realtek Bluetooth is untested |
| Sleep and wake | Untested |

[Hardware notes](docs/ASUS-HARDWARE.md) · [Microphone routing](docs/RPL_PORT.md) · [Display tools](docs/INSTALL.md#6-after-the-first-accelerated-boot)

## Credits

- ASUS EFI baseline. [Heitorpoggi’s Vivobook project](https://github.com/Heitorpoggi/ASUS-VivoBook-16X-Hackintosh-)
- Iris Xe graphics patches and tooling. [JulioNectar’s patch guide](https://github.com/JulioNectar/Hackintosh-XPS13Plus-IrisXe-Tahoe/blob/main/docs/INSTALL.md) · [NootedGreen](https://github.com/sgiammori/NootedGreen)
- Realtek Wi-Fi driver. [RTL88WiFi source at the bundled revision](https://github.com/X1REN41L/RTL88WiFi/tree/8cebe79209d05e8760e17603af6a344941920381) · [Driver license](licenses/RTL88WiFi-GPL-2.0.txt) · [Driver notices](licenses/RTL88WiFi-NOTICE.md)

[Repository license](LICENSE) · [Component notices](NOTICE.md) · [Português](README.pt-BR.md)
