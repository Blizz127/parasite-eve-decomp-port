# Phase 6E-B54D — material prefix to live CD poll

PE-6E-B54D SUCCESS — MATERIAL PREFIX COMPLETED TO LIVE CD POLL AT 0x8006AF54

base_commit=c1efff529e1529893c2ae73f78637b3677c30f77
b54c_evidence_commit=8bf5dbf0e43fd129f4a498ac91fe45cc30b36847
branch=phase6e-b-provider-frontier
head=COMMIT_CONTAINING_THIS_REPORT

old_frontier=0x8006AE68
new_frontier=0x8006AF54
production_range_implemented=0x8006AE68..0x8006AF54 (exclusive)

poll_result_invented=no
func_8006E7E8_consumed=no
func_800718D0_consumed=no
func_80030894_consumed=no
dma_checkpoint_added=no
cd_progression_hack_added=no

native=570/570
asan_ubsan=570/570, zero sanitizer diagnostics
focused_B54D=2/2 normal; 2/2 ASan+UBSan
retained_B54A=8/8
retained_B54B=8/8
retained_B54C=8/8
oracle_matrix=49/49
B49=PASS normal; PASS ASan+UBSan
determinism=3/3 byte-identical framebuffer and trace

framebuffer_sha256=fb28dc21dd1e41eb72b8fe22dd3295bb8ed0c040aa88f7885a68dedc2629dfdb
trace_sha256=42c1956e077a40fed5176653b6a18938a8a91e99e35fe9d7044f31581de785af
disc1_sha256=7f20fce99a7ff18accebf3156419b24d4c0145c5c0f8168d5e86005ccf28f9c4

## Implemented retail prefix

The production port now consumes exactly the 59 audited retail words from
`0x8006AE68` through `0x8006AF53`:

1. It iterates `D_80091648` offsets `0x20` and `0x30`, preserving records 0
   and 1 and writing only the two packed halfwords of records 2 and 3.
2. It calls the already-translated
   `func_8006E498(0x801229A0, 0xABADC06C)`, which returns `0x801229A8` on
   canonical Disc 1 data.
3. It walks the already-loaded archive using each record's aligned size and
   the already-translated `func_8007506C`.
4. It stops before the `jal func_8006E7E8` at `0x8006AF54`.

Canonical packing is exact:

| record | dest1 | value | dest2 | value |
| --- | --- | --- | --- | --- |
| 2 | `0x80091670` | `0x0034` | `0x80091672` | `0x7793` |
| 3 | `0x80091680` | `0x0034` | `0x80091682` | `0x7753` |

The canonical archive walk issues exactly one existing LoadImage call:

```text
RECT={448,0,64,254}
rect=0x801229AC
data=0x801229B4
size=0x00007F0C
terminator=0x8012A8B4
```

The prior channel-2 issue result is canonically `s2=1`, so the retail
`s2==-1` back-edge is not taken. B54D neither models nor assigns the result
of the following live CD poll. The channel-2 busy bits remain set at the
new frontier in focused proof.

## Focused proof

`test_B54D_6AD40_canonical_material_prefix` seeds the exact canonical base,
directory entry, archive key, material record inputs, one-record RECT/data
payload, and zero terminator. It proves:

- the retained B54B count of 13 texture dispatches and their order;
- the four exact packed halfwords and no writes to records 0 or 1;
- the fourteenth and only material-walk dispatch has
  `RECT {448,0,64,254}` and data `0x801229B4`;
- the aligned step reaches `0x8012A8B4`;
- the only provider is the established `func_8006AD40_prefix_cut`;
- the CD busy bits remain live and neither later dependency is entered.

`test_B54D_6AD40_no_walk_still_stops_before_poll` independently proves the
packing is not conditional on a nonzero archive record, a zero terminator
does not fabricate a LoadImage, the same `0x8006AF54` frontier is reached,
and no DMA completion checkpoint appears.

The accepted B54C evidence and independent oracle are preserved verbatim as
`pc_port/docs/b54c_func_8006AD40_material_table_audit.md` and
`pc_port/tools/b54c_6ad40_material_table_oracle.py`.

## Regression and determinism

Fresh normal and container-built ASan+UBSan suites each pass 570/570. The
complete independent oracle set passes 49/49 against executable SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`; B54A, B54B, and B54C each
pass 8/8 scenarios.

The retained B49 real-disc harness passes with both executables. Three fresh
normal real-disc runs produce byte-identical framebuffer and trace files at
the hashes above. The unchanged framebuffer is expected because presentation
precedes this loader prefix; no presentation or hardware completion was
added.

## Stop line

NEXT ONLY: PE-6E-B54E-A — READ-ONLY CANONICAL CD POLL SEMANTICS AUDIT

Do not consume `func_8006E7E8`, infer its result, enter `func_800718D0` or
`func_80030894`, add a DMA checkpoint, or add CD progression in B54D.
