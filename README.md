# ASUS Vivobook X1605VA — experimental Hackintosh EFI

**Development snapshot only. Not ready for installation or daily use.** 

## Hardware

| Component | This laptop |
| --- | --- |
| Model | ASUS Vivobook X1605VA |
| CPU | Intel Core i5-13500H |
| Integrated GPU | Intel Iris Xe, PCI `8086:A7A0` |
| Graphics topology settings | 80 EU / 6 L3 banks, preserved from the supplied EFI |
| SMBIOS template | `MacBookPro16,2`; private identity fields removed |
| macOS | Installed version/build not yet recorded; inherited patch scripts target Tahoe 26.7.1 (`25G241`) |

Model, CPU and GPU ID were read from Windows on this laptop. RAM, storage, panel, wireless hardware and touchpad ACPI identity still need an inventory. GPU topology settings were preserved, not inferred from the PCI ID alone.

## Current status

| Function | Status on this ASUS |
| --- | --- |
| Graphics acceleration | **Working**   |
| Wi-Fi |   No driver (yet)|
| Bluetooth | Worked once. Fails again after reboot |
| Trackpad | **Not working** |
| Camera | **Working** |
| Speaker | **Working** |
| Microphone | Built-in input appears but recording stalls. See [audio investigation](docs/RPL_PORT.md) |
| Fn keys | **Not all keys were tested**, volume and keyboard baclight works |
| Sleep / wake | **Untested** |
| Brightness control|  tools/xe-shim  |
| External displays and other functions | **Unverified** |

Kext presence does not establish that a device works. Dell hardware results in inherited documentation do not describe this ASUS.

Read the [graphics notes](docs/ASUS-GRAPHICS.md), and [remaining hardware work](docs/ASUS-HARDWARE.md).

See [install steps 6 and 7](docs/INSTALL.md) for the display utilities and moving OpenCore to the internal disk.

## Graphics patch source

Graphics must follow the parent project's [installation and patch guide](https://github.com/JulioNectar/Hackintosh-XPS13Plus-IrisXe-Tahoe/blob/main/docs/INSTALL.md), especially its kernel and user-space sections. Acceleration requires macOS-side driver bundles and patches as well as the EFI.


## Credits and licensing

- [Heitorpoggi/ASUS-VivoBook-16X-Hackintosh-](https://github.com/Heitorpoggi/ASUS-VivoBook-16X-Hackintosh-): origin of the ASUS EFI baseline.
- [JulioNectar/Hackintosh-XPS13Plus-IrisXe-Tahoe](https://github.com/JulioNectar/Hackintosh-XPS13Plus-IrisXe-Tahoe): parent fork, Iris Xe patch stack and inherited tools/documentation.
- [NootedGreen](https://github.com/sgiammori/NootedGreen), OpenCore/acidanthera, VoodooI2C, VoodooPS2, USBToolBox and the bundled kext authors.

Existing [LICENSE](LICENSE), [NOTICE](NOTICE.md) and component licenses remain applicable. This fork does not claim authorship of the upstream graphics work.
