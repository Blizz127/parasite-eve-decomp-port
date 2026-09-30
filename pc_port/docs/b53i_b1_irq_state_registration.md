# Phase 6E-B53I-B1 — CPU IRQ state and ResetCallback registration

## Result and boundary

B53I-B1 adds the single native 16-bit I_STAT authority beside the existing
I_MASK authority and restores the exact CPU source-0/source-3 registration
slice of retail ResetCallback. Completed initialization now leaves:

```text
I_STAT                         0x0000
I_MASK                         0x0009
D_80094614 registered mask     0x0009
D_800945E8 source-0 slot       0x8007440C
D_800945F4 source-3 slot       0x80074520
```

This rung has no interrupt-delivery operation. It does not complete DMA,
derive DICR bit 31, assert source 3 from DICR, scan pending CPU sources,
execute `func_80074520`, dispatch a DMA callback, enter
`func_80076EE4_idle_pump`, consume the GPU ring, or issue the second DMA.

Starting B53I-A commit:

```text
31da8f843241910a70d49cee6e738b4c19bb76fd
```

Its parent is `a470422d1accdae39a1d8cb4b501df599df9003c`.

## Retail provenance

All recovery uses the matching retail executable:

```text
SHA-1   452fb033f2eaa4b18aa20a5bca60b8125af3a37b
SHA-256 5d94938ee752e81ef375bd4493c9883850c25a86895f9cb0732cf3622b44351b
```

The standalone `pc_port/tools/b53i_b1_oracle.py` embeds and verifies all 210
literal words in the seven B1-relevant windows:

| Function | Range (exclusive end) | File offset | Size / words | SHA-256 |
| --- | --- | --- | --- | --- |
| `func_80073C94` | `0x80073C94..0x80073CC4` | `0x64494` | `0x30` / 12 | `497aefd54b6255dc9291f46bded9c3310f737aa4fbb132ded3aae4a4204d9f93` |
| `func_80073CC4` | `0x80073CC4..0x80073CF4` | `0x644C4` | `0x30` / 12 | `ff71c9ce2b4ad8e8d5afb17da0e186ee953dabddd306f06499c40fc17b3c879e` |
| `func_80073E28` | `0x80073E28..0x80073F00` | `0x64628` | `0xD8` / 54 | `2fef9e18b8a89258afceb2e3131dd34738ed0585120282beb8ee269a842b6c61` |
| `func_800740D0` | `0x800740D0..0x80074218` | `0x648D0` | `0x148` / 82 | `a6991559f3a0292a07423fdaf10d16d6da15eba33543f02dd1e1041aa5715332` |
| `func_80074330` | `0x80074330..0x80074354` | `0x64B30` | `0x24` / 9 | `7205982a9f3764f91f1bab001959ef54d1d12176ea93ee300af2b342db7a47df` |
| `func_800743B4` | `0x800743B4..0x8007440C` | `0x64BB4` | `0x58` / 22 | `a8d76ebfddf20244c3904501babf8e4409502f625eefcd21ea740d9fac6c76f2` |
| `func_800744D4` | `0x800744D4..0x80074520` | `0x64CD4` | `0x4C` / 19 | `e293c6aa8850c852f562fee7c52c855d8b020fad9669159721d8b8e4b152a1be` |

Executable data independently confirms:

```text
D_8009566C = 0x8009564C          SDK jump-table base
D_80095654 = 0x800740D0          source-setter field (+8)
D_80095670 = 0x1F801070          I_STAT
D_80095674 = 0x1F801074          I_MASK
```

## Instruction-addressed ResetCallback order

The complete B1-relevant ordering is:

