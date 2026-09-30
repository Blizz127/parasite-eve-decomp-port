# Phase 6E-B53I-A — DMA2 completion and source-3 DMA IRQ contract

> Retail instruction words and byte strings in this file are elided (`xxxxxxxx`, `<N bytes …>`): no game data is committed. Read them from your own disc (`tools/extract/disc_cache.py`, EXE `SLUS_006.62`).

## Verdict and scope

**Architecture decision: B — TWO-STAGE IMPLEMENTATION.**

B53I-A is an evidence-only rung. It recovers the asynchronous path that must
exist between the first canonical LoadImage DMA2 issue and re-entry into
`func_80076EE4` with DMA idle. It does not service that canonical transfer,
translate `func_80074520`, add I_STAT, dispatch an interrupt, consume the GPU
queue, or translate the idle pump.

The required future path has two independently observable service phases:

```text
explicit DMA hardware completion
  -> VRAM/CHCR/DICR state
  -> source-3 I_STAT edge

later CPU IRQ service
  -> source-3 callback func_80074520
  -> DMA-channel callback func_80076EE4
```

Completing DMA and invoking a callback in one host operation would violate the
retail ordering proved below.

## Starting lineage and collision guard

Git resolves the handed-off short B53H hash to:

```text
B53H HEAD  a470422d1accdae39a1d8cb4b501df599df9003c
           Phase 6E-B53H: translate GPU pump busy-DMA prefix
B53G HEAD^ 0eae8ad5c957f7ff5199eeae9cfad46eb071ce4e
```

The branch is `phase6e-b-provider-frontier`. Before audit work, both status
forms were empty. The process/cwd census found no other checkout owner, Git
lock, build, test, emulator, or writer. The investigations were read-only and
did not advance the accepted pending DMA.

The retail executable used for every literal comparison is:

```text
SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
```

Both `build/extracted/disc1/SLUS_006.62` and
`build/disc1.candidate.exe` match that identity.

## Reproved B53H frontier

Fresh normal and sanitizer Disc 1 runs remain at a translated-prefix
boundary, not a successful strict boot and not a missing provider:

| Field | Measured value |
| --- | --- |
| Boundary | `func_8006AD40_prefix_cut` |
| Boundary owner/caller | `func_8006AD40` |
| Exact retail cut PC | `0x8006AE50` |
| Prefix extent | 68 / 391 words |
| Registry type | `BOOTSTRAP_RET` |
| Stop reason | `PE_PORT_STOP_UNRESOLVED_BOUNDARY` (`5`) |
| Stop epoch | `0 -> 1` at the new request |
| Strict exit | `1` |

The monotonic B53H `PE_Port_StopEpoch()` machinery remains intact. The sticky
first reason is not used to infer whether a new callee boundary occurred.

Bootstrap strict is unchanged:

```text
func_8007F72C from func_800698D4, BOOTSTRAP_RET, exit 1
```

Neither result is described as genuine successful boot.

## Current accepted state

The canonical state at the B53H frontier is:

```text
first DMA2:
  MADR = 0x8012A8B8
  BCR  = 0x00400010
  CHCR = 0x01000201
  DPCR = 0x33333B33

GPU/DMA:
  DMA2 active              = 1
  DMA2 completion pending  = 0
  GPUSTAT ready bit 26     = 1
  DICR                     = 0x00840000

CPU/event:
  native I_MASK            = 0x0000
  native I_STAT authority  = absent
  VBlank                   = 0

DMA callback table:
  D_800956C0[2]            = 0x80076EE4

GPU queue:
  producer                 = 1
  consumer                 = 0
  second entry             = present and byte-identical
```

The last three CPU facts are a current-port discrepancy, not authentic retail
initialization: the full retail ResetCallback registers sources 0 and 3 and
returns with I_MASK `0x0009`.

## `PE_GPU_ServiceDMA2Completion` audit

The B53B implementation is at `platform/pe_gpu.c`. Its initial validation is
exactly:

```c
dma2_active && event_token != 0 &&
event_token == current_dma2_event_token
```

A failed validation returns before any guest read or mutation. Given a valid
descriptor produced by `PE_GPU_DMA2Issue`, the exact operation order is:

1. Reload the saved 32-bit guest source address.
2. Read each saved transfer word from guest RAM.
3. Write its low and high pixels into authoritative VRAM.
4. Increment `image_current_pixel` and decrement
   `image_remaining_pixels` per pixel.
5. On the final pixel, set `gp0_state = PE_GPU_GP0_IDLE` and retain
   `image_remaining_pixels = 0`.
6. Verify that no pixels remain.
7. Set `dma_data_order = ++order_counter`.
8. Set `dma2_active = 0`.
9. Clear only CHCR start/busy bit 24.
10. OR DICR channel-2 flag bit 26.
11. Set `dma_completion_order = ++order_counter`.
12. Increment `dma_event_count` and return `1`.

For the canonical descriptor, CHCR therefore changes exactly:

```text
0x01000201 -> 0x00000201
```

Every mutated field is accounted for:

| State | Mutation |
| --- | --- |
| VRAM pixels | source words become visible, low half then high half |
| `image_current_pixel` | increments once per pixel |
| `image_remaining_pixels` | decrements to zero |
| `gp0_state` | image-data to idle on the final pixel |
| `order_counter` | increments twice |
| `dma_data_order` | receives the first new order |
| `dma2_active` | one to zero |
| `dma2_chcr` | bit 24 clears |
| `dicr` | bit 26 sets |
| `dma_completion_order` | receives the second new order |
| `dma_event_count` | increments once |

