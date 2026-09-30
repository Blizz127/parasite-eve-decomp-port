# Legal boundaries

Parasite Eve is © Square (now Square Enix). This project is an unaffiliated
fan research effort.

## What this repository contains

- Original tooling, build scripts, and configuration written for this
  project.
- Documentation, research notes, symbol names, and structural observations
  produced by clean reverse engineering.
- Eventually: newly written C source that, when compiled, reproduces the
  behavior of the original executable.

## What this repository must NEVER contain

- Disc images or any part of them (`.bin`, `.cue`, `.iso`, `.img`, `.ccd`,
  `.sub`, `.chd`, etc.).
- Extracted game files or assets (executables, graphics, audio, text, data
  archives) — including the `SLUS_006.62` / `SLUS_006.68` executables
  themselves and raw disassembly dumps of them (`asm/` is generated locally
  and git-ignored).
- Sony Psy-Q / PsyQ SDK files, headers, libraries, or documentation, or any
  other proprietary SDK material.
- Copyrighted symbol/debug information from leaked sources.

Checksums and hashes of retail media, byte offsets, file names, and
structural documentation are facts about the work, not the work itself, and
are fine to record.

## User responsibilities

Users must supply their own legally obtained copies of the game. All scripts
in this repository take user-supplied disc images (placed under
`rom/image/`, which is git-ignored) as input and keep all derived game data
out of version control.

## Licensing note

A license for the original tooling and documentation in this repository has
not been chosen yet (candidates: MIT for tools/docs, with the usual decomp
caveat that reconstructed game code carries no license grant). Track this as
an open Phase 0/1 task.
