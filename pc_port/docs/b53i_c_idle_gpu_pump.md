# Phase 6E-B53I-C — idle GPU command pump contract

## Verdict and provenance

B53I-C starts from accepted B53I-B2 commit
`44d05fa06a4461dbb8e37a236bcdee26d44b74b8` (parent
`76d8cffc48b4a0c38627ff5dfbb771a704880f33`) on
`phase6e-b-provider-frontier`.

The retail oracle and freshly rebuilt candidate are both SHA-1:

```text
452fb033f2eaa4b18aa20a5bca60b8125af3a37b
```

This rung translates only the idle-DMA consumer of `func_80076EE4`. It does
not service the second DMA, dispatch another IRQ, translate DrawSync, or
extend the global B50 prefix.

## Exact retail body

`func_80076EE4` is now a complete bounded translation.

| Property | Value |
| --- | --- |
| Range | `0x80076EE4..0x80077143` |
| Exclusive end | `0x80077144` |
| File offset | `0x676E4` |
| Size | `0x260` |
| Instructions | 152 |
| Semantic ABI | `int func_80076EE4(void)` |
| Body SHA-256 | `a124857ab6fd91a3b68ea3e5c2efa5337bf6ed5528ef94ca23bc4a781337a78c` |

B53H already covered the 11-word busy prefix and five-word shared epilogue.
B53I-C adds the complete 136-word idle suffix:

| Property | Value |
| --- | --- |
| Range | `0x80076F10..0x8007712F` |
| Exclusive end | `0x80077130` |
| File offset | `0x67710` |
| Size | `0x220` |
| Instructions | 136 |
| Body SHA-256 | `fed73d363d43e4ce9ada5534224fca37a044bdb2fc590f494d6b1fb413c954c4` |

Every direct or indirect call, branch, and delay slot is verified by
`tools/b53i_c_oracle.py`. Dynamic guest calls are represented by typed
bindings or honest indirect boundaries; no identity is cast to a native
function pointer.

## Literal idle control flow

| Retail PC | Operation |
| --- | --- |
| `0x80076F10` | `func_80073E10(0)` disables I_MASK and returns the old mask. |
| `0x80076F2C` | Empty-ring branch; delay slot at `0x80076F30` always stores the old mask as a 32-bit word in `D_80095880`. |
| `0x80076F40` | Re-read DMA2 CHCR; bit 24 becoming busy takes the ordinary restore path. |
| `0x80076F5C` | Live consumer/producer loop head. |
| `0x80076F6C` | Compute `(consumer + 1) & 0x3F`. |
| `0x80076F74` | Only a final queued entry enters the callback-removal arm. |
| `0x80076F80` | Read live DrawSync callback `D_80095758`. |
| `0x80076F90` | If it is zero, call `func_80073CF4(2, 0)`; delay slot supplies handler zero. |
| `0x80076FA4` | First GPUSTAT read. |
| `0x80076FB8..0x80076FC8` | Literal unbounded bit-26 readiness poll. |
| `0x80076FD0/0x80076FD8` | First and second live consumer loads; they feed worker and argument offsets respectively. |
| `0x80076FFC` | Load argument from ring `+0x04`. |
| `0x80077004` | Third live consumer load, feeding the auxiliary offset. |
| `0x80077020` | Load auxiliary from ring `+0x08`. |
| `0x8007702C` | Load worker from ring `+0x00`. |
| `0x80077034` | `jalr` worker with `(a0=argument, a1=auxiliary)`; return value ignored. |
| `0x80077054` | Only after ordinary worker return, store `(live_consumer+1)&0x3F`. |
| `0x8007706C` | Drained ring exits toward restore. |
| `0x80077080` | If more entries exist, re-read CHCR. |
| `0x8007708C` | Loop only if DMA remains idle. |
| `0x8007709C` | Restore exact saved I_MASK through `func_80073E10`. |
| `0x800770B8` | Skip DrawSync cleanup while queue remains nonempty. |
| `0x800770D8` | Skip DrawSync cleanup while DMA2 is busy. |
| `0x800770F0` | Skip cleanup when the work marker is zero. |
| `0x80077100` | Skip callback when its live guest identity is zero; the delay slot still computes the marker address. |
| `0x80077108` | Clear the work marker before the callback. |
| `0x8007710C` | Indirect DrawSync callback call. |
| `0x80077128..0x8007712C` | Return `(producer-consumer)&0x3F` from live indices. |

## Guest authorities and ring decode

There is no native queue or callback mirror.

