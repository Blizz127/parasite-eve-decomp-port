# Phase 6E-B54B — complete `func_8006AD40` counted loop

PE-6E-B54B SUCCESS — COUNTED func_8006E1C0 LOOP COMPLETED TO 0x8006AE68

base_commit=f003e4aa5ecfd17c5b98aa466f8262f44c5d6d37
b54a_evidence_commit=d58ff9b88b71373fd34c3d3866e8c16220a9a5d0
branch=phase6e-b-provider-frontier
head=COMMIT_CONTAINING_THIS_REPORT

old_frontier=0x8006AE50
new_frontier=0x8006AE68

retail_loop_count=13
already_completed_entries=1
additional_func_8006E1C0_calls=12
final_completed_entries=13

entry0_replayed=no

production_range_implemented=0x8006AE50..0x8006AE68 (exclusive)
later_suffix_consumed=no

func_8006E498_consumed=no
func_8007506C_table_walk_consumed=no
func_8006E7E8_poll_result_invented=no
func_800718D0_consumed=no
func_80030894_consumed=no
third_dma_checkpoint_added=no

tests=568/568 normal; focused B54B 2/2; retained D/C/B2/B1/H/B53B 8/10/15/8/8/15
sanitizers=568/568 ASan+UBSan with zero diagnostics; same focused counts
retained_oracles=46/46 pre-B54B retained; 47/47 complete matrix; B54A 8/8; B54B 8/8
determinism=3/3 byte-identical framebuffer and trace; B49 normal/sanitizer PASS

framebuffer_sha256=fb28dc21dd1e41eb72b8fe22dd3295bb8ed0c040aa88f7885a68dedc2629dfdb
fnv=7D860391E1ED6C97

next_unresolved_address=0x8006AE68
next_unresolved_surface=D_80091648 packing followed by archive lookup/table walk and live CD poll; requires a read-only audit

hard_blockers=none
warnings=Do not infer a func_8006E7E8 result or func_800718D0 reachability; the next rung is audit-only

## Literal bounded behavior

B54A verified the full 391-word retail function against executable SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`. B54B implements exactly the
six words at `0x8006AE50..0x8006AE67`, reached after the already-translated
call and delay slot at `0x8006AE48/4C`:

```text
8006AE50  lw     v0,0x28(s3)       reload header
8006AE54  addiu  s1,s1,1           completed-entry counter
8006AE58  srl    v0,v0,22          current count
8006AE5C  sltu   v0,s1,v0          unsigned comparison
8006AE60  bnez   v0,8006AE44       next helper call
8006AE64   addiu s0,s0,0x14        unconditional branch delay
8006AE68  addiu  a1,zero,0x20      NOT consumed by B54B
```

Production retains separate `header`, `count`, `issued`, and `entry` values.
After every helper return it reloads the header, increments the unsigned
counter, performs the unsigned comparison, and advances the guest entry by
`0x14` regardless of whether the back-edge is taken. For canonical
`count=13`, the existing first call handles entry 0 once; the back-edge calls
entries 1 through 12 in order. At exit `issued/s1=13`, and the delay-slot
advance leaves the local entry/s0 equivalent at `0x8012E05C`, one record past
entry 12. Later retail code would overwrite it, but B54B does not enter that
code.

Initial count zero is a genuine retail path that branches directly to
`0x8006AE68` before the first helper. Count one is the valid zero-remaining
state at `0x8006AE50`: the entry-0 call has returned, `s1` increments to one,
the unsigned comparison is false, the delay-slot pointer advance still runs,
and no extra helper is called.

## Focused proof

`tests/test_native.c` adds two focused groups:

- canonical count 13 produces 13 total distinct dispatches, proving entry 0
  once plus exactly 12 additional entries in `1..12` order;
- counts 0, 1, and 2 prove direct bypass, valid zero-remaining behavior,
  increment-before-compare, the `0x14` stride, and off-by-one boundaries.

The canonical test leaves sentinel halfwords at `0x80091650/52` unchanged,
requires the established unresolved provider as the only stop, and rejects
later unresolved functions. The entry fixtures use zero-width image records
only to observe helper ordering without creating an artificial hardware
completion; the retail call count and addresses come from the literal table
loop, not those payload dimensions.

`tools/b54b_6ad40_counted_loop_oracle.py` imports no production code. It
requires the matching executable SHA-1, verifies the call/delay plus all six
B54B words and the first excluded word, and executes eight scenarios covering
canonical completion, entry-0 exclusion, count edges, header reload, literal
`sltu`, delay-slot pointer advancement, and the exact stop address.

## Hardware and scope preservation

The six added retail instructions perform no MMIO or global stores. Their
only calls are the remaining invocations of the already-translated
`func_8006E1C0`; B54B adds no GPU/DMA/IRQ/CD model and no hardware checkpoint.
Existing helper effects continue through the established authority. No third
DMA opportunity, autonomous completion, I_STAT/I_MASK/DICR mutation, callback
delivery, producer/consumer shortcut, or presentation is introduced by the
loop mechanics.

The implementation stops before the first `0x8006AE68` instruction. It does
not pack `D_80091648`, look up `0xABADC06C`, walk payloads with the later
`func_8007506C` site, poll with `func_8006E7E8`, or approach
`func_800718D0`/`func_80030894`.

## Regression and determinism results

```text
native suite, normal                 568/568
native suite, ASan+UBSan             568/568, zero diagnostics
focused B54B, normal/sanitizer         2/2 each
retained B53I-D                        8/8 each
retained B53I-C                       10/10 each
retained B53I-B2                      15/15 each
retained B53I-B1                       8/8 each
retained B53H                          8/8 each
retained B53B                         15/15 each
pre-B54B oracle matrix                46/46
complete oracle matrix                47/47
B54A oracle                            8/8
B54B oracle                            8/8
B49 host loop, normal/sanitizer       PASS / PASS
retail extraction, Disc 1/Disc 2      PASS / PASS
```

Three fresh normal real-disc runs produced byte-identical PPM and trace files:

```text
framebuffer SHA-256  fb28dc21dd1e41eb72b8fe22dd3295bb8ed0c040aa88f7885a68dedc2629dfdb
trace SHA-256        42c1956e077a40fed5176653b6a18938a8a91e99e35fe9d7044f31581de785af
real-data FNV-1a64   7D860391E1ED6C97
matching EXE SHA-1   452fb033f2eaa4b18aa20a5bca60b8125af3a37b
```

The framebuffer hash remains unchanged for an explained reason: the accepted
host framebuffer presentation occurs before this bounded field-loader loop,
and the added calls use the existing guest GPU/DMA/VRAM path without adding a
presentation or hardware completion opportunity. The FNV driver is an
independent PE.IMG read test and is likewise outside this loop. A hash change
would therefore have been unexplained and rejected; none occurred.

## Stop line

The next rung is a read-only audit beginning at `0x8006AE68`. It must classify
the packing, lookup, table walk, and live poll before naming another honest
execution boundary. B54B does not begin that audit or any later translation.
