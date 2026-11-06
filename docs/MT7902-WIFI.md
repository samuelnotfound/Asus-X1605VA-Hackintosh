# Native MT7902 Wi-Fi for the ASUS X1605VA

This experimental driver targets the internal MediaTek PCIe adapter `14c3:7902` on
Intel macOS Tahoe. The driver starts the radio and publishes an IO80211 station
interface for the built-in Wi-Fi menu. The two kexts communicate in the kernel.
The first connection test targets WPA2-Personal with AES-CCMP.

## Build and install

Download the `MT7902-Tahoe-native` GitHub Actions artifact and open the enclosed
`MT7902-Tahoe-native.zip` on macOS. To build locally with the Xcode command-line
tools, run `bash tools/build-mt7902-wifi.sh` from the repository root.
For an offline build, set `MT7902_ARCHIVES` to the directory containing the
four pinned archives `port.tar.gz`, `native.tar.gz`, `firmware.tar.gz`, and
`sdk.tar.gz`.

Use a copy of your working EFI and add both entries under OpenCore
`Kernel > Add`, in the order shown. Set `Arch` to `x86_64`, `Enabled` to
`true`, and `PlistPath` to `Contents/Info.plist` for each entry.

| BundlePath | ExecutablePath |
| --- | --- |
| `itlmt7922.kext` | `Contents/MacOS/itlmt7922` |
| `itlmt7902Native.kext` | `Contents/MacOS/itlmt7902Native` |

Replace any earlier MT7902 radio bundle with this package's copy. Remove
`mt7902_stage` from the test EFI boot arguments. Keep Apple's Tahoe IO80211Family
and IOSkywalkFamily enabled. This package targets Tahoe 26.7.2 build `25G332`
on the X1605VA. WTLKcAbi compared that build with `25G241`, the source of
the private headers, and found matching class sizes, virtual method tables,
and ioctl dispatch tables. Set both `MinKernel` and `MaxKernel` to
`25.6.0` and check the build with `sw_vers` before loading the frontend.
[Tahoe ABI comparison](https://github.com/watchthelight-labs/WTLKcAbi#status)

Boot once, select your WPA2 network from macOS Wi-Fi, and verify that the Wi-Fi
interface receives an IP address and passes traffic. To diagnose startup or
matching, run the following in Terminal.

```sh
sw_vers
kmutil showloaded | grep -E 'local\.x1605va\.MT7902(WiFi|Native)'
ioreg -r -c itlmt7922 -d 1 -l -w0 | grep -E 'firmware-result|scan-result|mt7902-native-ready|wifi-state|m[12]-log'
ioreg -r -c itlmt7925WifiInterface -d 1
networksetup -listallhardwareports
```

The radio sets `mt7902-native-ready` after firmware startup and initial scan.
The frontend matches on that property and gets the same station address the
radio uses on air. Build checks cover the two kext bundles and PCI personality.
Association, DHCP, and traffic on the MT7902 require a hardware run.

## REA analysis and source references

- REA 6.8.0 with Rizin 0.9.1 ran `af` and `pdfj` on the shipped Intel Tahoe
  kext. At `0x37cb0`, `setSCAN_REQ` calls `ieee80211_begin_cache_bgscan`.
  At `0x37f70`, `getSCAN_RESULT` walks scan nodes and calls
  `convertNodeToScanResult`. At `0x37600`, `setASSOCIATE` calls
  `associateSSID`. These calls separate the system-facing Wi-Fi interface
  from the radio-specific scan and join code.
  [REA](https://github.com/morluto/rea) · [AirportItlwm source and headers](https://github.com/laobamac/itlwm/tree/7204c69d1bfa7f5cc0f7864e08fb03e55d0ec29e)
- REA also traced the shipped MT7922 kext's `m1Main` at `0x5eb0` and
  `callPlatformFunction` at `0x135a0`. The latter dispatches to the radio's
  native scan snapshot, link status, and frame transmission handlers. The
  MT7902 frontend and radio now build against one bridge header and share the
  beacon counter used for macOS link quality events.
  [Radio base](https://github.com/hbyonly1/itlmt7922-wifi/tree/20aaac5c8e1d97d737a73f1a684ded2ec1d0f940) · [Tahoe frontend and headers](https://github.com/thanhnndev/ga403wm-hackintosh/tree/ee3131f6212c3b5fe946944e13c116578479161e)
- The radio follows Linux's MT7902 PCIe layout, including MCU TX ring 15,
  firmware TX ring 16, shared RX event ring 0, and MT7902 WFDMA prefetch
  settings. The build checks the two embedded firmware files by SHA-256.
  [Linux PCIe setup](https://github.com/openwrt/mt76/blob/66067d203bd04b703b55341bbf986ead5db05653/mt7921/pci.c) · [Linux WFDMA](https://github.com/openwrt/mt76/blob/66067d203bd04b703b55341bbf986ead5db05653/mt792x_dma.c)
