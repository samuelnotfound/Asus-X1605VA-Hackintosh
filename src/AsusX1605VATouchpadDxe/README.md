# ASUS X1605VA UEFI Touchpad — Beta

This x64 UEFI boot driver gives the OpenCore picker pointer input from the built-in ELAN `04f3:3284` touchpad. It matches the firmware's I²C child for `ASUE1306` and exposes relative movement and physical button reports through UEFI Simple Pointer. The bundled beta worked in OpenCanopy on the tested X1605VA.

Build with Zig 0.14.1 from this directory.

```sh
zig build-exe AsusX1605VATouchpadDxe.zig -target x86_64-uefi-msvc --subsystem efi_boot_service_driver -O ReleaseSafe -femit-bin=AsusX1605VATouchpadDxe-Beta.efi
```

Copy the resulting `AsusX1605VATouchpadDxe-Beta.efi` to `EFI/OC/Drivers` on a test EFI. Add or replace its entry under `UEFI → Drivers` with `Enabled = true` and `LoadEarly = true`. On the tested X1605VA, early loading was required for automatic input in OpenCanopy. OpenCore 1.0.8's `ocvalidate` flags this setting for the custom driver. Check cursor movement and physical clicks after a direct boot into the picker.

macOS uses VoodooI2CHID with the patched [VoodooI2C fork](https://github.com/samuelnotfound/VoodooI2C).
