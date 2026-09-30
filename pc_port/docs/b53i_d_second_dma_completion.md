# Phase 6E-B53I-D — second LoadImage DMA completion

## Verdict and scope

**B53I-D completes exactly the second DMA token at one later, explicitly
admitted hardware checkpoint.** It does not translate another retail
function and does not begin the B50 suffix.

Starting B53I-C commit:

```text
175b16a4a78b68ec7515deceb2fd6088b57c4356
```

Parent:

```text
44d05fa06a4461dbb8e37a236bcdee26d44b74b8
```

The production GPU/DMA implementation required no semantic change. B53I-B2
already provided token-specific completion, exact VRAM-before-CHCR ordering,
enable/master flag gating, and a sticky derived-DICR edge. The accepted
checkpoint already captures one token once and never loops or recaptures.

Two narrow, inert checkpoint guards make the phase boundary explicit. The
DICR bridge is entered only when the single GPU authority reports a retained
rising edge, and the retail CPU scanner is entered only when live
`I_STAT & I_MASK` is nonzero. The latter is the same guard the scanner already
applies and is not gated solely on a newly created DICR edge, so an older
pending source that becomes unmasked remains eligible. Value-only counters
record checkpoint calls, token queries/services, bridge calls, and scanner
calls; they never participate in scheduling. For the second completion no
edge is pending and I_STAT is zero, so neither delivery layer is called.

The canonical continuation now admits exactly two explicit invocations of
that same checkpoint surface. The first retains C's one-token contract: it
captures/completes the first DMA, services the authentic IRQ/callback/pump,
and returns with the callback-issued second DMA still untouched. Only after
that normal return does a distinct second invocation capture token 2. There
is no loop, token requery in either invocation, recursive service, periodic
scheduler, or third opportunity before the pre-existing B50 stop.

## SHA-exact executable and entry provenance

The retail/matching executable remains:

```text
SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
```

A fresh debugger capture immediately after the accepted C checkpoint
measured:

```text
first token                   0x0000000000000001
second token                  0x0000000000000002
MADR                          0x8012B8B8
BCR                           0x00020010
CHCR                          0x01000201
DMA active / completion       1 / 0
stored / physical DICR        0x00800000 / 0x00800000
DICR rising edge pending      0
I_STAT / I_MASK               0x0000 / 0x0009
D_800945E6                    0
producer / consumer           1 / 1
DMA callback slot 2           0
work marker / DrawSync        1 / 0
pump entries / worker calls   1 / 1
GPUSTAT / VBlank              0x04000000 / 0
destination endpoints         0x0000 / 0x0000
stop epoch                    0
```

The complete `0x60`-byte queue slot is:

```text
64 66 07 80 3c d0 0b 80 b8 b8 12 80 00 01 c8 01
40 00 01 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
```

This decodes to worker `0x80076664`, argument `0x800BD03C`, auxiliary
`0x8012B8B8`, and RECT `{256,456,64,1}`. The D rung never reads this entry
to decide hardware behavior; it snapshots it only as evidence that the
hardware service does not mutate the retail queue.

## Exact second transfer

The accepted C worker issued:

| Field | Value |
| --- | --- |
| Source / MADR | `0x8012B8B8` |
| BCR | `0x00020010` |
| CHCR before | `0x01000201` |
| Blocks | 2 |
| Words per block | 16 |
| DMA words | 32 |
| Pixels | 64 |
| Destination | `(256,456)` through `(319,456)` |

The 32 source words occupy `[0x8012B8B8,0x8012B938)`. Word `i` is:

```text
(0x6000 + 2*i) | ((0x6001 + 2*i) << 16)
```

The first word is `0x60016000`; the last is `0x603F603E`. Completion
therefore exposes pixels `0x6000..0x603F` at X `256..319`, low halfword
first. All 64 destination pixels are zero before service and exact afterward.
Pixels immediately left/right and above/below remain zero; the earlier first
image endpoints `(704,64)=0x2000` and `(735,127)=0x27FF` remain intact. The
entire source span and its adjacent words are byte-stable.

