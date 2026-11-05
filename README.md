# ASUS Vivobook X1605VA OpenCore EFI

Experimental OpenCore 1.0.8 configuration for the ASUS Vivobook X1605VA with an Intel Core i5-13500H and Iris Xe graphics (`8086:A7A0`). The system was tested on macOS Tahoe 26.7.2 (`25G332`). The EFI includes the Golden Gate picker theme.

The public `EFI/OC/config.plist` retains the `MacBookPro16,2` SMBIOS model. Fill in your own serial number, MLB, UUID and ROM before booting the public config.

| Function | Status |
| --- | --- |
| Iris Xe | Acceleration reported with the macOS-side graphics patches installed |
| Speakers | Working with AppleALC, SimpleHDA and layout 5 |
| Internal microphone | Input appears, but recording stalls |
| Bluetooth | MT7902 firmware loader included. BlueToolFixup 2.7.2 restored after a reboot regression. Retest pending |
| Wi-Fi | Experimental MT7902 driver 0.2.14 included. Retest pending |
| Trackpad | Unresolved |
| USB mouse, battery and keyboard lighting | Working |
| Screen brightness | XeShim utility included. Fn brightness control unresolved |
| Sleep and wake | Untested |

Use [install steps 6 and 7](docs/INSTALL.md) for the display tools and moving OpenCore to the internal disk. Graphics acceleration also requires the macOS-side driver and patches. See the [graphics notes](docs/ASUS-GRAPHICS.md) and [upstream patch guide](https://github.com/JulioNectar/Hackintosh-XPS13Plus-IrisXe-Tahoe/blob/main/docs/INSTALL.md).

See the [hardware notes](docs/ASUS-HARDWARE.md) and the [microphone routing assessment](docs/RPL_PORT.md).

## Credits and licensing

- [Heitorpoggi/ASUS-VivoBook-16X-Hackintosh-](https://github.com/Heitorpoggi/ASUS-VivoBook-16X-Hackintosh-): origin of the ASUS EFI baseline.
- [JulioNectar/Hackintosh-XPS13Plus-IrisXe-Tahoe](https://github.com/JulioNectar/Hackintosh-XPS13Plus-IrisXe-Tahoe): parent fork, Iris Xe patch stack and inherited tools/documentation.
- [NootedGreen](https://github.com/sgiammori/NootedGreen), OpenCore/acidanthera, VoodooI2C, VoodooPS2, USBToolBox and the bundled kext authors.

Existing [LICENSE](LICENSE), [NOTICE](NOTICE.md) and component licenses remain applicable. This fork does not claim authorship of the upstream graphics work.