| Retail PC | Literal operation | B1 effect |
| --- | --- | --- |
| `0x80073E3C` | `lhu D_800945E4` | one-time guard; a set guard returns before controller/table writes |
| `0x80073E60` | `sh zero, I_MASK` | temporary mask becomes zero |
| `0x80073E64` | `lhu I_MASK` | reads back zero |
| `0x80073E6C` | `sh v0, I_STAT` | W0C zero clears all pending status |
| `0x80073E7C` | `sw 0x33333333, DPCR` | preserves the accepted B53E reset write |
| `0x80073E80` | `jal func_80074330` | starts the guest IRQ-block clear |
| `0x80073E84` | `li a1,0x41A` (delay slot) | exactly `0x41A` words are cleared |
| `0x80073EC0` | `sh 1,D_800945E4` (delay slot) | publishes the guard before source-0 initialization |
| `0x800743D8` | `sw zero,D_800956AC` | clears VBlank dispatch count first |
| `0x800743DC..0x800743E0` | `jal func_800744A4`; delay `a1=8` | then clears callback slots 0 through 7 ascending |
| `0x800743E4..0x800743F0` | materialize `0x8007440C`; `jal func_80073CC4`; delay `a0=0` | installs source 0 first |
| `0x80074130..0x80074134` | `lhu I_MASK`; `sh zero,I_MASK` | setter saves mask and temporarily disables all sources |
| `0x80074148` | `sw handler,D_800945E8[source]` | stores the 32-bit guest identity |
| `0x8007414C..0x8007415C` | `lhu`/OR/`sh D_80094614` | registered mask becomes `0x0001` |
| `0x8007418C` | `jal func_80073C74` | BIOS B(5Bh), argument `handler == 0` |
| `0x80074190` | `move a0,s0` (delay slot) | source-0 install passes zero |
| `0x80074194` | `li a0,3` | selects VBlank counter for the next call |
| `0x80074198` | `jal func_80073C84` | BIOS C(0Ah) / ChangeClearRCnt |
| `0x8007419C` | `move a1,s0` (delay slot) | source-0 install passes `(3,0)` |
| `0x800741F0` | `sh s3,I_MASK` | only now restores updated mask `0x0001` |
| `0x800744EC` | `li a0,3` | selects CPU source 3 |
| `0x800744F8..0x800744FC` | materialize `a1=0x80074520` | preserves a guest identity, not a host pointer |
| `0x80074500` | `jal func_80073CC4` | installs source 3 second |
| `0x80074130..0x8007415C` | same generic setter path | mask/register bits become `0x0009` |
| `0x80074180` | branch for nonzero source | source 3 skips all source-specific BIOS calls |
| `0x800741F0` | `sh s3,I_MASK` | final I_MASK becomes `0x0009` |

The bulk-clear fact corrects one sentence in the B53I-A audit. The nine-word
`func_80074330` writes zero words from `0x800945E4` through `0x80095648`:

```text
start          0x800945E4
word count     0x41A = 1050
byte count     0x1068
exclusive end  0x8009564C
```

The range includes the guard, `D_800945E6` dispatch-active flag, all eleven
32-bit entries at `D_800945E8`, and the 16-bit registered mask
`D_80094614`; it ends exactly before the SDK jump table. Retail therefore
actively eliminates the same-handler hazard before reinstalling callbacks.
Retail subsequently writes exception/SDK setup state inside the cleared
range, including `D_80094620 = 0x800955FC`; B1 does not claim that every byte
of its post-return block is the complete retail-final image. Native tests
assert the B1-owned fields, representative stale-word clearing, and both
exclusive boundary canaries rather than freezing the omitted setup slice as
zeros.

## One I_STAT/I_MASK authority

`pc_port/platform/pe_irq.c` is the only native owner of both represented
controller registers:

```c
static uint16_t g_i_status;  /* 0x1F801070 */
static uint16_t g_i_mask;    /* 0x1F801074 */
```

The narrow B1 surface is:

```c
void            PE_IRQ_Reset(void);
uint16_t        PE_IRQ_ReadStatus(void);
void            PE_IRQ_WriteStatus(uint16_t retain_bits);
void            PE_IRQ_AssertSources(uint16_t sources);
int             PE_IRQ_AssertSourcesForGeneration(
                    uint16_t sources, PeIrqGeneration generation);
PeIrqGeneration PE_IRQ_Generation(void);
uint16_t        PE_IRQ_GetMask(void);
uint16_t        PE_IRQ_ExchangeMask(uint16_t new_mask);
```

There is no second I_STAT in GPU, SPU, callback, or libetc state. Basic
register operations do not inspect the guest callback table and cannot run a
callback.

### Exact I_STAT W0C rule

The sole write formula is:

```c
I_STAT = (uint16_t)(I_STAT & written_value);
```

A written zero clears; a written one retains. This is not DICR's W1C rule.
Focused native tests and the independent oracle cover:

| Current | Write | Result |
| --- | --- | --- |
| `0x0008` | `0xFFF7` | `0x0000` |
| `0x0009` | `0xFFFE` | `0x0008` |
| `0xFFFF` | `0xFFFF` | `0xFFFF` |
| `0xFFFF` | `0x0000` | `0x0000` |
| `0xA55A` | `0x0FF0` | `0x0550` |

Repeated acknowledgement is idempotent. A later hardware assertion can
relatch a cleared bit.

### Assertion and masked pending

Hardware assertion is exactly:

```c
I_STAT = (uint16_t)(I_STAT | asserted_sources);
```

It is independent of I_MASK. With mask zero, asserting `0x0008` leaves
I_STAT `0x0008`; subsequently restoring mask bit 3 retains that status. No
callback executes at assertion or unmask time because B1 deliberately has no
pending-service API.

