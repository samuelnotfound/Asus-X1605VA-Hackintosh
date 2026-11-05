# ASUS Vivobook X1605VA OpenCore EFI

Experimental OpenCore 1.0.8 configurations tested on both ASUS Vivobook X1605VA variants listed below. Both have Intel Iris Xe graphics (`8086:A7A0`). The i5-13500H MediaTek test used macOS Tahoe 26.7.2 (`25G332`). The EFI includes the Golden Gate picker theme.

Choose the template that matches your Wi-Fi hardware. Copy it to `EFI/OC/config.plist` on your boot drive before booting.

| Template | Tested X1605VA configuration |
| --- | --- |
| [`config-realtek.plist`](EFI/OC/config-realtek.plist) | Core i9-13900H with Realtek RTL8821CE (`10ec:c821`) |
| [`config-mediatek.plist`](EFI/OC/config-mediatek.plist) | Core i5-13500H with MediaTek MT7902 (`14c3:7902`) |

Both templates use the original AMFIPass and AppleALC from the author's OpenCore partition and retain `MacBookPro16,2`. Fill in your own serial number, MLB, UUID and ROM in the boot copy.

| Function | Status |
| --- | --- |
| Iris Xe | Acceleration reported with the macOS-side graphics patches installed |
| Speakers | Working with AppleALC, SimpleHDA and layout 5 |
| Internal microphone | Input appears, but recording stalls |
| Bluetooth | MediaTek config includes the MT7902 firmware loader and BlueToolFixup. Realtek Bluetooth is untested |
| Wi-Fi | The Realtek test config uses RTL88WiFi. The MediaTek config includes the experimental MT7902 driver 0.2.14 |
| Trackpad | Unresolved |
| USB mouse, battery and keyboard lighting | Working |
| Screen brightness | XeShim utility included. Fn brightness control unresolved |
| Sleep and wake | Untested |

Use [install steps 6 and 7](docs/INSTALL.md) for the display tools and moving OpenCore to the internal disk. Graphics acceleration also requires the macOS-side driver and patches. See the [graphics notes](docs/ASUS-GRAPHICS.md) and [upstream patch guide](https://github.com/JulioNectar/Hackintosh-XPS13Plus-IrisXe-Tahoe/blob/main/docs/INSTALL.md).

See the [hardware notes](docs/ASUS-HARDWARE.md) and the [microphone routing assessment](docs/RPL_PORT.md).

The Realtek config was tested on the i9-13900H X1605VA with RTL8821CE (`10ec:c821`). The upstream driver also lists RTL8821CE (`10ec:b821`), RTL8822BE (`10ec:b822`) and RTL8822CE (`10ec:c822`, `10ec:c82f`). [RTL88WiFi source at the bundled revision](https://github.com/X1REN41L/RTL88WiFi/tree/8cebe79209d05e8760e17603af6a344941920381) · [License](licenses/RTL88WiFi-GPL-2.0.txt) · [Notices](licenses/RTL88WiFi-NOTICE.md)

## Credits and licensing

- [Heitorpoggi/ASUS-VivoBook-16X-Hackintosh-](https://github.com/Heitorpoggi/ASUS-VivoBook-16X-Hackintosh-): origin of the ASUS EFI baseline.
- [JulioNectar/Hackintosh-XPS13Plus-IrisXe-Tahoe](https://github.com/JulioNectar/Hackintosh-XPS13Plus-IrisXe-Tahoe): parent fork, Iris Xe patch stack and inherited tools/documentation.
- [NootedGreen](https://github.com/sgiammori/NootedGreen), OpenCore/acidanthera, VoodooI2C, VoodooPS2, USBToolBox and the bundled kext authors.

Existing [LICENSE](LICENSE), [NOTICE](NOTICE.md) and component licenses remain applicable. This fork does not claim authorship of the upstream graphics work.
