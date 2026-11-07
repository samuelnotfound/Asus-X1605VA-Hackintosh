# AsusMicMuteLED — experimental X1605VA LED driver

Versions 0.1.0 and 0.1.1 have reported whole-system freezes. With 0.1.1, both `probe` and `on` reportedly freeze. Do not follow the old probe instructions.

Source here is an unvalidated 0.2.0 candidate. It binds directly to ATKD; startup and `micled check` perform no ACPI method evaluation. Explicit LED writes use the saved X1605VA firmware's SGOV GPIO helper. This bypasses the old registry traversal and WMI/probe route, but the freeze's root cause remains unknown and SGOV writes are not yet validated.

Build on macOS:

    bash Extras/AsusMicMuteLED/build.sh

Outputs are in Extras/AsusMicMuteLED/out. A build does not deploy the kext. The copies bundled under EFI remain 0.1.1; the existing MediaTek-profile LED entry is disabled. The Realtek profile has no LED entry.

After a separately prepared candidate boot, `micled status` reports cached state. The new tool refuses `check` and write requests unless the loaded driver reports 0.2.0. A cached provider check is not proof of firmware safety, and a returned write is not visual or GPIO readback confirmation.

This driver only controls the physical indicator. F9 key/audio integration remains outstanding. See docs/F9-MIC-LED.md and docs/PROJECT-CONTEXT.md for evidence, runtime limits, and next steps.
