# Experimental MT7902 Wi-Fi for the ASUS X1605VA

This port targets the internal MediaTek PCIe adapter `14c3:7902` on Intel
macOS Tahoe. It uses an Ethernet interface for IP traffic and the companion
menu app for Wi-Fi selection. The app and kext filenames retain the upstream
MT7922 names. The kext bundle ID is `local.x1605va.MT7902WiFi` and its PCI
personality matches only MT7902. Association, DHCP, and traffic on the target
hardware still require verification.

## Build or download

Get the `MT7902-Tahoe-test` artifact from the experimental GitHub Actions run.
Extract the artifact ZIP and then open its enclosed `MT7902-Tahoe-test.zip`
on macOS to preserve the executable permissions. To build the same source on
macOS with Xcode command-line tools, run `bash tools/build-mt7902-wifi.sh` from
the repository root. The script prints the paths to the kext and app.
From the extracted offline source bundle, run
`MT7902_ARCHIVES="$PWD/archives" bash tools/build-mt7902-wifi.sh`.

## Test on Tahoe

1. Copy your working MediaTek EFI to a test boot drive. In that copy, put
   `itlmt7922.kext` under `EFI/OC/Kexts`. Add an enabled OpenCore
   `Kernel > Add` entry with `BundlePath` set to `itlmt7922.kext`,
   `ExecutablePath` set to `Contents/MacOS/itlmt7922`, and `PlistPath` set to
   `Contents/Info.plist`. Use `Arch=x86_64` and `MinKernel=25.0.0`. Keep the
   existing Bluetooth entry.
2. Boot the test EFI with `mt7902_stage=1` in `boot-args`. Capture the
   output below. Look for `"firmware-ready" = Yes` and
   `"firmware-result" = 0`. If the firmware fails, share the `m0-log` and
   `m1-log` properties before advancing.
3. Change the test EFI to `mt7902_stage=2` and reboot. Look for
   `"scan-result" = 0` and access points in the `m2-log`. Share `m2-log` if
   the scan fails.
4. With a successful scan, boot with `mt7902_stage=3`, open
   `ItlWiFi7922.app`, and join a WPA2-Personal network. Leave HT/VHT at its
   default of off for the first connection. Use
   `networksetup -listallhardwareports` to identify the driver interface,
   then run `ipconfig getifaddr enX` and
   `curl --interface enX https://example.com/`, substituting the actual
   interface. Record whether association, DHCP and traffic each succeed.
   Keep the menu app's password-saving option off during the test. Its
   upstream storage is a local plaintext file.

```sh
sw_vers
kmutil showloaded | grep -F local.x1605va.MT7902WiFi
ioreg -r -c itlmt7922 -l -w0 | sed -E 's/("itl_psk[0-9]*" = ).*/\1"<redacted>"/'
```

## Driver references and REA observations

- REA 6.8.0 with Rizin 0.9.1 inspected the two shipped x86_64 Mach-O files
  with `execute-rizin-command` and `ij`, `isj`, and `pdj`. The Intel
  Tahoe binary has `AirportItlwm::apple80211SkywalkRequest` at `0x34170`.
  The MT7922 binary has `dmaEnable` at `0x4730`, `mcuSend` at `0x4a80`,
  `m1Run` at `0x58b0`, and `m2Scan` at `0x6e80`. The DMA
  disassembly shows the Connac2 register writes at the RX and TX prefetch
  bases `0xd4680` and `0xd4600`. The Intel binary was used to identify the
  native Tahoe integration boundary. The MT7922 source supplies the reused
  Ethernet and WPA2 paths. [REA](https://github.com/morluto/rea) ·
  [AirportItlwm Tahoe](https://github.com/laobamac/itlwm/releases/tag/v2.4.0-alpha) ·
  [MT7922 release](https://github.com/hbyonly1/itlmt7922-wifi/releases/tag/v0.2.2)
- The port programs MT7902 MCU TX ring 15, firmware TX ring 16, shared RX
  event ring 0 with 512 entries, and the chip's WFDMA prefetch layout. It
  retains the Connac2 MCU restart and power-state check before patch
  download, the hardware TXP layout, and the PSE read after each power
  table batch. [Linux MT7902 PCIe setup](https://github.com/openwrt/mt76/blob/66067d203bd04b703b55341bbf986ead5db05653/mt7921/pci.c) ·
  [Linux WFDMA prefetch](https://github.com/openwrt/mt76/blob/66067d203bd04b703b55341bbf986ead5db05653/mt792x_dma.c)
- The build pins the macOS base and checks the ROM patch and RAM firmware
  against their published SHA-256 hashes. [Pinned macOS base](https://github.com/hbyonly1/itlmt7922-wifi/tree/20aaac5c8e1d97d737a73f1a684ded2ec1d0f940) ·
  [MediaTek firmware provenance](https://github.com/KingAkeem/mt7902/tree/1b0294eb44457b7cb6d94dae0d0cc5bc87e71b89/firmware)
