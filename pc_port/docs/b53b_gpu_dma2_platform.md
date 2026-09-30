# Phase 6E-B53B — deterministic GPU/DMA2 platform substrate

> **B53I-B2 supersession note:** B53B intentionally used a bounded
> unconditional DMA2 completion-flag model because interrupt delivery was
> outside its scope. Retail/hardware recovery in B53I-A/B2 proves that a
> normal completion creates flag 24+n only when that channel's DICR enable
> and master bit 23 are set at completion. B53I-B2 deliberately corrects
> that one expectation, adds derived read-only bit 31 and a separate rising-
> edge bridge, and retains every other B53B substrate contract.

> **B54K-R extension note:** B54K-R adds the generic synchronous GP0(80h)
> VRAM-to-VRAM MoveImage operation and translates the exact one-packet
> `func_80076B98` path used by PsyQ MoveImage. General linked-list DMA,
> DrawOTag interpretation, StoreImage, and unrelated GP0 commands remain
> unsupported. The scope statements below describe the historical B53B rung.

> **B54K-S extension note:** B54K-S translates the execution-proven
> `func_80077294` DrawSync behavior. Blocking waits admit one active DMA2
> token through the established checkpoint owner between retail wait polls;
> polling reads themselves remain inert. Timeout recovery and no-progress
> states remain explicit named cuts.

## Verdict and scope

B53B implements the native hardware authority designed in B53A. It does not
translate `func_80076C34`, `func_80076EE4`, `func_80076664`, DrawSync, the
bounded wait functions, or any DMA interrupt dispatcher. The B52 LoadImage
wrapper still reaches the centralized unresolved boundary before any GPU
hardware API is called.

Accurate milestone wording is:

> deterministic GPU/DMA2 substrate implemented; retail queue dispatcher
> remains the strict frontier.

This is not a general PlayStation emulator and does not claim that LoadImage
is fully implemented.

## Changed files

| File | B53B change |
|---|---|
| `platform/pe_gpu.h` | bounded public hardware API and read-only snapshot |
| `platform/pe_gpu.c` | single VRAM/GPUSTAT/GP0/GP1/DMA2/DPCR/DICR/VBlank authority |
| `platform/pe_libetc.c` | include GPU platform initialization in host SDK reset |
| `CMakeLists.txt` | compile the module into production and native-test targets |
| `tests/test_native.c` | 15 focused B53B tests covering the 52 requested platform cases |
| `README.md` | current B53B milestone and measured gates |
| `docs/shim_inventory.md` | hardware authority entry while retaining the unresolved dispatcher |
| `../../docs/ai_context/ACTIVE_HANDOFF.md` | current phase, gates, and B53C boundary |
| `docs/b53b_gpu_dma2_platform.md` | this evidence |

No retail function implementation or bootstrap resolver changed.

## Single-authority model

`pe_gpu.c` owns one private `PeGpuAuthority` containing:

- one `1024 * 512` array of `uint16_t` PSX VRAM pixels;
- the modeled GPUSTAT ready bit;
- one bounded GP0 image parser/cursor;
- the proven GP1 DMA-direction subset;
- raw DMA2 MADR, BCR, and CHCR;
- raw DPCR and the bounded DICR state;
- one active hardware DMA descriptor;
- a monotonic completion-event token and causal-order counters;
- a deterministic 32-bit VBlank/VSync counter.

There is no host-side retail command queue. The module has no producer,
consumer, 64-entry ring, worker identity, copied packet, queue marker, or
DrawSync callback field. Tests poison and snapshot the complete retail ring
at `0x800BD030..0x800BE82F`, its indices, marker, the DMA2 callback slot, and a
VBlank callback word. Representative GPU operations including DMA2 issue and
explicit completion leave them unchanged.

The pending DMA descriptor contains only `active`, a 32-bit `pe_addr_t`
source, word count, and event token. Destination progress is the stable GP0
image cursor. It contains no host pointer, RECT pointer, payload copy, or
transient caller storage. Guest RAM is read only when the explicit completion
event is serviced.

## VRAM ownership and addressing

VRAM is one private flat array of 524,288 16-bit pixels. It is independent
of the existing 320x240 RGB HostFB presentation/test surface; neither module
includes, aliases, projects, or updates the other in B53B.

Image pixel ordinal `i` maps as:

```text
x = (origin_x + (i % width)) & 1023
y = (origin_y + (i / width)) & 511
```

X and Y therefore wrap independently. No draw-area clipping is applied.
The public VRAM accessor is read-only, checks `x<1024`, `y<512`, and does not
expose the backing pointer.

`PE_GPU_Init` is the explicit host-lifecycle initialization that clears VRAM
and hardware state. `PE_GPU_Reset` cancels hardware state but preserves VRAM.
The supported GP1 reset commands also preserve VRAM.

## GPUSTAT subset