It does **not** change MADR, BCR, the saved source/count/token, DPCR,
GPUSTAT/readiness, GP1 DMA direction, VBlank, I_MASK, any callback table, or
any retail queue field.

There is one private-invariant caveat: the remaining-pixel check occurs after
the word loop. Corrupt internal state could therefore cause a zero return
after pixel/parser mutation but before active/CHCR/DICR/order mutation. Public
issue validation proves that state unreachable for a valid descriptor.

There is no callback dependency, callback lookup, indirect call, or production
caller in this function or file. At B53H HEAD the completion, assertion, and
acknowledgement APIs have definitions and tests but no production call site.
Direct callback delivery from this service would be a new contract violation;
none exists now.

## DICR completion and physical delivery

### Canonical values

The accepted pre-completion word is:

```text
0x00840000 = master enable bit 23 | channel-2 enable bit 18
```

Current B53B completion stores:

```text
0x04840000 = prior controls | channel-2 flag bit 26
```

Physical guest readback must additionally expose derived read-only master flag
bit 31:

```text
0x84840000
```

The exact canonical phases are:

| Phase | CHCR | Stored DICR | Physical DICR read | IRQ3 edge |
| --- | --- | --- | --- | --- |
| Busy | `0x01000201` | `0x00840000` | `0x00840000` | none |
| Hardware completion returned | `0x00000201` | `0x04840000` | `0x84840000` | false-to-true; latch I_STAT bit 3 |
| Retail channel-2 W1C | `0x00000201` | `0x00840000` | `0x00840000` | line deasserted |
| GPU reset | `0x00000401` | `0x00000000` | `0x00000000` | none |

`PE_GPU_ReadDICR()` currently returns only stored state, so it exposes
`0x04840000` after completion. This is sufficient for the B53G setter and the
channel scan, but insufficient for a complete literal `func_80074520`, whose
tail consumes bit 31.

### Bit-31 verdict

**Bit 31 must be derived on guest-visible DICR reads for actual delivery and a
literal dispatcher translation. It must not be stored or writable.**

B53G remains correct: `func_800746A0` reads bit 31 only into a value that is
immediately masked with `0x00FFFFFF`, making it unobservable in that setter.
B53I is different. `func_80074520` explicitly compares the DICR high byte with
`0x80000000` at `0x80074608..0x80074618`.

The executable proves consumption; PSX hardware documentation corroborates
the controller relationship:

```text
flag[24+n] is latched on enabled/master-enabled channel n completion
bit31 = bit15 || (bit23 && any already-latched flag[24..30])
I_STAT IRQ3 is set on a false-to-true transition of bit31
```

The current bounded B53B model deliberately differs outside this canonical
all-enabled case: it always sets flag 26, defines its query as
master+channel-enable+flag, does not synthesize bit 31, and does not latch
I_STAT. Those differences do not alter the exact canonical values above, but
B53I-B2 must replace that bounded completion rule with the recovered physical
rule: a normal channel `n` completion latches flag `24+n` only if both channel
enable `16+n` and master enable 23 are set at completion. Bus-error/force
behavior remains a separately tested path. The current level query is not CPU
delivery.

This is required by the canonical repeated lifecycle, not an optional general
DMA extension. Stage 16 removes channel-2 enable before stage 18 starts the
second DMA. Its later hardware completion must still make pixels visible and
clear CHCR, but must not create flag 26, a bit-31 edge, or a second source-3
IRQ. Retaining B53B's unconditional flag creation would spuriously dispatch an
IRQ after the queue callback has deliberately deregistered itself.

B53I-A does not change the accepted B53B source or its frozen tests. B53I-B2
must revise the one bounded enable-off expectation deliberately, with the
hardware evidence and new disabled-channel/master-off regressions recorded,
while preserving the remainder of the B53B platform contract.

Derived bit 31 and its false-to-true edge must be recomputed on every
DICR-affecting transition, not only on DMA completion. In particular, a DICR
write can disable master while retaining a flag and later re-enable master,
creating a new edge from the retained flag; bit 15 can also force the derived
flag. Such a write may latch I_STAT source 3, but it still must not dispatch a
callback synchronously.

Hardware corroboration:

- <https://psx-spx.consoledev.net/dmachannels/#1f8010f4h-dicr-dma-interrupt-register-rw>
- <https://psx-spx.consoledev.net/interrupts/>

### W1C acknowledgement

Current `PE_GPU_WriteDICR(value)` faithfully implements the represented write
semantics:

```text
stored lower 24 bits = value lower 24 bits
stored flags 24..30 &= ~(value flags 24..30)
bit 31 is ignored
```

`func_80074520` constructs the channel-2 write from a fresh read as:

```text
fresh_DICR & 0x04FFFFFF
```

For either stored `0x04840000` or physical `0x84840000`, the exact write is
`0x04840000`. W1C then leaves stored/visible DICR `0x00840000`, preserving all
lower controls and unrelated completion flags. This occurs before callback
slot 2 is loaded.

## Complete recovery of `func_80074520`

### Identity

| Property | Value |
| --- | --- |
| Start | `0x80074520` |
| Inclusive final byte | `0x8007469F` |
| Exclusive end | `0x800746A0` |
| File offset | `0x64D20..0x64E9F` |
| Size | `0x180` / 384 bytes |
| Instructions | 96 |
| Body SHA-256 | `3dd9a2f9f85757a6e5c28f1fa6f48cf929370c6ad4da3ae8c08e08dc852259cd` |
| Split source | `asm/disc1/64CC8.s` |

The next verified word, `0x00803021` at `0x800746A0`, begins
`func_800746A0`.

### ABI, callers, and callback ABI

