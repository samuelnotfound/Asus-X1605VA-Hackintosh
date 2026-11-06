# ASUS X1605VA UEFI Touchpad — Beta

This x64 UEFI boot driver exposes the built-in ELAN `04f3:3284` touchpad to the OpenCore picker through UEFI Simple Pointer. It matches the firmware's I²C child for `ASUE1306`, reads relative movement and physical button reports, and polls every 20 ms. The diagnostic build of this driver worked in the picker on the X1605VA. This stripped-down beta needs its own hardware verification.

Build with Zig 0.14.1 from this directory:

```sh
zig build-exe AsusX1605VATouchpadDxe.zig -target x86_64-uefi-msvc --subsystem efi_boot_service_driver -O ReleaseSafe -femit-bin=AsusX1605VATouchpadDxe-Beta.efi
```

Copy the resulting `AsusX1605VATouchpadDxe-Beta.efi` to `EFI/OC/Drivers` on a test EFI. Add or replace its entry under `UEFI → Drivers` with `Enabled = true` and `LoadEarly = false`. Boot into the OpenCanopy picker and check cursor movement and physical clicks.

macOS uses a separate driver stack, VoodooI2C and VoodooI2CHID.
