# ASUS X1605VA hardware notes

## Trackpad

The ASUS trackpad is `ASUE1306` / `PNP0C50` at `_SB.PC00.I2C1.ETPD`, under Intel controller `8086:51E9`. VoodooI2C and VoodooI2CHID are enabled with polling. The trackpad remains unresponsive.

## Bluetooth and Wi-Fi

The MediaTek MT7902 Bluetooth function is USB `13D3:3579`. This EFI has MT7902Bt 0.1.0 and BlueToolFixup 2.7.2. Bluetooth worked previously and failed again after reboot with BlueToolFixup removed. This snapshot restores the companion kext and keeps the existing Bluetooth NVRAM values. Stability after the restoration still needs a macOS reboot.

The EFI includes the experimental [MT7902 Wi-Fi driver](https://github.com/samuelnotfound/itlmt7902-wifi). Connectivity was not retested for this snapshot.

## Audio

The Realtek ALC256 speakers work with AppleALC, SimpleHDA and layout 5. macOS lists a built-in microphone but recording stalls. The available evidence points to the Realtek capture path. [Microphone routing assessment](RPL_PORT.md)

## Brightness

Keyboard lighting works. Use the XeShim brightness utility for screen brightness. Screen-brightness Fn keys remain unresolved. [Install step 6](INSTALL.md#6-after-the-first-accelerated-boot)