The best-supported semantic ABI is:

```c
void func_80074520(void);
```

The body reads no incoming `a0..a3`. Every ordinary terminating path happens
to leave `v0 = 0`, but its upstream IRQ caller ignores it. An aligned census
finds no direct `jal` and no literal data word `0x80074520`; the sole standard
address materialization is at `0x800744F8/FC`, where `func_800744D4` installs
it as the CPU source-3 handler. Runtime execution is indirect from
`func_80073F00`.

DMA-channel callbacks are nominally no-argument and their return is ignored.
At the literal `jalr`, `a0` happens to hold the DICR pointer, normally
`0x1F8010F4`; `a1..a3` are residue. No channel index is passed. Canonical
`func_80076EE4` ignores all four argument registers.

### All literal words

```text
80074520: xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx
80074530: xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx
80074540: xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx
80074550: xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx
80074560: xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx
80074570: xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx
80074580: xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx
80074590: xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx
800745A0: xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx
800745B0: xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx
800745C0: xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx
800745D0: xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx
800745E0: xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx
800745F0: xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx
80074600: xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx
80074610: xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx
80074620: xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx
80074630: xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx
80074640: xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx
80074650: xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx
80074660: xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx
80074670: xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx
80074680: xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx
80074690: xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx
```

### Every transfer and delay slot

| Site | Transfer | Delay slot |
| --- | --- | --- |
| `0x80074558` | `beqz s1,0x800745FC` | `nop` |
| `0x80074574` | `beqz s1,0x800745D8` | `s0 = 0` |
| `0x80074584` | `beqz v0,0x800745D8` | `v0 = s1 & 1` |
| `0x8007458C` | `beqz v0,0x800745C8` | `v0 = s0 + 24` |
| `0x800745B8` | `beqz v0,0x800745C8` | `nop` |
| `0x800745C0` | `jalr v0` | `nop` |
| `0x800745D0` | `bnez s1,0x80074580` | `s0++` |
| `0x800745F4` | `bnez s1,0x80074574` | `nop` |
| `0x80074618` | `beq v0,v1,0x80074634` | `nop` |
| `0x8007462C` | `beqz v0,0x8007467C` | `nop` |
| `0x80074640` | `jal 0x80071A74` | `s0 = 0` |
| `0x80074668` | `jal 0x80071A74` | `s0++` |
| `0x80074674` | `bnez v0,0x80074648` | `nop` |
| `0x80074698` | `jr ra` | `sp += 0x30` |

There are no branch-likely instructions.

### Exact DMA dispatch algorithm

The DMA DICR pointer word is `D_800956BC = 0x1F8010F4`. The DMA callback
table is eight 32-bit guest identities at `D_800956C0..D_800956DF`; the
dispatcher scans only channels 0 through 6.

Initial and repeated pending extraction is exactly:

```c
pending = (ReadDICR() >> 24) & 0x7F;
```

Consequences proved directly by the CFG:

- only DICR bits 24 through 30 enter the scan;
- bit 31, enable bits, and master enable do not enter selection;
- channels scan low-to-high, 0 through 6;
- a disabled channel flag can be selected if this function is entered for
  some other reason because selection does not intersect enable bits;
- one nonzero snapshot is drained before live pending is resampled.

For each set snapshot bit `i`, retail performs:

```c
d = ReadDICR();
WriteDICR(d & (0x00FFFFFFu | (1u << (24 + i))));
callback = D_800956C0[i];
if (callback != 0) callback();
```

This order is strict:

1. fresh DICR read;
2. selected channel W1C write;
3. live callback-slot load;
4. null test;
5. optional indirect callback;
6. advance snapshot/channel.

A zero slot is therefore still acknowledged and drained. Callback return is
unused. After the pass, DICR is reread and the scan restarts at channel 0
until no flags remain. Multiple flags dispatch in one invocation; flags
raised during callbacks are seen on a later resample; a continually
reasserted flag can prevent return.

The exact acknowledgement nesting for canonical DMA2 is:

```text
CPU dispatcher clears I_STAT bit 3
  -> func_80074520 writes DICR 0x04840000
  -> W1C clears DICR bit 26
  -> func_80074520 loads D_800956C8
  -> func_80076EE4 executes
```

The post-loop diagnostic performs separate volatile reads. If
`(DICR & 0xFF000000) == 0x80000000`, it branches **into** the bus-error
diagnostic. Otherwise it rereads and also enters the diagnostic if bit 15 is
set. The diagnostic prints the fresh DICR and MADR for channels 0 through 6.
Canonical post-ack high byte and bit 15 are zero, so it returns normally.

The complete DICR MMIO census is:

| PC | Operation |
| --- | --- |
| `0x80074548` | initial pending read |
| `0x800745A0` | fresh per-selected-channel acknowledgement read |
| `0x800745AC` | per-selected-channel W1C write |
| `0x800745E4` | end-of-pass pending resample |
| `0x80074608` | post-dispatch high-byte diagnostic read |
| `0x80074620` | conditional bit-15 diagnostic read |
| `0x8007463C` | fresh DICR argument read on the diagnostic path |

## CPU source-3 installation chain

### Exact wrapper and backend

`func_80073CC4` is completely bounded:

| Property | Value |
| --- | --- |
| Range | `0x80073CC4..0x80073CF3` (exclusive `0x80073CF4`) |
| File offset | `0x644C4` |
| Size | `0x30` / 12 words |
| SHA-256 | `ff71c9ce2b4ad8e8d5afb17da0e186ee953dabddd306f06499c40fc17b3c879e` |

Its ABI is:

```c
pe_addr_t func_80073CC4(uint32_t source, pe_addr_t new_handler);
```

