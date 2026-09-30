# Phase 6E-B53D — `func_80073E10` I_MASK exchange + prefix advance

## Result

B53D adds the minimum faithful 16-bit I_MASK platform authority and
translates `func_80073E10` completely. The B53C dispatcher prefix now
consumes the real previous-mask return and continues through the retail
direct-issue decision and GPUSTAT bit-26 readiness poll, stopping at the
first genuine unresolved provider on each path:

- canonical non-full/direct path: the guest worker call at `0x80076D38`
  (canonical identity `func_80076664`) from `func_80076C34`;
- forced enqueue states (nonempty ring, DMA2 busy, or DrawSync callback
  present): `func_80073CF4(2, 0x80076EE4)` at `0x80076D60`;
- full ring (unchanged): `func_80077404` at `0x80076C68`.

No callback registration, queue pumping, worker issue, IRQ dispatch, or
I_STAT behavior is implemented or faked. Fresh canonical Disc 1 strict
execution in both normal and ASan/UBSan builds reports:

```text
FATAL: strict-stubs — first unresolved BOOTSTRAP_RET provider: func_80076664
       called from: func_80076C34
```

Bootstrap strict remains `func_8007F72C` from `func_800698D4`.

## Executable proof

The source executable is the SHA-exact retail image:

```text
SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
```

`func_80073E10` (independently recovered; B53A's 0x18/I_MASK-exchange
classification is confirmed exactly):

| Fact | Value |
| --- | --- |
| Executable range | `0x80073E10..0x80073E27` |
| Exclusive end | `0x80073E28` (next word `27BDFFE8`, a new prologue) |
| File offset | `0x64610` |
| Size | `0x18` / 24 bytes |
| Instructions | 6 |
| Live split | `asm/disc1/64610.s` |

Literal body, verified word-for-word against the executable:

```text
0x80073E10  3C038009  lui  v1,0x8009
0x80073E14  8C635674  lw   v1,0x5674(v1)   ; v1 = D_80095674
0x80073E18  00000000  nop
0x80073E1C  94620000  lhu  v0,0(v1)        ; v0 = *I_MASK (zero-extended)
0x80073E20  03E00008  jr   ra
0x80073E24  A4640000  sh   a0,0(v1)        ; *I_MASK = a0 (delay slot)
```

`D_80095674` is executable initial data with value `0x1F801074` (I_MASK);
its neighbors are `D_80095670 = 0x1F801070` (I_STAT) and
`D_80095678 = 0x1F8010F0` (DPCR). No code initializes the pointer.

## True ABI

```c
uint16_t func_80073E10(uint16_t new_mask);
```

- argument: only `a0`'s low halfword is stored (`sh`);
- return: the previous 16-bit mask, zero-extended by `lhu` (never sign
  extended; upper 16 bits of `v0` are always zero);
- access width: exactly 16 bits in both directions;
- ordering: the read completes before the store, and the store sits in the
  `jr ra` delay slot, so it always executes;
- the body contains no call of any kind: no IRQ dispatch, no callback, no
  I_STAT access.

## Caller census (all nine executable `jal func_80073E10` sites)

Independently rescanned; every site and delay slot verified against the
executable.

| Caller / site | `a0` | Delay slot | Return consumer | Purpose |
| --- | --- | --- | --- | --- |
| `func_80076C34` `0x80076CA0` | 0 (delay `addu a0,zero,zero`) | as left | `v0` stored to `D_8009587C` at `0x80076CB4` | mask off around submit |
| `func_80076C34` `0x80076D48` | `D_8009587C` (reload `0x80076D44`) | `nop` | discarded; `v0=0` return follows | restore after direct worker |
| `func_80076C34` `0x80076E9C` | `D_8009587C` | publishes producer `sw v0,D_80095874` | discarded | restore after enqueue publication |
| `func_80076EE4` `0x80076F10` | 0 | as left | stored to `D_80095880` | mask off around pump |
| `func_80076EE4` `0x8007709C` | `D_80095880` (`0x80077098`) | `nop` | discarded | pump restore |
| `func_80077144` `0x80077154` | 0 | as left | stored to `D_80095884` | mask off around queue/GPU reset |
| `func_80077144` `0x80077268` | `D_80095884` (`0x80077264`) | `nop` | discarded | reset restore |
| `func_80077404` `0x800774A8` | 0 | as left | stored to `D_80095884` | mask off around timeout recovery |
| `func_80077404` `0x80077524` | `D_80095884` (`0x80077520`) | `nop` | discarded; returns `-1` | timeout restore |

All nine uses are mask-off / mask-restore pairs; B53D's only in-scope call
is the dispatcher's `0x80076CA0` exchange. The B53C-proved return use: the
previous mask is published as a 32-bit word at `D_8009587C` before the
initialization-byte branch, and reloaded from there for both restores.

## I_MASK reader/writer census (0x1F801074)

Every libetc/libgpu access is a 16-bit `lhu`/`sh` through `D_80095674`.
There is exactly one raw-address access in the whole executable.

