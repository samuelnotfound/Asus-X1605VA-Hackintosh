# ASUS X1605VA hardware notes

## Trackpad

The i5 test unit has an ELAN trackpad identified as `ASUE1306` / `PNP0C50` at `_SB.PC00.I2C1.ETPD`, under Intel controller `8086:51e9`.

- **macOS** uses VoodooI2CHID with a VoodooI2C fork that can poll the I2C1 host controller when interrupt registration fails. The trackpad worked with the initial beta on the i5 unit. This EFI includes the smaller follow-up build, which still needs a hardware retest. Both profiles retain the touchpad's native interrupt. [Bundled VoodooI2C source](https://github.com/samuelnotfound/VoodooI2C/tree/5a5bd0da6a3ffa0365da1791b7acd0d9489c666b)
- **OpenCore picker** uses `AsusX1605VATouchpadDxe-Beta.efi` for pointer input before macOS loads. The tested setup requires `LoadEarly = true`. OpenCore 1.0.8's `ocvalidate` flags that setting on the custom driver. [Picker driver source and build instructions](../src/AsusX1605VATouchpadDxe)

## Bluetooth and Wi-Fi

MediaTek MT7902 Bluetooth (`13D3:3579`) works with MT7902Bt and BlueToolFixup.

MediaTek MT7902 Wi-Fi is currently unsupported.

## Audio

The Realtek ALC256 speakers work with the recompiled AppleALC and SimpleHDA. The internal microphone now records. [AppleALC fork](https://github.com/samuelnotfound/AppleALC)

## Brightness

Keyboard lighting works. Use the XeShim brightness utility for screen brightness. Screen-brightness Fn keys remain unresolved. [Display tools](INSTALL.md#after-the-first-accelerated-boot)