## Token and completion ordering

The later checkpoint executes the already-accepted sequence:

1. capture `PE_GPU_DMA2EventToken()` once;
2. call `PE_GPU_ServiceDMA2Completion(captured_token)` at most once;
3. evaluate the separate DICR edge bridge;
4. enter CPU service only if `I_STAT & I_MASK` is nonzero;
5. return without recapturing current DMA state.

For token 2, the hardware service:

1. validates active state and exact token before reading payload data;
2. reads all 32 guest words in ascending address order;
3. publishes all 64 pixels, low halfword first;
4. records data visibility order (`3` in this two-transfer fixture);
5. clears DMA active and CHCR busy (`0x01000201 -> 0x00000201`);
6. evaluates completion-flag gating;
7. records completion order (`4`) and increments the event count from 1 to 2.

Thus data order is strictly before completion order. MADR, BCR, and the
captured token identity remain readable; the descriptor is inactive, so a
second service with token 2 is rejected before any mutation. A third
checkpoint sees no active token and returns `IDLE`.

Hardware reset clears the active descriptor/token while preserving the
monotonic event serial. A new transfer receives a distinct nonzero token;
the old token cannot complete it, while the new token completes exactly once.

## DICR gating and physical bit 31

The normal completion-flag condition remains:

```text
(stored_DICR & (master_bit_23 | channel2_enable_bit_18)) ==
    (master_bit_23 | channel2_enable_bit_18)
```

Canonical D entry is `0x00800000`: master is one, channel-2 enable is zero,
all flags are zero, and force bit 15 is zero. Therefore completion does not
create flag 26. Stored DICR stays exactly `0x00800000`.

Physical bit 31 remains derived, never stored:

```text
bit15 || (bit23 && any(bits24..30))
```

It is zero before and after. No false-to-true transition is latched. The
focused contrast proves all three relevant cases independently:

| Controls at completion | Data / CHCR | Stored DICR result | Edge |
| --- | --- | --- | --- |
| master + channel2 enable | complete / idle | `0x04840000` | one rise |
| master only | complete / idle | `0x00800000` | none |
| channel2 enable only | complete / idle | `0x00040000` | none |

An unrelated retained channel-6 flag is preserved byte-for-byte by a
channel-2-disabled completion. An already-high, already-consumed force-bit
level is also preserved without generating a second rising edge. The tests
inspect raw flags and physical bit 31 directly; no incorrect IRQ is cleared
afterward to manufacture the expected endpoint.

## No interrupt, callback, pump, or queue progress

Canonical second completion leaves:

```text
I_STAT                         0x0000 -> 0x0000
I_MASK                         0x0009 -> 0x0009
registered CPU mask            0x0009
CPU source-3 slot              0x80074520
D_800945E6                     0 -> 0
source-3 CPU service entries   +0
func_80074520 entries          +0
pump entries                   +0
worker calls                   +0
DrawSync calls                 +0
producer / consumer            1/1 -> 1/1
DMA callback slot 2            0 -> 0
work marker / DrawSync slot     1/0 -> 1/0
GPUSTAT / VBlank               unchanged
```

The full queue slot and surrounding canaries are byte-identical. No third
DMA is issued. Value-only entry telemetry proves the second checkpoint calls
neither the edge bridge nor the CPU pending-service function. Direct
hardware-phase tests also prove `PE_GPU_ServiceDMA2Completion` contains no
path to IRQ, retail callback, pump, worker, or queue code.

## Admission and run control

Disabling the checkpoint withholds token 2 with CHCR busy and pixels
invisible. Re-enabling it admits exactly that token. A pre-latched frame-limit
stop neither suppresses completion nor changes its stop epoch/reason. The
checkpoint does not consult sticky stop state during already-admitted
hardware work, but it also does not execute caller continuation or another
hardware token.