| Retail authority | Address/layout |
| --- | --- |
| Producer | `D_80095874` |
| Consumer | `D_80095878` |
| Pump-saved I_MASK | `D_80095880` |
| Work marker | `D_80095754` |
| DrawSync callback identity | `D_80095758` |
| DMA callback slot 2 | `D_800956C8` |
| Ring | `D_800BD030`, 64 entries × `0x60` |
| Worker | entry `+0x00` |
| Argument | entry `+0x04` |
| Auxiliary/source | entry `+0x08` |
| Inline payload | entry `+0x0C` |

Retail uses raw 32-bit `consumer*96` arithmetic before each field access; it
does not pre-mask consumer. The native safety envelope validates each exact
U32 access after the same wrapped arithmetic. An unrepresentable access
stops at `func_80076EE4_ring_span`; it is never normalized to another slot.

The canonical entry is decoded live as:

```text
worker      0x80076664
argument    0x800BD03C
auxiliary   0x8012B8B8
RECT        {256,456,64,1}
payload     0x01C80100 0x00010040
```

## I_MASK lifecycle

On the accepted IRQ path, I_MASK is `0x0009` at pump entry:

```text
0x80076F10  exchange 0x0009 -> 0
             queue/callback/readiness/worker/consumer mutations occur masked
0x8007709C  exchange 0 -> saved 0x0009
```

No value is hard-coded: alternate 16-bit masks are saved in `D_80095880`
and restored exactly. Empty, became-busy, drained, and worker-issued-DMA
ordinary exits all restore. A worker/ring/DrawSync indirect boundary is a
retail non-return and does not invent later cleanup. A pre-latched host stop
cannot suppress restoration after a normally returned worker because the
binding compares the monotonic stop epoch only across that worker call.

## Callback removal and DICR

The channel-2 callback is removed only when both conditions hold:

1. `(consumer+1)&0x3F == producer` (the current item is last), and
2. `D_80095758 == 0` (no DrawSync callback).

The existing complete `func_80073CF4`/`func_800746A0` path stores slot 2
zero before its DICR RMW. Canonically:

```text
DMA callback slot 2   0x80076EE4 -> 0
stored DICR           0x00840000 -> 0x00800000
```

No completion flag is acknowledged and no interrupt is delivered by this
removal. Removal precedes GPU readiness polling and the second worker.

## GPUSTAT readiness

The exact predicate is `GPUSTAT & 0x04000000`. A clear bit produces an
unbounded tight read loop with I_MASK zero. There is no timeout helper, wall
clock, queue mutation, callback, DMA completion, VBlank step, or automatic
ready transition. Tests hold the bit clear in a bounded child process, then
set it explicitly in a separate execution. The oracle accepts readiness as
an explicit input sequence.

## Typed worker and consumer ordering

The sole canonical binding is guest identity `0x80076664`, invoked through
its guest-address ABI:

```text
func_80076664(argument=0x800BD03C, source=0x8012B8B8)
```

Its integer return is ignored. An ordinary `-1` still advances the consumer;
only an actual nested stop-epoch change is a non-return. Any other identity,
including literal zero, stops at a typed indirect-call boundary with the
exact target and two argument values. No host function pointer is stored.

The retail ordering is hard:

```text
callback removal (when selected)
GPUSTAT ready
worker call
ordinary worker return
consumer store at 0x80077054
I_MASK restore
```

A worker boundary preserves all earlier acknowledgements/removal but leaves
consumer unchanged and I_MASK zero. Through the CPU IRQ chain it also leaves
`D_800945E6 = 1`; the canonical normally returned worker permits the CPU
dispatcher to reach its authentic terminal clear, yielding zero.

## Exact second DMA issue and asynchronous boundary

The canonical worker issues:

```text
MADR  0x8012B8B8
BCR   0x00020010
CHCR  0x01000201
```

It then returns, and only then does consumer advance `0 -> 1`. The first
hardware checkpoint captured the first DMA token before completion and
never loops or recaptures. The new transfer has a different token and stays
active; its 64 pixels are not yet DMA-visible in VRAM.

Because callback removal cleared DICR channel-2 enable before issue, the new
DMA starts with stored DICR `0x00800000`. A later hardware completion must
copy its pixels and clear CHCR without creating flag 26, derived bit 31,
I_STAT source 3, or another pump callback. That later completion is outside
B53I-C.

## Marker and DrawSync

Canonical post-worker state has an empty ring but a busy second DMA, so the
cleanup arm is skipped:

```text
work marker         1
DrawSync callback   0
```

For the general represented branch, zero callback leaves the marker set.
A nonzero unbound identity clears the marker first and then stops at a typed
DrawSync indirect boundary after I_MASK restoration. This models every pump
instruction without importing an unrelated DrawSync implementation.

## Canonical measured endpoint

One admitted first-DMA checkpoint produces:

