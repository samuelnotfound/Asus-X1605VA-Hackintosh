## Patch guide

Follow the parent project's [INSTALL.md](https://github.com/JulioNectar/Hackintosh-XPS13Plus-IrisXe-Tahoe/blob/main/docs/INSTALL.md), especially the kernel and user-space sections, with the retained [kernel patch reference](KERNEL-PATCHES.md) and [Metal reference](METAL.md). The local `install.sh` targets **Tahoe 26.7.1, build 25G241**.

## Preserved settings

- GPU path: `PciRoot(0x0)/Pci(0x2,0x0)`.
- `device-id` bytes: `499a0000` (Tiger Lake spoof `0x9A49`).
- `AAPL,ig-platform-id` bytes: `0000499a`.
- NootedGreen, Lilu and WhateverGreen remain enabled; WhateverXe stays disabled.
- `ngreen-eu=80 ngreen-l3=6` is preserved, following the parent's 80-EU guidance.
- Existing `-ngreentglwithgfx`, `-allow3d`, `-disablegfxfirmware`, `-ngreenv189`, `-ngreenkeepefi`, `-ngreenxefix`, `-ngreenfixbootmode`, `ngreen-dmc=adlp`, `ngreenSched=4`, `ngreenV142=3` and `gfxrst=1` remain unchanged.
- `NVRAM > Delete` already includes `boot-args` for GUID `7C436110-AB2A-4BBB-A880-FE41995C9F82`, matching the parent's reused-NVRAM procedure.

The imported NootedGreen binary is the supplied working baseline. Its source revision has not been established; its config comment does not prove provenance. Retained parent source may be newer. A rebuild is not necessarily a byte-identical replacement.

## Future build / update sequence

After recording the macOS build and preserving a working backup, follow the parent procedure on an APFS/HFS macOS checkout:

```sh
xcode-select --install       # if Command Line Tools are missing
zsh install.sh --check       # compatibility checks
zsh install.sh --build-only  # downloads and builds without installing
```

When testing resumes, the parent's `zsh install.sh` performs macOS-side installation: NootedGreen, HookCase, patched Metal driver, libxedrv and shader-compiler adapters. HookCase belongs in `/Library/Extensions` under that procedure. Review differences before putting the matching new NootedGreen in a test EFI. Keep the ASUS ACPI/USB/input baseline instead of generating a Dell EFI.

The parent's Dell tuning values `ngreenblmax=17777` and `ngreenpm=22` are absent from this baseline. Do not add them as generic graphics requirements. Dell audio, fingerprint and haptic tools are unrelated to establishing ASUS graphics acceleration.

## Evidence for later macOS testing

```sh
sw_vers
kmutil showloaded | grep -E 'NootedGreen|HookCase|AppleIntelTGL'
system_profiler SPDisplaysDataType
ioreg -rc IntelAccelerator -d1 | grep -E 'xe-topo|xe-rowchk'
nvram boot-args
```

Record Metal support, topology and loaded versions alongside the tested EFI revision. No installer/fallback config is provided. The parent warns that disabling NootedGreen alone while retaining the GPU spoof after installing the TGL stack can panic or stall; read its recovery section before designing one.