| Function / site | Instruction | Width | R/W | Meaning |
| --- | --- | --- | --- | --- |
| `func_80073DF8` `0x80073E04` | `lhu v0,(ptr)` | 16 | R | plain mask reader, returns I_MASK |
| `func_80073E10` `0x80073E1C/24` | `lhu` + `sh` | 16 | R+W | the B53D exchange (translated) |
| `func_80073E28` `0x80073E60/64` | `sh zero` + `lhu` | 16 | W+R | ResetCallback one-time reset: I_MASK=0, readback feeds I_STAT clear |
| `func_80073F28`-region dispatcher `0x80073F70/78`, `0x80074004/0C`, `0x8007402C/34` | `lhu` | 16 | R | kernel IRQ dispatch: `I_STAT & I_MASK & handler-mask` scans |
| `func_800740D0` `0x80074130/34`, `0x800741F0` | `lhu`, `sh zero`, `sh s3` | 16 | R+W | IRQ callback setter: read, mask-off, commit updated mask |
| handler entry `0x80074248/54` | `lhu` | 16 | R | saves I_MASK into the handler frame `[s0+0x32]` |
| handler exit `0x800742E8/F8` | `sh` | 16 | W | restores I_MASK from the frame |
| raw `0x8007E370` | `lw v0,0x1074(v1)` | 32 | R | resident kernel-stub region (B0/vector data in the executable image); tests bit `0x0080`; outside the retail game/libetc call graph and outside B53D scope |

Reset initialization of I_MASK is solely the `func_80073E28` write inside
ResetCallback's one-time guarded path. IRQ callback registration paths
(`func_800740D0`) and the kernel dispatcher are deliberately NOT
translated. The minimum state authority needed now is one 16-bit mask.

## Existing-authority audit and single authority

The pre-B53D audit found no native I_MASK state: `pe_gpu.c` owns only
GPU/DMA2/DPCR/DICR/VBlank; `pe_libetc.c` documented the retail I_MASK
write as a host no-op; `pe_spu_dma.c` explicitly assigns I_STAT/I_MASK to
the kernel interrupt domain; `pe_callback.c` owns only guest VBlank slots.

B53D adds `platform/pe_irq.[ch]` as the single authority:

```c
void     PE_IRQ_Reset(void);                  /* host lifecycle: I_MASK = 0 */
uint16_t PE_IRQ_GetMask(void);
uint16_t PE_IRQ_ExchangeMask(uint16_t new_mask);
```

Semantics: 16-bit state; exchange returns the previous value and replaces
the mask exactly; repeated exchanges are deterministic; no implicit IRQ
dispatch, no automatic I_STAT mutation, no callback invocation, no GPU/DMA
interaction.

## Reset ownership

Exactly one host lifecycle owner: `PE_Sdk_ResetState()` calls
`PE_IRQ_Reset()` (I_MASK = 0), alongside the existing SPU/GPU resets.
`PE_GPU_Init`/`PE_GPU_Reset` do not touch the mask — B53B's GPU reset
has no retail evidence of owning the global interrupt mask.

The retail-faithful runtime writer is the translated ResetCallback
(`func_80073C94`): its guard-passing path now performs
`PE_IRQ_ExchangeMask(0)`, matching `func_80073E28`'s `sh zero -> I_MASK`.
The retail readback feeds an I_STAT clear; that I_STAT effect is a
documented no-op (B53D models no I_STAT). The one-time guard
(`D_800945E4`) means later ResetCallback calls do not rewrite the mask;
tests prove both the guard-passing write and the guarded no-write.
Both writers drive the same single authority, so no divergence is
possible.

## I_STAT separation

`func_80073E10` touches only I_MASK (proved by the literal words). I_STAT
(`0x1F801070`) is not modeled: no production code reads, writes, or
derives anything from it. The oracle hard-fails on any I_STAT access or
any 32-bit access to either register during exchange execution.

## Translation and prefix advance

