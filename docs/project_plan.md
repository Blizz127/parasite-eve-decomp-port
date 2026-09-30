# Project plan

Roadmap for the Parasite Eve (PS1, NTSC-U) decompilation. Each phase must be
complete and reproducible before the next begins. "Complete" means: scripts
exist, outputs are verified, and evidence (commands + hashes) is recorded in
`docs/`.

## Phase 0 — Scaffold and legal boundaries ✅ (this commit)

- Repository structure, ignore rules, documentation skeleton.
- Legal boundaries documented (`docs/legal.md`): no game data in git, ever.
- AI handoff structure (`docs/ai_context/ACTIVE_HANDOFF.md`).

## Phase 1 — Disc verification and executable extraction

- Identify and verify the exact USA disc layout for both discs.
- Record redump-verified hashes of user-supplied images in
  `docs/disc_info.md` (hashes of retail discs are facts, not game data).
- Extract `SLUS_006.62` (disc 1) and `SLUS_006.68` (disc 2) locally via
  `scripts/extract_us.sh`; record their SHA-1/MD5 and PS-X EXE header info.
- Scripts must expect images under `rom/image/` and never write into git.

## Phase 2 — Initial Splat/spimdisasm configs

- Author `configs/USA/disc1.yaml` and `configs/USA/disc2.yaml` for
  [splat](https://github.com/ethteck/splat).
- Start with a single coarse text/data/bss segmentation of the main EXE;
  refine iteratively.

## Phase 3 — First disassembly and linker scripts

- `scripts/split_us.sh` produces `asm/` output and linker scripts locally
  (both git-ignored; the config is the source of truth).
- Sanity-check function boundaries against known PS1 SDK (Psy-Q) idioms.

## Phase 4 — Checksum/rebuild harness

- Assemble the split output back into a byte-identical `SLUS_006.62`.
- `scripts/verify_us.sh` compares SHA-1 against the recorded original.
- From this point on, "matching" claims are allowed only when this passes.

## Phase 5 — Symbol naming and function boundaries

- Symbol maps, function inventories, cross-references with SDK signatures.
- Document overlay structure and disc-file loading in
  `docs/reverse_engineering_notes.md`.

## Phase 6 — Matching C replacement

- Replace assembly functions with C under `src/`, one function at a time,
  verified by the Phase 4 harness (correct compiler/flags to be determined
  from EXE fingerprinting — likely a Psy-Q-era gcc).

## Phase 7 — Native runtime / PC-port research (in progress)

- Host-safe native runtime under `pc_port/` (guest RAM, disc, GPU/IRQ
  shims, translated boot and battle leaves, oracles).
- Battle runtime is partial: promoted oracles through BTL120 exist;
  teardown, first Eve, and full command coverage remain incomplete.
- UE5 (`Blizz127/parasite-eve-ue5`) consumes promoted native behavior
  only. It is not gameplay authority.

## Non-goals (for now)

- A complete playable PC port of the full game.
- Shipping Unreal Engine source in this repository.
- Non-US regions (JP `SLPS-01291/01292`, later Squaresoft Millennium
  Collection reprints) — revisit after the US target rebuilds.