The first C checkpoint and the second D checkpoint are separate invocations.
C's retained regression proves the first invocation cannot recapture the DMA
issued by its callback. D proves the later invocation captures token 2 once,
completes it, and that the following invocation is idle.

## Production integration decision

**Decision B, bounded second admission.** The existing B2 checkpoint remains
the single scheduler surface. At its existing host-safe continuation, the
caller now makes two explicit calls: call one admits the first token and call
two is allowed only after call one returns normally. Either nested boundary
returns immediately, while an idle/stale first call does not admit a second.
There is no loop, periodic emulator service, status-read progress, wall clock,
or second checkpoint implementation. Consequently the canonical path can
service token 1 and token 2, but cannot query or service a third token before
the named B50 stop.

The checkpoint itself also gains value-only capture/service telemetry and
conditional bridge/CPU-helper entry. These counters do not participate in
hardware authority or scheduling; they prove the canonical D delta is one
query, one service of the captured token, zero bridge calls, and zero CPU
service calls.

## Tests and oracle

Eight focused native groups cover:

- exact post-C token/geometry/register/queue entry;
- all 64 pixels, source stability, VRAM canaries, and data-before-completion;
- direct hardware-only completion and a separate later checkpoint;
- no flag26/bit31/source3/CPU/DMA callback/pump/worker/DrawSync progress;
- queue, callback, marker, mask, dispatcher, GPUSTAT, VBlank, and stop inertia;
- withheld admission, sticky stop, replay rejection, reset/reissue safety;
- enabled/channel-disabled/master-disabled contrasts, unrelated flags, force;
- three deterministic repeats and an immediately following idle checkpoint.

`tools/b53i_d_oracle.py` verifies the executable SHA-1 and runs 15 independent
state scenarios. It has explicit token, DICR control, source, destination,
VRAM, queue, callback, mask, pump, and worker state; it imports no production
C and has no automatic hardware evolution or interrupt dispatch.

Final preservation results:

```text
native suite, fresh normal                 566/566
native suite, fresh ASan/UBSan             566/566, zero diagnostics
focused B53I-D, normal/sanitizer             8/8 each
retained B53I-C, normal/sanitizer            10/10 each
retained B53I-B2, normal/sanitizer           15/15 each
retained B53I-B1, normal/sanitizer             8/8 each
retained B53H, normal/sanitizer                8/8 each
corrected B53B, normal/sanitizer              15/15 each
B53I-D oracle                                15/15 scenarios
oracle/executable matrix                      46/46
retained oracle subset                        33/33
B49 host-loop, normal/sanitizer               PASS / PASS
```

Preserved hashes:

```text
framebuffer SHA-256  fb28dc21dd1e41eb72b8fe22dd3295bb8ed0c040aa88f7885a68dedc2629dfdb
real-data FNV-1a64   7D860391E1ED6C97
matching EXE SHA-1   452fb033f2eaa4b18aa20a5bca60b8125af3a37b
matching EXE SHA-256 5d94938ee752e81ef375bd4493c9883850c25a86895f9cb0732cf3622b44351b
```

## Lifecycle classification and frontiers

**TWO-LOADIMAGE ASYNCHRONOUS GPU LIFECYCLE VERIFIED.**

First LoadImage cycle: issue, defer, explicit completion, DICR/source-3 IRQ,
CPU/DMA callback dispatch, queue pump, and second issue.

Second LoadImage cycle: issue, defer, separate later token completion, VRAM
visibility and CHCR idle, with no IRQ because channel-2 callback/enable was
removed before issue.

The global continuing frontier remains the explicit B50 boundary:

```text
func_8006AD40_prefix_cut from func_8006AD40, retail PC 0x8006AE50
```

Bootstrap strict remains:

```text
func_8007F72C from func_800698D4
```

The exact next task is a fresh **read-only audit** of the `func_8006AD40`
suffix beginning at `0x8006AE50`, using the now-richer post-GPU/IRQ state to
choose a new bounded split. B53I-D does not translate or enter that suffix.