It loads `(*D_8009566C) + 8`, calls that target with the original `a0/a1`,
and returns the backend result. Retail data is:

```text
D_8009566C -> D_8009564C
D_8009564C + 8 = D_80095654 = 0x800740D0
```

The backend `func_800740D0` is `0x800740D0..0x80074217`, file `0x648D0`,
size `0x148`, SHA-256
`a6991559f3a0292a07423fdaf10d16d6da15eba33543f02dd1e1041aa5715332`.
It returns the previous full 32-bit handler identity.

For source `n`, its four coupled locations are:

```text
CPU handler slot        D_800945E8[n]
registered-source mask  D_80094614 bit n
I_MASK                  bit n
I_STAT                  bit n (dispatcher, not setter)
```

The setter has no range check. If the new identity equals the old one, it
returns immediately without repairing masks. If the ResetCallback guard is
zero, it also refuses mutation. Otherwise it:

1. saves the 16-bit I_MASK;
2. writes I_MASK zero;
3. writes/clears the guest handler slot;
4. sets/clears the bit in the saved mask and `D_80094614`;
5. restores the updated I_MASK;
6. returns the previous handler.

It never accesses I_STAT and never dispatches a callback.

That summary is the exact source-3 path. `func_800740D0` contains additional
SDK side calls for sources 0, 4, 5, and 6, all bypassed by source 3:

```text
source 0: func_80073C74(handler == 0)
          func_80073C84(3, handler == 0)
source 4: func_80073C84(0, handler == 0)
source 5: func_80073C84(1, handler == 0)
source 6: func_80073C84(2, handler == 0)
```

The first is a BIOS B(5Bh) trampoline and the second is the BIOS C(0Ah)
trampoline used for ChangeClearRCnt. B53I-A does not claim a full all-source
translation of the backend.

### Exact source-3 call

`func_800744D4` is `0x800744D4..0x8007451F`, file `0x64CD4`, size
`0x4C`, SHA-256
`e293c6aa8850c852f562fee7c52c855d8b020fad9669159721d8b8e4b152a1be`.
It:

1. clears all eight DMA callback identities at `D_800956C0`;
2. materializes `a0 = 3` and `a1 = 0x80074520`;
3. writes zero to DICR in the `jal func_80073CC4` delay slot, clearing lower
   controls/master/enables but preserving any existing W1C flags 24..30;
4. executes `func_80073CC4(3, func_80074520)`;
5. returns the DMA-channel setter `func_800746A0` for the SDK jump table.

The proposed pair is therefore exact; there is no alternative retail pair.
On clean boot the preexisting flags are zero, so the resulting word is zero;
that clean-state observation must not be generalized into a flag-clearing
write.

### Reset owner and authentic final mask

`func_80073E28` is `0x80073E28..0x80073EFF`, file `0x64628`, size
`0xD8`, SHA-256
`2fef9e18b8a89258afceb2e3131dd34738ed0585120282beb8ee269a842b6c61`.

On its first guard-passing call it:

1. writes I_MASK zero;
2. reads that zero and writes it to I_STAT, clearing all status under W0C;
3. writes DPCR `0x33333333`;
4. clears `0x41A` words from `D_800945E4` through `0x8009564B`;
5. sets the one-time guard;
6. invokes `func_800743B4`, which installs
   `func_80073CC4(0, func_8007440C)`;
7. invokes `func_800744D4`, which installs
   `func_80073CC4(3, func_80074520)`.

Direct B53I-B1 recovery corrects one B53I-A audit omission here:
`func_80073E28` calls `func_80074330(D_800945E4, 0x41A)` at
`0x80073E80`, with the word count in the delay slot. `func_80074330` clears
exactly `0x41A` words (`0x1068` bytes), range
`[0x800945E4,0x8009564C)`. This includes `D_800945E8` and
`D_80094614`, and ends exactly before the SDK jump table. The callback table
and registered mask therefore do not merely inherit clean executable zeros:
ResetCallback actively resets them before installing sources 0 and 3. This
strengthens, rather than removes, the same-handler/reset requirement below.

Thus clean retail state at full ResetCallback return is:

```text
I_STAT                  = 0x0000
I_MASK                  = 0x0009
D_80094614              = 0x0009
D_800945E8[0]           = 0x8007440C
D_800945E8[3]           = 0x80074520
```

The current collapsed `func_80073C94` stops after I_MASK zero, DPCR, and an
unrelated VBlank callback-table reset. It omits I_STAT and both CPU source
registrations. Therefore current canonical I_MASK zero is faithful to the
accepted port state but not to full retail initialization.

## CPU interrupt source mapping and dispatcher

Executable data proves:

```text
D_80095670 = 0x1F801070 (I_STAT)
D_80095674 = 0x1F801074 (I_MASK)
```

All relevant accesses are 16-bit `lhu`/`sh`. Source index `n` maps directly
to I_STAT bit `n`, I_MASK bit `n`, registered-mask bit `n`, and
`D_800945E8[n]`. Consequently source 3 is exactly:

```text
I_STAT bit       3 = 0x0008
I_MASK bit       3 = 0x0008
CPU table slot     = D_800945F4
retail identity    = 0x80074520
```

This mapping was derived from the executable before consulting hardware
documentation.

`func_80073F00` is `0x80073F00..0x800740CF`, file `0x64700`, size
`0x1D0`, SHA-256
`4862b3fbc3bb78e49db65b4ebacc3c188f4238d6f4cc9cbd8d2ba3dce59bcace`.
Its core expression is:

```c
pending = D_80094614 & ReadIStat16() & ReadIMask16();
```

