# Matching residual policy

## Current disposition

The authoritative exact matching-C count is generated from
`configs/USA/disc1.yaml` and published in
[`docs/generated/DISC1_MATCHING_STATUS.md`](../generated/DISC1_MATCHING_STATUS.md).
`scripts/verify_us.sh --public` rejects a stale published count.

There are also **24 `ACCEPTED-RESIDUAL` leaves**. These are not counted as
matching C and are not registered as `c` spans. They are semantically
reconstructed C candidates whose retail bytes differ only in a documented
compiler, assembler, or per-translation-unit decision that the available
toolchain cannot reproduce under the bounded source-and-flag process.

`ACCEPTED-RESIDUAL` means:

- function hood, boundaries, callers/references, and semantics are proven;
- the candidate C and retail objdump comparison are preserved;
- the allowed phrasing/flag attempts are exhausted or the mechanism is proven
  source-invariant;
- no YAML, build, verifier, or matching-count integration is made;
- the residual mechanism and a possible future unblocker are named.

This is an evidence disposition, not a claim of byte identity. A residual may
be revisited only with a new compiler/toolchain or assembler lever, or with
contradictory retail evidence. It must not be closed with pins, inline
assembly, or a source spelling whose only purpose is to hide the mismatch.

## Residual inventory

The 24 leaves are represented by the current `PARKED-*` records in
`docs/ai_context/parked_blockers.json`; twin records are counted separately:

| mechanism | leaves |
|---|---|
| dbr delay-slot liveness | `func_800698D4` |
| register coloring | `func_800374E8`, `func_80078C94` |
| address-lifetime coloring | `func_800824C8`, `func_800824DC`, `func_80087798`, `func_8005DBAC` |
| assembler temporary selection | `func_8007FBF0` |
| direct volatile delay-slot scheduling | `func_8008783C`, `func_80087864` |
| independent-load scheduling | `func_8006E454`, `func_80080C48`, `func_8007AA34` |
| GP-loop scheduling | `func_80055724` |
| loop-body canonicalization | `func_80062CE4` |
| cross-block/threshold layout | `func_80043474` |
| control-flow constant scheduling | `func_8005E988` |
| comparison-result canonicalization | `func_80073244` |
| register-home / cross-block scheduling | `func_80070D6C` |
| GP load/store scheduling and register home | `func_800339A0` |
| texture-window control-flow/load scheduling/coloring | `func_800762BC` |
| prologue save-batching | `func_8001220C` |
| per-TU toolchain/configuration skew | `func_800725DC`, `func_8007264C` |

The historical `PARKED-*` names remain in the evidence. This policy supplies
the accepted residual disposition without rewriting the original park record.
SDK handwritten COP2/syscall skips are outside this residual count.

## Exact-build relationship

The exact executable SHA-1 remains
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`. It is produced from the
YAML-registered matching-C leaves plus the existing assembly spans; the 24
residual leaves are intentionally excluded from the exact build.