## Reset phases and generation

Raw platform reset and completed retail ResetCallback are distinct:

| Phase | I_STAT | I_MASK | Guest CPU table | `D_80094614` |
| --- | --- | --- | --- | --- |
| `PE_IRQ_Reset()` plus coherent SDK reset | `0` | `0` | cleared | `0` |
| completed `func_80073C94()` | `0` | `0x0009` | slots 0/3 installed | `0x0009` |

`PE_IRQ_Reset()` advances one nonzero 64-bit generation and clears both
registers. Assertions and mask/status writes do not change it. Retail
ResetCallback installation does not advance it: only the host/platform reset
is the stale-event boundary. `PE_IRQ_AssertSourcesForGeneration` admits the
OR only when the captured generation still matches; a pre-reset token is an
inert stale event. The counter would wrap only after `2^64-1` host resets;
zero is skipped and this is documented as practically unreachable.

`PE_Sdk_ResetState()` also coherently clears the guest-backed guard,
dispatch-active flag, eleven CPU slots, registered mask, and watchdog before
any reinstall. This targeted host reset avoids erasing unrelated guest
program state; the retail guard-passing ResetCallback still performs its
authentic full `0x1068`-byte clear. A regression installs source 3, dirties
status/mask, resets, and reinstalls the identical handler to prove stale slot
contents cannot trigger the early same-handler return.

## Guest callback-table authority

The sole CPU callback table is retail guest RAM:

```text
base                 D_800945E8
entries              11 x uint32_t guest identities
source-0 slot         D_800945E8 = 0x8007440C
source-3 slot         D_800945F4 = 0x80074520
registered-mask       D_80094614, 16-bit
```

No native callback-table mirror was added. `pe_addr_t`/`uint32_t` preserves
the full high guest address. Registration never casts a guest address to a
host function pointer. B2 must later resolve only known bound identities at a
typed indirect boundary.

The recovered ABIs are:

```c
pe_addr_t func_80073CC4(uint32_t source, pe_addr_t handler);
pe_addr_t func_800740D0(uint32_t source, pe_addr_t handler);
```

Both return the previous full 32-bit guest identity. `func_800740D0` returns
immediately when the old and new identities match, before checking the guard
or repairing masks. With a different identity and a zero guard it returns the
old identity without mutation. B1 implements the exact, bounded source-0 and
source-3 paths. Sources 4, 5, and 6 have separate root-counter BIOS calls;
all unsupported sources stop at the named `func_800740D0_source_cut` rather
than pretending to register.

`func_80073CC4` dispatches through SDK jump-table field `+8` in retail. The
canonical table contains `0x800740D0`; B1 binds that proven target directly,
matching the established bounded-wrapper treatment of `func_80073CF4`.
Dirty/pre-install jump-table dispatch remains outside this rung.

## Source-0 BIOS side calls

For source 0 only, retail calls:

```text
func_80073C74(handler == 0)       BIOS B(5Bh), ChangeClearPAD
func_80073C84(3, handler == 0)    BIOS C(0Ah), ChangeClearRCnt
```

Installation of `0x8007440C` therefore passes `0`, followed by `(3,0)`.
Removal passes `1`, followed by `(3,1)`. B(5Bh) has a void ABI; the C(0Ah)
previous-value result is ignored. B1 models only these two BIOS control
values and a value-only diagnostic order trace. At each call the trace also
snapshots the live source-0 guest word and 16-bit registered mask so tests
prove those retail mutations precede B(5Bh). This diagnostic state is not a
callback-table authority and contains no callable native function pointer.

The exact invariant is:

```text
slot/reg-mask mutation
< B(5Bh), with I_MASK=0
< C(0Ah), with I_MASK=0
< final I_MASK restoration
```

Source 3 takes the nonzero-source branch at `0x80074180`, misses the special
source-4/source-5/source-6 branches, and reaches the mask restore without a
BIOS side call.

## Bounded ResetCallback translation

B1 translates the controller and CPU-registration effects required by this
rung. It preserves the existing DPCR reset and guest VBlank callback-table
reset. It deliberately does not import these adjacent retail operations:

- setjmp/HookEntryInt exception-buffer setup and the `D_80094620` stack word;
- the conditional `func_80073F00` IRQ-dispatcher re-entry when that setjmp
  context resumes with a nonzero result;
- SDK jump-table result stores: the loaded fields are zero, while B1's
  bounded native wrappers directly bind the recovered canonical targets;
