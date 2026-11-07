# ASUS X1605VA project handoff

Updated: 2026-11-07 (America/Los_Angeles).
Repository: https://github.com/samuelnotfound/Asus-X1605VA-Hackintosh
Baseline investigated: `fa76908c6e558a586c3e2b2fac275ef8a621d7d4` on `main`.

## User goal and workflow

Make the ASUS laptop's F9 microphone-mute key and orange indicator work in macOS. The user also mentioned blinking; expected steady-vs-blinking behavior is not yet established. Confirm microphone mute vs speaker mute if the user's intent changes.

The user switches between Windows and macOS and wants future Codex sessions to retain project context. These tracked notes and the root AGENTS.md are the shared handoff. They preserve documented facts, not the entire chat or internal model state. Open the repository in Codex on each OS and synchronize the same branch before continuing. Save and sync new findings before switching again.

The GitHub connector authenticated as `samuelnotfound` in the 2026-11-07 session. This does not establish Git CLI write credentials on either OS. Never copy access tokens into this repository.

## Evidence from the target macOS boot

User-pasted terminal output, received 2026-11-07:

- macOS 26.7.2, build 25G332 (reported by sw_vers; not independently verified on the target).
- Loaded AppleALC 1.9.8, AsusSMC 1.4.1, AsusMicMuteLED 0.1.1.
- AsusMicMuteLED registered, matched, active. Its cached status was idle, Complete=Yes, Result=0, RequestID=0, FirmwareReply=0. This only establishes an idle snapshot after boot, not successful firmware access or LED operation.
- AsusSMC registered and active, release build, matched PNP0C14. Keyboard-backlight support reported Yes, and VirtualAppleKeyboard was present.
- The filtered loaded-kext output did not show VoodooPS2. Absence from that excerpt is not a diagnosis of the keyboard input path.
- Raw terminal output was not committed; only relevant technical facts are recorded here.

## Failed experiments — do not repeat as routine diagnostics

- Existing repository notes report AsusMicMuteLED 0.1.0 froze the laptop on an on/off request.
- User now explicitly reports **both `probe` and `on` freeze with version 0.1.1**. Exact last printed stages and whether each test was on a separate boot are unknown. Do not ask the user to reproduce a freeze merely to collect a stage.
- No successful LED write or automatic microphone synchronization has been established.
- Earlier Extras/AsusMicMuteLED/README.md and EFI/MicMuteLED/README.txt say 0.1.1 validation is pending and instruct probe testing. The user's newer failure report supersedes that advice. Do not follow those test instructions.
- Disabling the AsusMicMuteLED Kernel/Add entry in the active boot config was recommended. The user has not confirmed doing so. No boot partition was mounted or modified in this session.

## What source inspection establishes

`Extras/AsusMicMuteLED/AsusMicMuteLED.cpp`:

- Driver starts on IOResources and locates ATKD later.
- Probe locates ATKD, validates WMNB, evaluates _WDG, and checks the management GUID, NB method descriptor and instance count. It does not call DEVS or DSTS and does not verify LED-device presence.
- LED write evaluates ATKD.WMNB with instance 0, method 0x53564544 (DEVS), and three 32-bit words: device 0x00040017, state, reserved zero.
- Version 0.1.1 queues work on a thread call. This does not impose a firmware execution timeout or cancel a blocked ACPI operation.
- On/off requests require a successful probe during that boot. Because the user reports both commands freeze, collect the remembered sequence before attributing the on failure to DEVS specifically.
- Status reads cached registry properties. This driver controls only the indicator; it never mutes microphone audio.

Linux asus-nb-wmi maps event 0x7C to KEY_MICMUTE. Linux asus-wmi uses device 0x00040017 for binary mic-mute LED control. Upstream AsusSMC key handling lacks a microphone-mute case for 0x7C. This is a candidate missing key path, not proof the target emits that event or that the bundled binary is identical to upstream.

The fact that probe also freezes means a DEVS-only explanation is insufficient. ACPI evaluation, device selection, interaction with AsusSMC, and firmware/EC behavior remain hypotheses. No root cause is established.

References:

- https://github.com/torvalds/linux/blob/master/drivers/platform/x86/asus-nb-wmi.c
- https://github.com/torvalds/linux/blob/master/drivers/platform/x86/asus-wmi.c
- https://github.com/hieplpvip/AsusSMC/blob/master/AsusSMC/AsusSMC.cpp
- https://learn.chatgpt.com/docs/agent-configuration/agents-md

## New investigation on 2026-11-07

Read `docs/F9-MIC-LED.md` for the detailed new findings and the 0.2.0 test boundary. The original "no dump available" situation below is superseded: a saved 620086-byte original DSDT and accompanying SSDTs were found on the mounted OpenCore volume and inspected offline. No full dump was committed.

