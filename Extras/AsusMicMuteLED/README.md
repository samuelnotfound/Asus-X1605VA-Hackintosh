# AsusMicMuteLED

Version 0.1.0 froze macOS when an on/off command was issued on this X1605VA. Its EFI entry is disabled pending diagnosis.

Manual F9 microphone-mute LED control for ASUS Vivobook X1605VA. This driver uses ATKD WMNB device 0x00040017 and checks its presence before writing. Root permission is required.

Run the tool in macOS after loading the kext.

    sudo ./micled status
    sudo ./micled on
    sudo ./micled off

This version controls the LED independently of microphone audio. Confirm the physical light before adding audio-mute synchronization.

Build with bash Extras/AsusMicMuteLED/build.sh. Output is in the out folder beside the source.

For OpenCore use executable Contents/MacOS/AsusMicMuteLED, plist Contents/Info.plist and architecture x86_64. Disable the Kernel/Add entry to roll back.

The WMNB calling convention follows AsusSMC and this laptop's ACPI dump. Firmware calls occur only on tool requests.