| State | Before C callback | After normal C callback |
| --- | ---: | ---: |
| Producer | `1` | `1` |
| Consumer | `0` | `1` |
| DMA callback slot 2 | `0x80076EE4` | `0` |
| Stored / physical DICR | `0x00840000` | `0x00800000` |
| I_STAT | `0` | `0` |
| I_MASK | `0x0009` | `0x0009` |
| `D_800945E6` | `1` | `0` |
| Work marker | `1` | `1` |
| DrawSync callback | `0` | `0` |
| DMA2 MADR | `0x8012A8B8` | `0x8012B8B8` |
| DMA2 BCR | `0x00400010` | `0x00020010` |
| DMA2 CHCR | `0x00000201` | `0x01000201` |
| DMA2 active | `0` | `1` |
| Completion pending | `0` | `0` |
| Pump entries during focused callback window | `0` | `1` |

The complete `0x60` entry remains byte-identical after consumption; only the
guest consumer word changes. GPUSTAT remains ready, VBlank remains zero,
and the canonical path adds no stop epoch.

## Tests and oracle

B53I-C adds ten focused native groups, bringing the suite to `558/558`:

- exact canonical checkpoint endpoint and byte-stable entry/canaries;
- empty/nonempty, final/non-final, callback-present/already-zero branches;
- exact mask save/disable/restore and pre-latched-stop behavior;
- worker-before-consumer, ordinary failure, and `63 -> 0` wrap;
- unbound worker, invalid ring span, DrawSync, and nested IRQ boundaries;
- held GPUSTAT and explicit readiness transition;
- deterministic repeated issue and no same-checkpoint second completion.

`tools/b53i_c_oracle.py` verifies the retail SHA-1, all 152 reused literal
words, the full and idle hashes, all 19 control-transfer delay slots, exact
consumer-store PC, and 20 explicit idle scenarios. It imports no production
C and has no automatic hardware evolution.

Retained focused gates remain B53I-B2 `15/15`, B53I-B1 `8/8`, B53H `8/8`,
and corrected B53B `15/15` in both normal and sanitizer configurations.

Final measured preservation gates are:

```text
native suite, fresh normal                 558/558
native suite, fresh ASan/UBSan             558/558, zero diagnostics
focused B53I-C, normal/sanitizer           10/10 each
retained B53I-B2, normal/sanitizer         15/15 each
retained B53I-B1, normal/sanitizer          8/8 each
retained B53H, normal/sanitizer              8/8 each
corrected B53B, normal/sanitizer            15/15 each
oracle/executable matrix                    45/45
retained oracle subset                      33/33
B53I-C oracle                               152 words / 20 cases
B49 host-loop, normal/sanitizer             PASS / PASS
canonical strict, normal/sanitizer          exit 1 / exit 1
bootstrap strict, normal/sanitizer          exit 1 / exit 1
```

Preserved outputs are framebuffer SHA-256
`fb28dc21dd1e41eb72b8fe22dd3295bb8ed0c040aa88f7885a68dedc2629dfdb`,
real-data FNV-1a64 `7D860391E1ED6C97`, matching executable SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`, and matching executable
SHA-256 `5d94938ee752e81ef375bd4493c9883850c25a86895f9cb0732cf3622b44351b`.

Five independent read-only review lanes covered literal MIPS/control flow,
worker-before-consumer and typed non-return behavior, async/reset/scope
separation, I_MASK/callback-removal/DICR correctness, and a final adversarial
stop-epoch/authority audit. All five closed with no HIGH or MEDIUM finding.
The only optional LOW suggestion was broader end-to-end coverage for
noncanonical nested worker/DrawSync boundaries; their constituent typed
propagation paths are already covered and no production defect was found.

## Frontiers and scope

The focused B2 boundary `func_80076EE4_idle_pump` is resolved. The complete
callback now returns normally through `func_80074520` and the CPU dispatcher.
The separately measured global continuing frontier remains:

```text
func_8006AD40_prefix_cut from func_8006AD40
retail PC 0x8006AE50
```

Bootstrap strict remains `func_8007F72C` from `func_800698D4`.

The adjacent queue-reset function `func_80077144` and DrawSync implementation
are not reached dependencies of this idle consumer and remain separate
future work; B53I-C does not broaden into ResetGraph ownership.

## Next bounded task

The exact next asynchronous rung is a later, separately admitted completion
of the second DMA token. It must prove VRAM visibility and CHCR idle while
retaining stored DICR `0x00800000`, I_STAT zero, callback slot 2 zero, and no
source-3 dispatch or pump entry, then measure the next retail frontier. It
must not be folded into the first-DMA checkpoint.
