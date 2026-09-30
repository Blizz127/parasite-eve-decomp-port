# Splitting (Phase 2+): splat configuration for SLUS_006.62

## Canonical target

**Disc 1's `SLUS_006.62` is the single canonical executable target.**
Disc 2's `SLUS_006.68` is byte-identical to disc 1's `SLUS_006.62` (SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b` for both, verified 2026-07-04 —
see `docs/disc_info.md` and `docs/reverse_engineering_notes.md`), so every
split, symbol, and eventual rebuild result applies to both discs.
`configs/USA/disc2.yaml` is intentionally a documented pointer, not a config.

## What exactly is split

`build/extracted/disc1/SLUS_006.62` — extracted locally from a user-supplied
disc image by `scripts/extract_us.sh 1`, git-ignored, never committed.
`scripts/split_us.sh` refuses to run unless that file exists and its SHA-1
matches the recorded value.

## Toolchain

`scripts/setup_env.sh` creates a git-ignored virtualenv at `.venv/` and
installs the pinned splitter: **`splat64[mips]==0.41.0`** (latest PyPI
release as of 2026-07-05, when the pin was decided). `scripts/split_us.sh`
prefers `.venv/bin/splat` and falls back to `PATH`. Version bumps are
deliberate decisions recorded in `docs/ai_context/ACTIVE_HANDOFF.md`.

## Dry run first

`scripts/split_us.sh --check` verifies every prerequisite — repo root,
config, extracted EXE presence + SHA-1, splat availability, and that every
output path is covered by `.gitignore` — prints exactly what a real run
would generate, and exits without invoking splat. Run it before the first
real split. A real run additionally diffs `git status` before/after and
fails loudly if the split created anything git does not ignore.

## What is verified (and used in `configs/USA/disc1.yaml`)

From `docs/disc_info.md`, produced by the Phase 1 tooling:

| Fact | Value |
| --- | --- |
| File size | 2,025,472 bytes (`0x1EE800`) |
| SHA-1 | `452fb033f2eaa4b18aa20a5bca60b8125af3a37b` |
| Header | 2048-byte PS-X EXE header (`0x0`–`0x800`) |
| Load address (t_addr) | `0x80010000` |
| Loadable size (t_size) | `0x1EE000` (= file size − header) |
| Entry point (pc0) | `0x80072534` |
| gp0 | `0x00000000` |

The initial config is deliberately a **single conservative segment**: header
plus one `asm` subsegment covering the whole loadable image at vram
`0x80010000`. Data regions will initially disassemble as meaningless
instructions — expected, and preferable to inventing boundaries.

## What is NOT known yet

- Text/data/rodata/bss boundaries inside the image.
- Any symbol names, function boundaries, or overlay structure.
- The exact compiler and flags (`compiler: GCC` in the config is standard
  splat setup for PSX, not a verified toolchain identification — Phase 5+
  fingerprinting will settle it; expect Psy-Q-era gcc).
- `PE.IMG` internal format (out of scope for splitting the EXE).

## What this is NOT

Running `scripts/split_us.sh` produces a **study artifact**: a first-pass
disassembly under `asm/disc1/` plus a linker script. It is **not matching
progress and not a rebuild** — those claims are only permitted once the
Phase 4 checksum/rebuild harness (`scripts/verify_us.sh`) exists and passes,
per `CLAUDE.md`.

## Output policy

All split output (`asm/`, `linkers/`, `assets/`, `.splache`,
`undefined_*_auto.txt`) is git-ignored and stays local; `split_us.sh`
verifies this with `git check-ignore` before running and re-checks
`git status` afterwards. The committed config `configs/USA/disc1.yaml` is
the source of truth; anyone can reproduce the split from their own legally
obtained disc via
`scripts/setup_env.sh && scripts/extract_us.sh 1 && scripts/split_us.sh`.