Only the execution-proven bit is represented:

```text
bit 26 = 0x04000000 = ready to receive a GP0 command word
```

The initial/reset state is ready. `PE_GPU_SetReady` provides an explicit,
deterministic transition. Repeated `PE_GPU_ReadStatus` calls are inert: they
do not advance VBlank, service DMA, alter the parser, or make the GPU ready.
GP0 writes are rejected while the bit is clear. GPU readiness remains
independent of DMA busy/completion and VBlank progression.

## GP0 parser subset

The supported idle commands are exact:

- `0x01000000`: the cache operation emitted before LoadImage A0;
- `0xA0000000`: CPU-to-VRAM image load.

The A0 parser states are `idle -> position -> size -> image data`. Position
uses raw low/high 16-bit X/Y. Size accepts only the B53A platform path's
`width=1..1024`, `height=1..512`; zero or out-of-subset geometry is rejected
and the parser returns to idle.

Each image-data word consumes its low halfword first and its high halfword
second. If only one pixel remains, the high padding halfword is ignored.
CPU-fed prefix/remainder pixels become visible synchronously because retail
has already supplied them to GP0 before the worker returns. Once DMA owns the
suffix, external GP0 data writes are rejected until completion or reset.
Unsupported idle GP0 commands return failure without pretending success.

## GP1 subset

The exact supported commands are:

| Value | B53B effect |
|---:|---|
| `00000000` | reset represented GPU control/parser; DMA direction 0; preserve VRAM |
| `01000000` | reset command parser only; preserve DMA direction and VRAM |
| `02000000` | accept GPU-IRQ acknowledgement; no GPU IRQ state is represented |
| `04000000` | DMA direction off |
| `04000002` | DMA CPU-to-GP0 direction |

Unknown commands return failure. GP0 `01000000` and GP1 `01000000` are
deliberately separate operations.

## DMA2 issue and pending representation

`PE_GPU_DMA2Issue(madr,bcr,chcr)` validates the entire request before changing
registers or the descriptor. It requires:

- no existing active DMA;
- an active A0 image-data cursor;
- GP1 direction 2;
- DPCR channel-2 enable bit `0x00000800`;
- exact CHCR `0x01000201`;
- four-byte-aligned, nonzero guest MADR;
- BCR low half `0x0010` and nonzero upper-half block count;
- 64-bit-safe `blocks*16` words and `words*4` bytes;
- the exact word count `ceil(remaining_pixels/2)`;
- the complete DMA source span passing `PE_RangeIsRam`.

Requiring DPCR enable is the bounded platform start invariant selected for
B53B. B53A proves the bit is enabled before this path but does not separately
exercise disabled-channel hardware.

Successful issue stores the exact raw MADR/BCR/CHCR, records the guest source
and word count, allocates a nonzero monotonic event token, and leaves CHCR bit
24 set. It does not read source data, change VRAM suffix pixels, set DICR
completion, invoke a callback, pump the retail ring, advance VBlank, or change
GPU readiness. Raw DMA register getters exist for later translated MMIO
reads.

This API sees the post-CPU-prefix MADR. B53C must separately validate the
original LoadImage source and its complete rounded span before performing any
CPU-prefix `lw`; DMA MADR validation alone cannot detect an original-source
addition that wrapped before reaching the platform API.

## Explicit completion ordering (historical B53B contract)

The ordering below remains valid. Its unconditional flag-creation sentence
is superseded by the B53I-B2 banner above: step 5 now occurs only when the
channel enable and DICR master were set at completion.

Only `PE_GPU_ServiceDMA2Completion(event_token)` advances DMA. A zero, stale,
or already-consumed token returns without mutation. On a matching event:

1. read each pending word from the saved 32-bit guest address;
2. write low/high pixels through the active A0 cursor;
3. record completed VRAM visibility;
4. clear only CHCR busy bit 24 (`01000201 -> 00000201`);
5. set DICR channel-2 completion flag bit 26;
6. record completion-event order and increment the event count.

Thus completed pixels are observable before completion pending becomes the
later retail dispatcher's responsibility. B53B does not acknowledge DICR,
invoke `func_80076EE4`, or fire DrawSyncCallback.

## DPCR and DICR subsets (historical B53B model)

DPCR is represented as a raw 32-bit word. Enabling channel 2 ORs
`0x00000800`, preserving every other stored bit.

DICR represents its lower 24 control bits plus completion flags 24..30. A
write replaces the lower 24 bits and applies W1C to written completion flags.
For channel 2:

- master enable is bit 23 (`0x00800000`);
- channel enable is bit 18 (`0x00040000`);
- completion pending is bit 26 (`0x04000000`).