It scans sources 0 through 10 low-to-high. For each selected source it writes
`~(1 << source)` as a halfword to I_STAT **before** loading and invoking
`D_800945E8[source]`. Under I_STAT write-zero-to-clear semantics, that clears
only the selected bit. The callback has no defined arguments and its return is
ignored.

Before scanning, `0x80073F68` stores `1` to the dispatch-active halfword
`D_800945E6`. It is cleared only in the delay slot at `0x800740AC` of the
terminal `func_80074384` call. A normally returning dispatcher therefore ends
with zero, but a nested non-return/boundary before that PC faithfully leaves
the guest flag equal to one.

After a snapshot pass it rereads the registered mask, I_STAT, and I_MASK and
restarts at source 0. New/reasserted enabled registered sources can therefore
dispatch in the same invocation.

The tail separately watches `I_STAT & I_MASK` sources that have no registered
handler. It eventually logs and clears status after its watchdog threshold.
Fully masked pending does not enter this tail and remains pending.

## I_STAT authority audit and I_MASK semantics

### Current census

The native port has **no I_STAT authority** (case A): no storage, accessor,
assert primitive, W0C write, or reset operation. `platform/pe_irq.c` owns only
one `uint16_t g_i_mask`. DICR completion state, VBlank accounting, SPU DMA,
critical-section depth, and the separate VBlank callback table are not
I_STAT owners.

The CPU source table `D_800945E8[11]` and registered mask `D_80094614` exist
only as passive guest bytes at this HEAD. They must remain guest-backed when
translated; adding native mirrors would create duplicate callback-table
authority.

### Future minimum single authority

Extend the existing `pe_irq` authority, rather than adding a second owner:

```text
uint16_t I_STAT
uint16_t I_MASK (the existing field)
monotonic reset generation

assert_sources(bits): I_STAT |= bits
write_status(value):  I_STAT &= value        // 16-bit W0C
write_mask(value):    I_MASK = value
reset:                I_STAT=0; I_MASK=0; generation++
```

Hardware assertion must be independent of I_MASK. A masked bit remains in
I_STAT. Writing/changing I_MASK does not acknowledge it and does not invoke a
callback. If it is later enabled, the next explicit CPU service opportunity
can dispatch it.

Source registration temporarily masks the CPU but never changes I_STAT. Its
final I_MASK update can make existing pending status eligible, but the setter
itself does not call the dispatcher.

All callback identities remain 32-bit guest addresses. Native code may resolve
a known identity through typed bindings at call time; it must not store a
native function pointer in guest callback state.

Setters accept arbitrary nonzero guest identities. A nonzero identity without
a typed binding must stop at an honest indirect-call boundary after the retail
acknowledgement that precedes its lookup/call. It must never be silently
skipped, cast to a native pointer, or treated as if the callback returned.
Only a literal zero slot takes the retail no-call path. This rule applies to
both CPU-source and DMA-channel tables, and the boundary/non-return signal must
propagate through the enclosing scans and host checkpoint.

## Two callback layers

The executable proves two distinct tables and every arrow in the canonical
chain:

```text
hardware DMA2 completes
  -> DICR channel-2 flag bit 26
  -> derived DICR bit31 false-to-true
  -> I_STAT source bit 3
  -> CPU dispatcher func_80073F00
  -> CPU table D_800945E8[3]
  -> func_80074520
  -> DMA table D_800956C0[2]
  -> func_80076EE4
```

| Layer | Table | Index | Canonical identity | Ack before call |
| --- | --- | --- | --- | --- |
| CPU IRQ source | `D_800945E8[11]` | 3 | `0x80074520` | I_STAT bit 3 |
| DMA channel | `D_800956C0[8]` | 2 | `0x80076EE4` | DICR bit 26 |

Neither table may be collapsed into a host callback registration. The hardware
completion service owns neither table.

## Canonical end-to-end state trace

There are two initial-state qualifications:

- servicing the exact accepted B53H host state can complete hardware and
  latch future I_STAT bit 3, but current I_MASK is zero and source-3 CPU
  registration is absent, so it cannot authentically deliver the callback;
- a future canonical replay must first restore retail ResetCallback state
  (`I_MASK=0x0009`, CPU slot 3 `0x80074520`) and then reach the same
  DMA/queue state before exercising delivery.

With that prerequisite, the exact lifecycle is:

Stages 1 through 14 are the B53I-B2 delivery chain. Stages 15 through 20 are
literal recovery of the later idle-pump rung, not permission for B2 to execute
them. Until that body is translated, B2 must propagate a non-return boundary
between stages 14 and 15 with I_STAT/DICR already acknowledged and the queue
still producer 1 / consumer 0. Because the CPU dispatcher has not reached its
terminal clear, `D_800945E6` remains `1` at this B2 boundary.

