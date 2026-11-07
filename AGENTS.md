# Project continuity

Read `docs/PROJECT-CONTEXT.md` before investigating or changing this project. It holds the current hardware findings, failed experiments, and next steps across Windows and macOS sessions.

- Keep that file current after meaningful work: record changes, validation, failures, unresolved questions, and the next action. Separate user reports, observed logs, source findings, and hypotheses.
- Check the working tree and branch before edits. Preserve user changes. Review remote changes before synchronizing; do not overwrite divergent work.
- Keep continuity notes portable: use repository-relative paths and identify which OS produced evidence. The current execution host is not necessarily the target ASUS laptop.
- Do not store credentials, tokens, SMBIOS identifiers, or unredacted personal diagnostic logs in tracked notes.
- Both `micled probe` and `micled on` with AsusMicMuteLED 0.1.1 reportedly freeze the target laptop. Do not repeat those calls or describe the probe as safe. Investigate offline ACPI tables first. See the context file for details.
- Do not equate a successful build, loaded kext, descriptor match, or matching Linux command with hardware validation.
- Before switching systems, save the handoff and synchronize authorized changes to GitHub. Report the branch and commit, and distinguish local changes from published changes. Never claim another checkout was updated without checking it.
- Repository updates do not install anything into the laptop's boot EFI. Treat boot deployment and runtime firmware tests as separate actions.