At B53B, completion set bit 26 regardless of enables and the bounded assertion
query required all three enable/master/flag bits. B53I-B2 supersedes that
model: enable+master gate flag creation, then physical bit 31 is derived as
force15 or master23 plus any retained flag24..30. Acknowledgement still writes
bit 26 while carrying the current lower 24 bits, so unrelated represented
controls are preserved. No callback lives inside the DICR authority.

## Reset and stale-event safety

Both host lifecycle operations cancel an active descriptor, restore idle CHCR
`0x00000401`, clear DICR completion/control, reset the parser, set GPU ready,
and reset VBlank/order diagnostics. `PE_GPU_Init` additionally clears VRAM;
`PE_GPU_Reset` preserves it.

The internal event serial survives reset. Each issue receives a new token,
so an event captured before reset cannot complete a newer post-reset DMA.
Tests cover reset while busy, service after reset, reset then new issue, stale
old-token service, repeated reset, repeated completion, and sequential new
transfers.

## VSync/VBlank substrate

`PE_GPU_VSyncQuery` is an inert 32-bit counter read. `PE_GPU_VBlankStep`
increments it exactly once. There is no wall-clock source, background thread,
or automatic tie to HostFB VSync, DMA completion, or GPU readiness. Tests can
hold and advance all three state dimensions independently.

## Unsupported functionality

B53B intentionally does not provide:

- the retail libgpu ring, submitter, consumer, queue pump, or callbacks;
- `func_80076C34`, `func_80076EE4`, `func_80076664`, DrawSync, or wait-loop translations;
- DMA IRQ dispatch or guest callback binding;
- StoreImage, MoveImage (added later by B54K-R), linked-list DMA, display
  readback, or DMA directions other than 0/2;
- polygon, line, sprite, environment, ordering-table, or general GP0 commands;
- a general GP1/GPUSTAT/DMA controller;
- HostFB projection, rendering, or presentation changes;
- automatic progress from polling, VBlank, wall time, or a background thread.

Unsupported GP0/GP1 commands and malformed DMA issues return deterministic
failure rather than silently succeeding.

## Test matrix

The 15 B53B tests cover every platform case requested by the rung:

| Test | Covered contract |
|---|---|
| reset/dimensions/initial VRAM | cases 1-4; explicit init vs reset/GP1 VRAM preservation |
| GPUSTAT manual progress | cases 5-8, repeated inert polling, not-ready GP0 rejection |
| GP1 subset/parser reset | both command spaces, exact supported GP1 values, unknown rejection |
| GP0 parsing/pixel order | cases 9-14, parser phases, low/high order, odd padding |
| VRAM XY wrapping | cases 15-17 |
| CPU-only/15-word threshold | cases 18-19 |
| exact 16-word DMA issue | cases 20, 22-27; raw registers, no issue-time visibility/event, busy reissue rejection |
| remainder plus DMA visibility | cases 21, 28-31; live guest data, causal order |
| DICR W1C/preservation | cases 32-34; historical enable-off flag expectation corrected by B53I-B2 while W1C/preservation remains frozen |
| DPCR preservation | cases 35-36 |
| sequential/reset/stale event | cases 37-39; token rejection and exactly-once service |
| guest range/pointer safety | cases 40-44 and 52; exact RAM end, malformed modes, 32-bit address storage |
| VBlank/DMA/ready independence | cases 45-50 |
| maximum geometry | case 51: `1024x512`, `0x40000` words, BCR `40000010`, exact 1 MiB span |
| authority separation guard | no retail-ring, callback, stub, or HostFB mutation/alias |

B53A's remaining queue-full, ring-wrap, consumer-advance, DrawSync, and
callback-order acceptance cases are intentionally deferred to B53C. Modeling
them in B53B would duplicate or translate the retail software authority.

## Verification

- normal native suite: 483/483;
- fresh ASan/UBSan suite: 483/483;
- all 15 B53B tests deterministic in normal and sanitizer builds;
- B52, B51, corrected B50, and 33 retained standalone oracles: pass;
- retained B49 normal and sanitizer harnesses: pass;
- canonical Disc 1 continuing strict remains `func_80076C34` from
  `func_8007506C`;
- bootstrap strict remains `func_8007F72C` from `func_800698D4`;
- framebuffer SHA-256 remains
  `fb28dc21dd1e41eb72b8fe22dd3295bb8ed0c040aa88f7885a68dedc2629dfdb`;
- real-data FNV-1a64 remains `7D860391E1ED6C97`;
- rebuilt retail executable remains SHA-1
  `452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.

## B53C boundary

B53C may target `func_80076C34` and translate only the minimum retail
dispatcher/ring/pump path needed by the execution-proven LoadImage call. It
must keep the guest ring authoritative and call this substrate for GPUSTAT,
GP0/GP1, DMA2, DPCR/DICR, and explicit events. It must separately validate
the original source before CPU-prefix reads. StoreImage, drawing primitives,
general GPU emulation, unrelated callers, and later strict frontiers remain
out of scope.
