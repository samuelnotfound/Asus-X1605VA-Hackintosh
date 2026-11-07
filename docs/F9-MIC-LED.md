# X1605VA F9 microphone indicator investigation

## Status as of 2026-11-07

The orange physical F9 indicator is part of the requested fix. Software mute alone does not satisfy it. There is no confirmed working macOS fix for this particular machine yet. Version 0.1.1 reportedly freezes on both `probe` and `on`; do not repeat those commands with that driver.

Source in `Extras/AsusMicMuteLED` is now an **unvalidated 0.2.0 LED-only test candidate**. The bundled EFI binaries remain 0.1.1, and their existing entry in `config-mediatek.plist` is disabled. The Realtek profile did not contain an LED-driver entry. Building the new source does not replace the bundled binary or deploy the active boot EFI.

## New offline evidence

A saved original ACPI dump was found on the target's OpenCore volume. DSDT length: 620086 bytes; OEM/table IDs `_ASUS_` / `Notebook`; checksum valid; SHA-256 `f15b63cef73a172c49a95fb340e630f72378b98e69958c87b3c6373227e93fcf`. The full firmware dump is kept outside Git because it can contain unrelated machine data. Disassembly was inspected offline. Joint table loading reported duplicate namespace entries in OEM SSDTs; relevant DSDT AML bytes were also inspected directly.

In this saved DSDT:

- `\_SB.ATKD._WDG` is a static 60-byte buffer. Evaluating it should not itself perform the GPIO write. The probe freeze is therefore not explained by the LED's DEVS branch alone.
- `\_SB.ATKD.WMNB` is serialized. Its DEVS branch for device `0x00040017` calls `\_SB.SGOV(0x09020007, 1)` for on and the same helper with zero for off.
- SGOV uses GGRP, GNMB and GADR to locate the pad, then writes bit zero of a SystemMemory operation region. There is no color-selection or blink parameter in this branch. It is a binary indicator control; visible color still requires observation.
- `\_SB.PC00.LPCB.EC0._Q16` sends ASUS event `0x7C` through ATKD.IANE when ATKP is set. This establishes a firmware route, not observation of a live F9 press.
- A saved Linux report detected `platform::micmute`, brightness zero and maximum one. The adjacent test script looked for a different name, `asus::micmute`; the report contains no completed-blink confirmation. Linux detection is not proof that a visual test passed.

Upstream ThinkpadAssistant documents microphone mute with a status LED, and YogaSMC supports Lenovo systems. These establish precedent on Hackintosh, but their hardware-specific LED methods cannot be installed as an ASUS fix:

- https://github.com/MSzturc/ThinkpadAssistant
- https://github.com/zhen-zen/YogaSMC

## Candidate design and limits

Version 0.2.0 binds directly to the ATKD IOACPIPlatformDevice alongside AsusSMC using a distinct match category. It rejects other provider names. Startup performs no ACPI method evaluation or GPIO operation.

The new `micled check` command checks the already-bound provider only. It does not walk the registry, validate WMNB, read _WDG, or probe the electrical LED. `on` and `off` queue a direct call to the exact SGOV helper identified above. SGOV has no return value, so the driver uses evaluateObject; a successful return is not GPIO readback or visual verification. The helper refuses writes/check requests to any loaded version other than 0.2.0, to avoid inadvertently calling the old driver.

This removes the old probe path and the WMI wrapper, but **the actual reason for the 0.1.1 freeze is still unknown**. Direct SGOV access also needs target validation and may still hang. A userspace timeout cannot cancel an in-flight kernel firmware call. Do not label this candidate safe or fixed based on compilation alone.

## Validation completed here

- Live cached registry inspection saw AsusSMC 1.4.1 and AsusMicMuteLED 0.1.1 active, plus the PS/2 keyboard and the X1605VA ALC256 codec configuration.
- Source compilation, kext signing verification and Mach-O checks are recorded in the project handoff.
- A read-only CoreAudio query returned success with default input device ID zero; system_profiler also listed no audio devices in this command session. This is a session visibility result, not proof that the user's microphone is missing.
- `sudo` was denied in the command environment. No driver was live-loaded, no reboot was initiated, and no LED GPIO write was executed.

## Next validation boundary

First boot the candidate on a recoverable test EFI, preserving the previous files and all unrelated active settings. Inspect `micled status` and verify version 0.2.0 and the ATKD binding. `micled check` is a cached check only; it does not establish firmware safety. Any subsequent on/off operation is a new hardware test whose result must be recorded before adding automatic F9/audio synchronization. Never run both old and new copies of the driver.

F9 key capture, actual microphone mute and state restoration, LED synchronization, device switching, and wake handling are still outstanding. The LED must follow confirmed microphone state; blinking or a UI overlay must not substitute for successful muting.
