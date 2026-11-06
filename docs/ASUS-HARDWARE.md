# ASUS X1605VA hardware notes

## Trackpad

The ASUS trackpad is `ASUE1306` / `PNP0C50` at `_SB.PC00.I2C1.ETPD`, under Intel controller `8086:51E9`. VoodooI2C and VoodooI2CHID are enabled with polling. The trackpad remains unresponsive.

## Bluetooth and Wi-Fi

MediaTek MT7902 Bluetooth (`13D3:3579`) works with MT7902Bt and BlueToolFixup.

MediaTek MT7902 Wi-Fi is currently unsupported.

## Audio

The Realtek ALC256 speakers work with the recompiled AppleALC and SimpleHDA. The internal microphone now records. [AppleALC fork](https://github.com/samuelnotfound/AppleALC)

## Brightness

Keyboard lighting works. Use the XeShim brightness utility for screen brightness. Screen-brightness Fn keys remain unresolved. [Display tools](INSTALL.md#after-the-first-accelerated-boot)
