# ASUS X1605VA hardware work remaining

No hardware fixes or boot validation were performed while preparing this repository. These statuses apply to this ASUS only.

## Trackpad — not working

Retained VoodooI2C, VoodooI2CHID, I2C services/GPIO/input plugins, VoodooPS2Controller and the supplied `SSDT-GPI0.aml`. `-vi2c-force-polling` is preserved. Only the I2C copy of VoodooInput is explicitly enabled; the PS2 copy stays disabled.

Before changing ACPI or selecting a different satellite, collect the touchpad hardware ID and ACPI path from Windows Device Manager / an ACPI dump. Compare `_HID`, `_CID`, `_CRS`, `_STA`, interrupt routing and the I2C controller with macOS IORegistry and VoodooI2C logs. Retest one change at a time when testing resumes. The inherited `SSDT-XPS-I2C` source and `build-voodoogpio.sh` are Dell work, not verified ASUS fixes.

## Wi-Fi — not working, excluded


## Audio, microphone and Fn keys — not working

AppleALC, SimpleHDA, AsusSMC and BrightnessKeys retain their supplied flags. Their presence does not establish working audio, microphone or Fn keys. Collect codec/PCI IDs, routing and ASUS ACPI hotkey events before choosing fixes. The parent's Dell SoundWire audio driver is not a validated ASUS solution.

## Sleep / wake — untested


## Brightness — unconfirmed

The owner's brightness message was incomplete. Record slider behavior and brightness-key behavior separately when confirmed. Fn keys are reported not working.

