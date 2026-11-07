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

To integrate this package into an EFI, add both entries under OpenCore
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

## Protocol changes in 0.3.0

Both kexts must be replaced together: 0.3.0 uses version 3 of the internal
join/link bridge and rejects the older bridge. This is a larger protocol
change, with a persistent WPA2-PSK/CCMP RSN session in the kernel.

- The WCL request carries its RSN element, channel, exact SSID byte length and
  immutable request sequence. The radio validates AP/request security before
  association and uses one selected station RSN for association and M2.
- M3 requires the matching AP RSN, CCMP key length, valid MIC/ANonce/replay,
  Secure/Install bits and valid encrypted GTK data. Post-link M3 retransmission,
  GTK rekey and PTK rekey remain active without reinstalling identical keys.
- EAPOL replies use the actual TX token's firmware completion, rather than a
  fixed delay. A cancelled attempt, AP leave, expired grant, failed TX or failed
  key transaction cannot authorize the port. Cleanup removes keys and the AP
  station/BSS state and clears the session's secret material.
- RX retains CCMP PN/key metadata. Per-key/per-TID replay checks run after
  reorder; A-MSDU subframes share one MPDU check. Key epochs invalidate buffered
  packets from a previous key. Missing security metadata fails closed.
- Native status/AP records are published as locked snapshots. Link callbacks
  carry their request sequence. Connect completion waits for a real beacon
  observed after successful association. The frontend advertises the same
  22 non-DFS scan channels that the radio implements.
- Association responses are checked for AID, capabilities, rates and bounded
  IEs; fast-mode firmware capabilities come from that response.

Regression tests exercise the persistent RSN session, malformed M3 rejection,
large key frames, duplicate-message handling, GTK/PTK rekey, cancellation,
rollback, deadlines, RX PN extraction and replay checks after reordering.
These checks run with address/undefined-behavior sanitizers in macOS CI.
They cannot establish RF transmission, firmware key acceptance, or successful
connection on the physical MT7902. The saved 0.2.1 authentication failure
preceded RSN; its hardware cause remains unverified.

WPA3/SAE, enterprise authentication, PMF-required profiles, FT and DFS scanning
remain unsupported. The missing-GTK rejection for the selected CCMP group
profile is retained, consistent with the official Linux supplicant reference.

The diagnostic collector also records `wifi-rsn-error`. `wifi-handshake` now
contains persistent session state, message counts, group-rekey count, rejection
count and attempt sequence. No PMK/PTK/GTK bytes are published.

## Authentication changes in 0.2.2

The X1605VA's 0.2.1 report reached `auth-fail` on channel 11 after accepting
the native PMK. The WPA2 four-way handshake had not started. That report
does not distinguish a missing authentication response from an AP rejection.

Version 0.2.2 clears the fixed-rate TX descriptor's SPE index, following
`mt76_connac2_mac_write_txwi`, which writes that field only for devices outside
the Connac2 family. MT7902 is Connac2. It also keeps BSS RLM transmit streams
and receive chain mask at one when HT/VHT is enabled, and rejects authentication
or association responses shorter than their 30-byte fixed fields.
CI now runs the RX/data/TX codec suites as well as the native join and WPA2
protocol suites. Passing these checks does not establish a successful MT7902 join.

The `wifi-auth-result` property records authentication algorithm, transaction
and IEEE status when a response is accepted. `wifi-mgmt` records request type,
attempts, queued packets, response presence and RX/TX completion counters.
These properties survive a failed attempt in the current boot. The diagnostic
collector also includes the existing `m3-log` and native request trace.

## Connection changes in 0.2.1

The frontend accepts WPA2-Personal PMK and PMKSA key types 6 and 7, matching
AirportItlwm's join decoder. It validates and forwards the BSSID selected by
macOS instead of selecting another access point with the same SSID.
EAPOL reception starts before the association exchange, and the handshake
preserves frames received while the firmware BSS and station records are
updated. The MT7902 RX-path command uses one transmit stream and one receive
chain, following the Linux firmware command layout.

For a failed join, the frontend records `mt7902-join-error`,
`mt7902-join-key-cipher`, and `mt7902-join-key-length`. These fields contain
protocol metadata. The radio's `m3-res`, `wifi-error`, and `wifi-handshake`
properties show the failing connection operation and the received EAPOL
messages. Capture them after one connection attempt.

```sh
ioreg -r -c itlmt7925WifiInterface -d 1 -l -w0 | grep -E 'mt7902-join-'
ioreg -r -c itlmt7922 -d 1 -l -w0 | grep -E 'port-version|m3-res|wifi-error|wifi-handshake|wifi-pmkid-match|wifi-auth-result|wifi-mgmt'
```

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
- For the failed native join, REA disassembled the deployed frontend at
  `0x7d50` and confirmed its comparisons against key types 6 and 9. It
  traced the Intel release's `beginWCLJoin` at `0x3af90` and
  `startWCLJoin` at `0x3bd40`, and the MT7902 radio's `m3Handshake` at
  `0xbb50`. The Intel source accepts personal PMK types 6 and 7. Its
  published decoder also preserves the selected candidate's BSSID.
  [Intel WCL protocol decoder](https://github.com/laobamac/itlwm/blob/7204c69d1bfa7f5cc0f7864e08fb03e55d0ec29e/AirportItlwm/WCL/WCL.hpp)
- The radio follows Linux's MT7902 PCIe layout, including MCU TX ring 15,
  firmware TX ring 16, shared RX event ring 0, and MT7902 WFDMA prefetch
  settings. The build checks the two embedded firmware files by SHA-256.
  [Linux PCIe setup](https://github.com/openwrt/mt76/blob/66067d203bd04b703b55341bbf986ead5db05653/mt7921/pci.c) · [Linux WFDMA](https://github.com/openwrt/mt76/blob/66067d203bd04b703b55341bbf986ead5db05653/mt792x_dma.c)