`func_80073E10` is translated in `platform/pe_libetc.c` as the direct
`PE_IRQ_ExchangeMask` call — no bootstrap fallback, no host pointer state,
no signed truncation (uint16_t in, zero-extended uint16_t out, widened
explicitly by the caller's 32-bit store).

`game/boot/func_80076C34_port.c` now executes, in exact retail order:

1. `func_800773D0` timeout init (B53C);
2. guest producer/consumer full check (B53C; full -> `func_80077404`
   boundary unchanged);
3. `func_80073E10(0)` and the 32-bit previous-mask store to `D_8009587C`
   (`0x80076CB4`);
4. initialization-byte load `D_8009574D` and the unconditional marker
   store `D_80095754 = 1` (the `0x80076CC4` delay slot);
5. when initialized: reload producer/consumer, then the enqueue selectors
   (nonempty ring -> `func_80073CF4` boundary; DMA2 CHCR bit 24 via
   `PE_GPU_ReadDMA2CHCR` -> same; `D_80095758` nonzero -> same);
6. direct path: the unbounded GPUSTAT bit-26 tight poll
   (`0x80076D20..0x80076D2C`) against the inert B53B status read;
7. the `jalr s3` worker call with `a0 = argument`, `a1 = auxiliary` —
   recorded as the honest unresolved boundary (canonical identity
   `func_80076664`), followed by `PE_PORT_STOP_UNRESOLVED_BOUNDARY`.

The I_MASK restores at `0x80076D48`/`0x80076E9C` sit behind the worker
and pump dependencies and are intentionally not reached. No ring entry is
constructed or published on any path. The worker symbol is diagnostic:
the four B53A command-issue identities map to literal names, any other
guest address formats as `func_%08X`; the target identity itself is always
the exact 32-bit guest address.

## Oracle

`pc_port/tools/b53d_oracle.py` is standalone (no production C). It:

- asserts the executable SHA-1;
- embeds and verifies all 6 literal exchange words, the exclusive end, the
  pointer-table initial data, all 9 call sites with delay slots, and the
  dispatcher caller-context words;
- executes the literal body with a tiny delay-slot-aware MIPS-I
  interpreter: initial/zero exchange, `0xFFFF`, alternating masks,
  high-bit masks, a full 16-bit walk, `a0` upper-bit truncation,
  zero-extended returns, and repeated-sequence determinism;
- hard-fails on any I_STAT access, any 32-bit MMIO access, any guest-RAM
  write, or any call inside the helper;
- executes the literal dispatcher slice `0x80076CA0..0x80076D50`: the
  canonical state reaches the worker boundary with exact
  `a0 = argument` / `a1 = auxiliary`; a seeded nonzero previous mask is
  saved exactly; the zero initialization byte skips all checks; each
  enqueue selector reaches `func_80073CF4(2, 0x80076EE4)` with no ring
  publication; a controlled not-ready/not-ready/ready GPUSTAT script
  proves the poll re-reads without queue mutation.

Result: PASS.

## Native tests

Six focused B53D tests extend the suite from 490 to 496:

1. exact exchange ABI: initial zero, exchange 0/nonzero, previous-mask
   return, repeated/alternating, all 16 bits, zero boundary invocations;
2. no I_STAT/callback/GPU mutation: full 2 MiB guest-RAM canary, GPU
   state snapshot equality, no stub or boundary records;
3. reset ownership: `PE_Sdk_ResetState` clears (twice, deterministic);
   ResetCallback guard-passing zeroes, guarded call does not rewrite;
4. direct worker boundary integration: exact boundary identity/arguments,
   saved mask, marker, mask cleared; nonzero prior mask saved exactly;
5. initialization-byte zero skips every check to the direct worker;
6. enqueue selectors: a real B53B DMA2-busy state and a DrawSync callback
   each stop at `func_80073CF4(2, 0x80076EE4)` with no ring publication
   and no fabricated DMA completion.

Retained B53C/B52/B51/B50 tests were updated for the advanced frontier
(snapshot+boundary record pairs in the four-register log; the two new
persistent guest words `D_8009587C`/`D_80095754` in canary skip sets).

## Verification

- normal native suite: 496/496;
- fresh ASan/UBSan suite: 496/496;
- B53D oracle: PASS; B53C, B52, B51, corrected B50 oracles: PASS;
- all 33 retained standalone oracles: PASS;
- all 15 B53B platform tests green inside both suites; the frozen B53B
  contract is unchanged (I_MASK authority never touches GPUSTAT, DMA2,
  DICR, DPCR, VRAM, the pending GPU event, or VBlank — proven by test 2
  and the B53B authority-separation guard);
- retained B49 host-loop harness (updated frontier assertions): PASS in
  normal and sanitizer builds;
- bootstrap strict unchanged: `func_8007F72C` from `func_800698D4`;
- canonical Disc 1 continuing strict, normal and sanitizer agree:
  `func_80076664` from `func_80076C34`;
- framebuffer SHA-256 remains
  `fb28dc21dd1e41eb72b8fe22dd3295bb8ed0c040aa88f7885a68dedc2629dfdb`;
- real-data FNV-1a64 remains `7D860391E1ED6C97`;
- matching executable remains SHA-1
  `452fb033f2eaa4b18aa20a5bca60b8125af3a37b`;
- `git diff --check` clean.

## Deferred functionality and next bounded rung

B53D does not translate or adapt:

- `func_80076664` (LoadImage issue worker) — the new canonical frontier;
- `func_80073CF4` DMA callback registration (enqueue-path boundary);
- `func_80076EE4` queue pump, `func_80077404` timeout recovery;
- the I_MASK restore sites at `0x80076D48`/`0x80076E9C` (behind the
  worker/pump dependencies);
- IRQ dispatch, I_STAT semantics, nested masking, or any asynchronous
  interrupt delivery/timing.

The next bounded task is B53E: recover and translate the direct-path
worker `func_80076664` (already fully recovered in B53A: 143 words,
`0x80076664..0x800768A0`), including its bounded GPUSTAT wait, exact
GP1/GP0 A0 sequence, source-span validation before CPU-prefix reads, and
DMA2 issue against the B53B substrate. Before DMA issue can pass the B53B
start invariant, the reset owner must also wire the retail DPCR
channel-2 enable — canonical state currently has DPCR = 0.