- timer-1/root-counter mode programming in `func_800743B4`;
- DMA callback-table clearing in `func_800744D4`;
- the DICR zero-control write in `func_800744D4`'s source-3 call delay slot;
- the final `func_8007436C` BIOS A(72h) call, `func_80072724`
  `ExitCriticalSection`, and retail queue-block-pointer return sequence.

Those are not needed to establish CPU I_STAT/source registration. Performing
the DMA callback-table clear or DICR zero-control write against the accepted
live canonical DMA state would cross the B1 no-delivery/no-hardware-progress
boundary. The literal oracle retains the full initializer windows so B2
cannot lose their authentic relative order.

## No-dispatch and no-hardware-progress proof

There is no `PE_IRQ_ServicePending` symbol or equivalent production service
point. `PE_IRQ_AssertSources`, status writes, mask writes, registration, and
ResetCallback contain no call to `func_80074520`, DMA callback dispatch, or
the GPU pump.

Focused tests snapshot live B53H-style state across completed
ResetCallback. Registration leaves all of these unchanged:

```text
DMA2 MADR / BCR / CHCR
DMA2 active and completion-pending state
DICR and GPUSTAT
the relevant VRAM region
ring producer / consumer / queued entry
VBlank
```

The test separately proves callback invocation count remains zero. Hardware
reset tests are isolated from this registration-only snapshot because GPU
reset intentionally owns different state.

## Oracle and preservation gates

`b53i_b1_oracle.py` verifies:

- the executable SHA-1;
- all 210 literal words and seven body hashes;
- wrapper table pointers and I_STAT/I_MASK addresses;
- the `0x41A`-word clear and exact exclusive end;
- source 0 before source 3;
- source-0 handler, side-call arguments, delay slots, and mask-zero order;
- exact `func_80073CC4(3, 0x80074520)` call and delay slot;
- final I_STAT/I_MASK/registered/table state;
- W0C, masked-pending, coherent-reset, and same-handler behavior;
- zero callback delivery and no hardware evolution.

Final measured preservation results:

```text
native suite, normal                     533/533
native suite, fresh ASan/UBSan            533/533, zero diagnostics
focused B53I-B1, normal/sanitizer         8/8 each
frozen B53B, normal/sanitizer             15/15 each
retained B53H, normal/sanitizer           8/8 each
oracle/executable matrix                  43/43
retained oracle subset                    33/33
B53I-B1 oracle                            7 windows / 210 words
B49 host-loop, normal/sanitizer           PASS / PASS
canonical strict, normal/sanitizer        exit 1 / exit 1
bootstrap strict, normal/sanitizer        exit 1 / exit 1
```

The canonical continuing frontier remains the named translated-prefix
boundary `func_8006AD40_prefix_cut` from `func_8006AD40`, retail PC
`0x8006AE50`. In the bounded non-strict continuation it is a named
`BOOTSTRAP_RET`, requests stop reason `unresolved-boundary`, and advances the
fresh stop epoch from 0 to 1; it is not a successful boot. Bootstrap strict
remains `func_8007F72C` from `func_800698D4`. Preservation hashes are:

```text
framebuffer SHA-256  fb28dc21dd1e41eb72b8fe22dd3295bb8ed0c040aa88f7885a68dedc2629dfdb
real-data FNV-1a64   7D860391E1ED6C97
matching EXE SHA-1   452fb033f2eaa4b18aa20a5bca60b8125af3a37b
matching EXE SHA-256 5d94938ee752e81ef375bd4493c9883850c25a86895f9cb0732cf3622b44351b
```

Four independent reviews covered I_STAT/W0C, literal ResetCallback order,
guest callback/reset authority, and asynchronous-scope honesty. Their
findings were repaired before these gates; no HIGH or MEDIUM issue remains.
The only retained LOW caveat is the practically unreachable 64-bit
generation reuse after `2^64-1` host resets described above.

## Exact B53I-B2 boundary

B53I-B2 must implement the separately serviceable delivery chain, without
folding hardware completion into callback execution:

1. revise DMA completion-flag creation to the recovered physical
   enable/master gating and expose derived DICR bit 31;
2. make a DICR IRQ3 false-to-true edge assert I_STAT source 3 for the current
   IRQ generation;
3. add deterministic CPU pending service with executable source order and
   W0C acknowledgement-before-callback behavior;
4. translate `func_80074520` with its DICR W1C-before-callback scan/resample
   contract;
5. bind DMA callback identity `0x80076EE4` through the retail-returned-aware
   pump surface;
6. preserve already-committed acknowledgements and dispatch-active state
   across a nested untranslated boundary;
7. keep hardware completion, source assertion, CPU service, DMA dispatch,
   and queue consumption observably distinct.

B53I-B1 does none of those steps.