| # | Domain | Transition and observable state |
| --- | --- | --- |
| 1 | Hardware | First DMA2 is active: MADR `0x8012A8B8`, BCR `0x00400010`, CHCR `0x01000201`. |
| 2 | Retail C state | Second LoadImage is queued: producer 1, consumer 0, entry intact. |
| 3 | Platform service | An explicit valid-token hardware-service call begins; no polling/read triggered it. |
| 4 | Hardware model | All first-image pixels become visible in VRAM; parser reaches idle. |
| 5 | Hardware model | `dma2_active` clears; CHCR becomes `0x00000201`. |
| 6 | Hardware model | DICR flag 26 sets: stored `0x04840000`, physical read `0x84840000`. |
| 7 | IRQ controller | DICR bit31 false-to-true latches I_STAT bit 3 `0x0008`, regardless of CPU I_MASK. |
| 8 | CPU interrupt | A later `PE_IRQ_ServicePending`-equivalent evaluates registered mask & I_STAT & I_MASK. |
| 9 | Retail CPU dispatcher | Source 3 is selected; I_STAT receives `0xFFF7` before the callback lookup/call. |
| 10 | Retail C translation | CPU slot 3 yields and enters `func_80074520`. |
| 11 | Retail C translation | Pending snapshot is `4`; channels 0 and 1 are skipped, channel 2 selected. |
| 12 | Retail C translation/hardware | It writes exact DICR W1C `0x04840000`; stored DICR returns to `0x00840000`. |
| 13 | Retail C translation | DMA slot 2 is loaded live and yields guest identity `0x80076EE4`. |
| 14 | Retail C translation | `func_80076EE4` re-enters; CHCR bit 24 is now clear, so the busy return is not taken. |
| 15 | Retail idle pump (untranslated) | `func_80073E10(0)` masks CPU IRQs and the pump sees producer 1 / consumer 0. |
| 16 | Retail idle pump | Last-entry/no-DrawSync case calls `func_80073CF4(2,0)` to remove DMA slot 2/enable; DICR changes `0x00840000 -> 0x00800000`. |
| 17 | Retail idle pump | GPUSTAT ready succeeds; ring entry yields worker `0x80076664`. |
| 18 | Retail C translation | Worker issues second LoadImage: MADR `0x8012B8B8`, BCR `0x00020010`, CHCR `0x01000201`. |
| 19 | Retail idle pump | Only after worker return, consumer advances `0 -> 1` at retail PC `0x80077054`. |
| 20 | Retail idle pump | Consumer equals producer; saved I_MASK is restored at `0x8007709C`. Because the second DMA is busy, marker clearing/DrawSync is suppressed; the pump returns zero. |

The second DMA is active after stage 18. Producer remains 1; the entry is not
prematurely cleared. Because the last-entry path removes DMA slot 2 before
issuing that transfer, its later completion does not need a queue-pump
callback for an already drained queue.

The recovered endpoint after exactly one first-DMA event is:

```text
producer / consumer       = 1 / 1
DMA callback slot 2       = 0
work marker               = 1
DrawSync callback          = 0
I_STAT                     = 0
I_MASK                     = 0x0009 (authentic retail setup)
D_80095880                 = 0x00000009
D_800945E6                 = 0 (after the later normal CPU-dispatch return)
DICR stored                = 0x00800000
DMA2 MADR/BCR/CHCR         = 0x8012B8B8 / 0x00020010 / 0x01000201
DMA2 active/pending flag   = 1 / 0
second image pixels left  = 64
second payload in VRAM     = no
```

An event checkpoint must service only its captured first-transfer token. A
loop that services "the current DMA until none remain" would accidentally
complete the newly issued second transfer in the same turn and destroy this
retail endpoint.

## Observable intermediate states

All four queried separations are observable and must remain distinct:

**A. CHCR idle before DICR dispatch:** yes. Completion writes all VRAM pixels,
then clears CHCR, then sets DICR. After service return, CPU dispatch can still
be withheld.

**B. DICR pending while CPU IRQ is masked:** yes. The DICR completion/bit31
edge latches I_STAT independently of I_MASK. I_STAT and DICR remain pending
while I_MASK bit 3 is zero.

**C. CPU source pending before callback:** yes. I_STAT bit 3 exists after the
hardware phase and before the later CPU dispatcher phase.

**D. DMA pending acknowledged while callback executes:** yes. The CPU
dispatcher first clears I_STAT bit 3; `func_80074520` then clears DICR bit 26;
only afterward does it load/call `func_80076EE4`. The callback observes
complete VRAM, idle CHCR, cleared DICR pending, and cleared source-3 I_STAT.

## Deterministic service points

### Hardware completion

Keep `PE_GPU_ServiceDMA2Completion(token)` as a hardware-only operation. Make
it eligible only at an explicit deterministic host hardware-service
checkpoint. Tests must be able to omit that checkpoint or withhold the token.

It must not be triggered by:

- wall clock;
- polling CHCR;
- calling the B53H busy-DMA pump;
- reading GPUSTAT;
- GPU ready state;
- callback registration;
- VBlank alone;
- a stop request.

The smallest current production integration point is the host-safe
continuation in `bootstrap/func_8001220C_port.c` immediately after
`func_8006AD40()` returns and before the following `PE_Port_ShouldStop()`
test. The retail call is at `0x800122C4`; by this continuation the translated
prefix has established the DMA and queue state, but no caller code has
consumed results from the untranslated suffix. A non-strict run can service
the explicit event there and then honor the already-requested prefix stop.
Strict mode still aborts at the named prefix boundary before returning, so
its frontier remains honest.

B53I-B2 should place a narrow, suppressible hardware checkpoint at that
continuation (or route the exact point through an equivalent deterministic
event scheduler). It must not hide completion inside a register read, retail
callback, or generic stop operation. A stop already latched by the prefix must
not cause an admitted hardware/IRQ checkpoint to skip callback cleanup; the
normal translated caller body must still remain blocked after the checkpoint.

The scheduled item captures the first transfer's token when admitted. It must
not look up whichever token is current at delivery time, and it must service
at most that one token. This prevents reset/reissue confusion and prevents the
callback's newly issued second DMA from completing recursively in the same
checkpoint.

### CPU IRQ dispatch

Use three separately testable operations, even if a deterministic scheduler
invokes the first two adjacently:

```text
PE_GPU_ServiceDMA2Completion(token)  // VRAM/CHCR/DICR only
PE_IRQ_AssertSources(0x0008)         // on a derived bit31 rising edge
PE_IRQ_ServicePending()              // later CPU source scan/callbacks
```

This preserves the observable progression DICR asserted/I_STAT clear, then
I_STAT source 3 pending, then I_STAT acknowledged while DICR is still pending,
then DICR acknowledged by retail. CPU service reads live
I_STAT/I_MASK/registered/table state. Completion never directly invokes
`func_80074520`; registration never services pending status. The IRQ edge is
latched once; acknowledging I_STAT before DICR must not invent a second
source-3 edge while DICR bit 31 remains high.

The same edge bridge applies to a DICR write that creates a false-to-true
derived transition. It is not a completion-only shortcut.

No unique retail service PC is recoverable because completion timing belongs
to hardware. The `func_8001220C` point selected above is a deterministic
native scheduling decision constrained by the recovered ordering, not a claim
that retail completed DMA at one exact instruction.

Run-control checks must not suppress retail cleanup that has already become
due. B53H's explicit `retail_returned` result and monotonic stop epoch remain
the model for distinguishing a newly reached boundary from an older sticky
stop.

In particular, guest identity `0x80076EE4` must bind through
`PE_func_80076EE4_Pump(&retail_returned)` (or an equivalent typed binding that
preserves the signal). Treating the untranslated idle cut as the plain
wrapper's integer return can fabricate a legal-looking zero and bypass the
boundary contract.

If `retail_returned == 0`, the nested retail call has not returned. That
outcome must propagate immediately through the DMA dispatcher, CPU dispatcher,
and host checkpoint. `func_80074520` must not execute its post-`jalr` snapshot
advance/resample/diagnostic; `func_80073F00` must not continue its source scan;
and `func_8001220C` must not execute ordinary caller code. The I_STAT and DICR
acknowledgements already performed before the callback remain committed. The
future translations therefore need an internal non-return/boundary signal at
each nested layer, not merely a corrected leaf binding. Host unwinding must
not RAII-clear `D_800945E6`; it remains `1` because retail never reached
`0x800740AC`.

## Reset and stale-event contract

B53B already preserves a private monotonically increasing DMA event serial
across GPU reset. Reset cancels the descriptor; old tokens fail against
inactive state or a newer token. Preserve that contract.

Future coherent reset behavior is:

| Reset moment | Required result |
| --- | --- |
| DMA active | Cancel descriptor, idle CHCR, clear DICR, reject old token. |
| DMA completed, IRQ not dispatched | Clear DICR and I_STAT; no later delivery. |
| Source 3 pending | Clear I_STAT bit 3; no callback. |
| DICR channel flag pending | Clear the flag and derived bit31. |
| DMA callback registered | Clear DMA table in retail `func_800744D4` order, then allow normal reinstallation. |
| CPU callback registered | Coherently reset CPU table/mask/controller, then register source 0 and 3 in retail order. |
| Callback absent | A pending DMA flag is still acknowledged by `func_80074520`; no call occurs. |

`func_800740D0`'s same-handler fast return makes partial reset unsafe: clearing
native I_MASK while preserving stale guest handler slots will not repair the
mask during identical re-registration. One reset owner must coherently reset
I_STAT/I_MASK, CPU table, registered mask, guard/dispatch/watchdog state, and
the event generation before reinstalling source 0 then source 3.

The DMA setter `func_800746A0` has the analogous hazard: preserving stale slot
2 while clearing DICR controls lets its same-handler fast return skip DICR
repair. A coherent host hardware reset must clear pending DICR flags (through
reset authority or explicit W1C), then reproduce the retail DMA-table clear
and zero-control write before later slot-2 registration. `func_800744D4`'s
literal zero write alone does not acknowledge stale flags; leaving one could
create a new IRQ3 edge when master is later enabled.

Add a monotonic IRQ/reset generation. Any host work item that captures a
pending assertion or CPU-service opportunity must compare its generation and
no-op after reset. Prefer live-state service with no queued callback object;
never retain a native callback pointer. Mask changes do not advance the
generation and must not discard same-generation pending status.

## First idle-pump dependency after delivery

The untranslated range is `0x80076F10..0x8007712C`. With genuine completion
and callback entry, its dependencies occur in this order:

1. `func_80073E10(0)` — already translated I_MASK exchange;
2. queue producer/consumer and ring guest state — already represented;
3. `func_80073CF4(2,0)` in the last-entry/no-DrawSync case — already
   translated wrapper and DMA setter;
4. GPUSTAT ready read — existing GPU provider;
5. queued guest identity `func_80076664` — already translated worker;
6. second DMA issue — existing GPU/DMA2 platform authority;
7. consumer write at `0x80077054` — belongs to the idle pump itself;
8. saved I_MASK restore — existing helper.

No new nested retail helper blocks the canonical path. The first remaining
dependency is the idle-pump body itself. IRQ integration is therefore
sufficient to reach the honest named `func_80076EE4_idle_pump` boundary; it is
not permission to translate that boundary in B53I.

## Architecture decision and implementation split

One bounded rung is rejected because the current port lacks both the authentic
CPU-source installation state and any I_STAT authority. Combining those with
hardware completion, CPU dispatch, DMA dispatch, and callback delivery would
make intermediate-state regressions difficult to isolate.

### B53I-B1 — controller authority and source installation

Add exactly one 16-bit I_STAT authority beside the existing I_MASK, including
W0C, masked-pending retention, reset generation, and no automatic callback.
Restore the retail reset/source installation path sufficiently to produce:

```text
D_800945E8[0] = 0x8007440C
D_800945E8[3] = 0x80074520
D_80094614    = 0x0009
I_MASK        = 0x0009
```

