# Reverse engineering notes

Running log of verified observations about the Parasite Eve (NTSC-U)
binaries and disc data. **Every entry must cite its evidence**: the tool,
command, address/offset, or checksum that backs it. Speculation goes in a
clearly marked "Hypotheses" section, never mixed with facts.

## Template for entries

```markdown
### <topic> (YYYY-MM-DD)

- Observation: ...
- Evidence: <tool + command + output excerpt / address / hash>
- Confidence: verified | probable | hypothesis
```

## Verified observations

### Disc 1 and disc 2 boot executables are byte-identical (2026-07-04)

- Observation: `SLUS_006.62` (disc 1) and `SLUS_006.68` (disc 2) are the
  same file under two names. There is exactly one executable to decompile;
  disc-specific behavior must come from data (`PE.IMG`, FMV/XA
  files) or a runtime disc check, not from different code.
- Evidence: `scripts/extract_us.sh all` — both extracted EXEs are 2,025,472
  bytes with SHA-1 `452fb033f2eaa4b18aa20a5bca60b8125af3a37b`, MD5
  `cb095240a2ba358b8fdcbfd4d4f97f04`, CRC-32 `7c10c01c`, and identical PS-X
  EXE headers (pc0 `0x80072534`, t_addr `0x80010000`, t_size `0x1EE000`).
  Full values in `docs/disc_info.md`.
- Confidence: **verified** (for the local dumps; redump cross-check of the
  images themselves still pending).

### PE.IMG is byte-identical across both discs (2026-07-04)

- Observation: the 206 MB packed data archive `PE.IMG` is the same file on
  disc 1 and disc 2 (same LBA 1013, same size, same hashes). Together with
  the identical boot EXEs, the two discs differ **only** in FMV/XA streams
  and volume metadata — the decomp target is effectively one EXE plus one
  archive. The archive also contains no Mode 2 Form 2 sectors (the
  extractor's Form 2 guard never tripped across all 100,690 sectors).
- Evidence: `tools/extract/psxiso.py extract "<discN>.bin" PE.IMG
  build/extracted/discN/PE.IMG` for both discs, then
  `tools/verify/hashfile.py` on both copies: 206,213,120 bytes, CRC-32
  `678da5c6`, MD5 `e8e714191694ce3516c56c1f0b3999a9`, SHA-1
  `146c0ce7308bf9fdc2ba5a84230e198db0663f3b` — identical for both.
- Confidence: **verified** (local dumps).

## Known starting points (unverified, to confirm in Phase 1–2)
- Era/toolchain: 1998 Square PS1 title — expect Psy-Q SDK libraries linked
  into the EXE and a Psy-Q-era gcc for game code. Confirm via library
  signature matching before assuming compiler flags.
- Expect overlays and/or file-based code loading from disc; PE2 uses
  overlay-heavy structure and PE1 likely does something similar. Verify, do
  not assume.

## Reference projects (structure only — do not copy source)

- Parasite Eve 2 decomp: https://github.com/GabeRealB/parasite-eve-2-decomp
  (splat-based layout, ninja build, checksum verification flow)
- Xenogears decomp — comparable Square-era discipline and tooling.

## Hypotheses

(none currently open — the PE.IMG identity hypothesis was verified
2026-07-04 and moved to Verified observations above)