The saved firmware's mic LED DEVS branch calls `\_SB.SGOV(0x09020007, state)`, a binary GPIO output. Its EC query `_Q16` forwards event `0x7C`. The static _WDG buffer contains no LED write. None of these offline findings establishes the reason for the probe freeze.

A 0.2.0 candidate source binds directly to ATKD, eliminates the old runtime probe and registry search, and queues explicit SGOV writes. The CLI now calls its cached provider check `check` and refuses requests to loaded versions other than 0.2.0. It remains LED-only and hardware-unvalidated; F9/audio synchronization is not implemented. The bundled EFI driver/tool remain 0.1.1. Their existing MediaTek-profile driver entry is disabled; the Realtek profile has no such entry. No automatic LED writes occur at candidate startup.

This command environment saw the target's cached registry (AsusSMC 1.4.1, old LED 0.1.1 and X1605VA codec), but CoreAudio returned default input ID zero and system_profiler listed no devices. `sudo` was denied. Therefore it cannot establish live microphone muting or load the new driver using sudo. No firmware write or reboot was executed.

## Current boot-file state and next user action

The 0.2.0 candidate compiled as an x86_64 KEXTBUNDLE and passed codesign verification. The new CLI's `status` read the loaded 0.1.1 cached state; `check` correctly refused to send a request to that old version. The SDK linker warned that libkmod startup objects were built for macOS 26.5 while the chosen deployment target is 11.0; only the current Tahoe target is intended for this experiment. No GPIO write was tested.

After the user granted filesystem access, the compiled 0.2.0 kext and matching tool were staged on the mounted OPENCORE volume. Its existing LED entry stays enabled for the next boot; only the entry comment changed, and the rest of the active config was compared structurally against the backup. Files were copied and signature/content verified. An initial attempt to replace the bundle directory was denied at directory removal; the old files were restored and verified before completing a file-by-file replacement inside the existing bundle.

A local backup of the previous kext, tool and private active configuration is in the chat's outputs folder under `LED-backup-before-0.2.0-20261107`, with restoration notes. The candidate ZIP is `AsusMicMuteLED-0.2.0-candidate.zip` in that same outputs folder. These artifacts are not committed to Git. The repository's bundled binaries remain old and its MediaTek profile disables them; pulling Git alone does not reproduce the separately staged candidate.

**Pending user action:** reboot using that OpenCore volume, then return to Codex. No reboot was initiated. First inspect only `micled status` and cached IORegistry properties: confirm 0.2.0 and the ATKD provider binding. Do not assume the new driver loaded from a changed boot file. Do not send old `probe` commands. A new SGOV write still requires staged hardware validation; neither the source fix nor the LED/microphone goal is confirmed. F9-to-CoreAudio synchronization is not implemented yet.

## Earlier investigation plan (review against the new findings above)

1. Obtain an existing original DSDT plus accompanying SSDTs from the target, ideally OpenCore SysReport/ACPI. The repository baseline contains injected AML patches but no complete original firmware-table dump. Inspect dumps offline before another firmware-access test.
2. Trace ATKD._WDG, WMNB, the DEVS branch for 0x00040017, underlying EC/firmware helpers, and relevant locking/waits. Check the target BIOS and which OpenCore profile is actually deployed; do not assume BIOS305 or the i5/i9 variant from inherited comments.
3. Ask which last messages the user already remembers and whether the on test followed a successful probe. Do not trigger new probes to answer this.
4. Determine what F9 and Fn+F9 deliver using an appropriate input-event capture, without invoking the failed LED path. Event 0x7C remains unconfirmed on this laptop.
5. Develop microphone muting separately from LED access. Candidate design: key event to a userspace CoreAudio helper, confirm audio mute state, then synchronize the LED only after the firmware problem is understood. Input mute availability and any volume fallback require testing and clear behavior.
6. Do not claim the fix works until key detection, microphone silence, indicator state, input-device changes, and wake behavior have been validated on the target.

## Continuing on Windows or macOS

In an existing clean checkout, use `git pull --ff-only` on the shared branch. If there are local changes, review and preserve them first; do not reset or force-pull. In a new location, clone the repository and open that folder in Codex.

Start with: "Read AGENTS.md and docs/PROJECT-CONTEXT.md, inspect the working tree, and continue the F9 microphone-mute investigation. Both 0.1.1 probe and on freeze; do not rerun them."

Before leaving a session, update this file with new evidence, the current branch, validation, and outstanding work. Commit and push authorized continuity changes so the other OS can obtain them. Local-only notes do not synchronize automatically.