Translate/bound `func_80073CC4` and the required `func_800740D0` setter
semantics over guest-backed tables. Do not complete DMA and do not dispatch a
CPU callback in B1.

Because B1 restores the exact source-0 installation as well as source 3, it
must also preserve source 0's non-null side calls
`func_80073C74(0)` followed by `func_80073C84(3,0)`, or bind explicit tested
host equivalents. Both calls occur while I_MASK is still temporarily zero and
before the final updated-mask restore at `0x800741F0`. Merely writing
source-0 table/mask bits, or moving these calls after restore, is not an exact
ResetCallback installation.

### B53I-B2 — completion edge and two-layer dispatch

Extend guest-visible DICR readback/edge behavior on every DICR mutation,
gate normal completion-flag creation by the channel/master enables, connect an
explicit valid-token DMA completion edge to source-3 I_STAT assertion, add an
independently callable CPU pending-service path for source 3, and translate
complete
`func_80074520`. Resolve the two guest callback identities through separate
tables. Stop honestly at `func_80076EE4_idle_pump`; do not translate it as part
of B2.

The canonical service occurs outside a represented critical section. B2 must
not claim full CPU SR/exception gating merely by implementing I_STAT/I_MASK;
that broader interrupt architecture remains out of scope unless a recovered
call path requires it.

Only a later rung should translate the idle pump and prove second-worker issue
plus the exact consumer advance.

## Future acceptance matrix

The implementation rungs must cover at least:

1. active DMA withheld;
2. explicit hardware completion;
3. VRAM visible before interrupt delivery;
4. CHCR clears;
5. DICR channel-2 pending;
6. source-3 pending;
7. source 3 masked;
8. masked pending retained;
9. source 3 unmasked;
10. source-3 dispatch;
11. `func_80074520` entry;
12. no pending DMA flags;
13. DMA2 flag with callback;
14. DMA2 flag without callback;
15. exact W1C acknowledgement;
16. multiple DMA flags ordering;
17. callback executes once;
18. callback sees DMA idle;
19. `func_80076EE4` idle re-entry;
20. queue entry consumed;
21. consumer advances exactly once;
22. second worker invoked;
23. second DMA starts;
24. producer remains correct;
25. no premature queue clear;
26. I_MASK save/restore;
27. I_STAT acknowledgement;
28. unrelated IRQ bits preserved;
29. unrelated DICR flags preserved;
30. reset before completion;
31. reset after hardware completion before IRQ dispatch;
32. reset with source 3 pending;
33. stale completion rejected;
34. stale IRQ rejected;
35. frame-limit/stop epoch cannot suppress cleanup;
36. repeated deterministic completion chain;
37. sanitizer guest-pointer safety;
38. no native pointers in callback state;
39. no automatic GPU ready;
40. no wall-clock dependence.

Additional invariants exposed by this audit should be asserted alongside that
matrix: physical bit 31 is derived rather than stored; I_STAT is edge-latched
independently of I_MASK; CPU I_STAT ack precedes `func_80074520`; DICR W1C
precedes the DMA callback; zero callback slots are still acknowledged; and
source/DMA tables never acquire duplicate native mirrors. DICR control-write
tests must cover a master `0 -> 1` edge with a retained flag, bit-15 force,
deassert/acknowledgement without a new edge, and channel-enable changes that do
not independently satisfy the derived-bit transition. A nested
`func_80076EE4_idle_pump` non-return must preserve the already-issued two
acknowledgements while suppressing every post-callback DMA/CPU scan and normal
host-loop continuation; it must also preserve dispatch-active
`D_800945E6 = 1`. A normally returning CPU-dispatch case must reach the retail
terminal clear and leave it zero. Normal DMA completion with its channel enable clear or
master enable clear must produce no completion flag/source edge; the second
canonical DMA after slot-2 deregistration is the required integration case.
Dirty/unbound nonzero identities in either callback table must acknowledge in
retail order and then stop at a typed indirect-call boundary; zero is the only
case that may be drained without a call or boundary.

## B53I-A preservation evidence

- normal native suite: 525/525;
- fresh ASan/UBSan suite: 525/525, zero diagnostics;
- executable/oracle programs: 42/42 PASS, including B53H and B53G through
  corrected B50;
- all 33 retained standalone oracles: PASS;
- frozen B53B platform group: 15/15 in normal and sanitizer suites;
- retained B49 harness: PASS normal and sanitizer;
- canonical strict normal/sanitizer: `func_8006AD40_prefix_cut` from
  `func_8006AD40`, exit 1;
- bootstrap strict normal/sanitizer: `func_8007F72C` from
  `func_800698D4`, exit 1;
- framebuffer SHA-256:
  `fb28dc21dd1e41eb72b8fe22dd3295bb8ed0c040aa88f7885a68dedc2629dfdb`;
- real-data FNV-1a64 normal/sanitizer: `7D860391E1ED6C97`;
- matching executable SHA-1:
  `452fb033f2eaa4b18aa20a5bca60b8125af3a37b`;
- no canonical DMA completion was serviced;
- no production source or test behavior changed.

No B53I oracle was necessary: the complete 96-word dispatcher transcription,
raw-body hashes, independent disassembly, and existing hardware/oracle gates
provide the evidence without adding an executable model that could obscure
the two deferred implementation phases.

## Exact next task

**Phase 6E-B53I-B1: add the single 16-bit I_STAT authority and restore exact
source-0/source-3 CPU IRQ registration at ResetCallback, including masked
pending and reset-generation semantics, without completing DMA or dispatching
callbacks.**
