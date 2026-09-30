# Shim Inventory — Phase 6E-B54K-B6

Bootstrap stubs invoked in the `func_8001220C` (main) → first-clear path.
All stubs are explicitly classified. No anonymous empty stubs.

## Real translated functions (not stubs)

### Phase 6E-B21 correction

`func_80071A24` is the verified retail BIOS A(28h) trampoline, not
SysEnqIntRP: raw words are `240A00A0 01400008 24090028` at executable
`0x80071A24..0x80071A2F` (file offset `0x62224`).  A(28h) is
`bzero(dst,len)`; the checked host adaptation uses `pe_addr_t` and clears
exactly the requested guest byte range.  `func_80064964` calls it with
`dst=0x800A3060`, `len=0x120`, then stores `0xFF` at
`0x800A3078, 0x800A30A0, 0x800A30B0, 0x800A30B8, 0x800A30C0,
0x800A30C4, 0x800A3124, 0x800A3134` in that order.  The return value is
not consumed by this caller.  The general provider returns the incoming
destination, as documented for the BIOS memset family; an executable-wide
scan finds 11 call sites (including `0x8005D70C`, `0x80040238`,
`0x80040BB8` where `$v0` is copied to `$s2`, `0x80040D30`, `0x80042204`,
`0x80042548`, both `0x80064974/0x800649EC`, `0x80084598`, and
`0x80084880`).  C(02h) SysEnqIntRP requires vector `0xC0` and is not this
call.  Independent checks live in `tools/b21_bzero_oracle.py` and
`tools/b21_order_oracle.py`.

| Function | Size | Words | Port file |
|----------|------|-------|-----------|
| `func_8001220C` | - | 187 | `bootstrap/func_8001220C_port.c` |
| `func_800698D4` | - | - | `bootstrap/func_800698D4_port.c` |
| `func_8006E834` | - | 91 | `bootstrap/func_8006E834_port.c` |
| `func_8006E9A0` | 0x234 | 141 | `bootstrap/func_8006E9A0_port.c` (de-adapted Phase 6E-FD1; `70E54` real since Phase FTE1) |
| `func_80070E54` | 0x158 | 86 | `game/boot/func_80070E54_port.c` (Phase FTE1; matched leaf `src/func_80070E54.c`) |
| `func_80042FE8` | 0x50 | 20 | `game/boot/func_80070E54_port.c` (Phase FTE1; matched leaf `src/func_80042FE8.c`) |
| `func_8006EBE4` | 0x18 | 6 | `game/boot/func_80070E54_port.c` (Phase FTE1; matched leaf `src/func_8006EBE4.c`) |
| `func_8003E610` | 0x70 | 28 | `game/boot/func_8003E610_port.c` |
| `func_8003E680` | 0xD4 | 53 | `game/boot/func_8003E680_port.c` |
| `func_8006A5BC` | 0x90 | 36 | `game/boot/func_8006A5BC_port.c` |
| `func_8006A64C` | 0x28 | **10** | `game/boot/func_8006A64C_port.c` |
| `func_8006A674` | 0x260 | 152 | `game/boot/func_8006A674_port.c` |
| `func_8006A8D4` | 0x110 | **68** | `game/boot/func_8006A8D4_port.c` |

## IMPLEMENTED (real host equivalents)

### Native hardware authority (not a retail-function translation)

`platform/pe_gpu.[ch]` is the B53B/B53I-B2 deterministic GPU/DMA2 authority: one
1024x512x16 VRAM, GPUSTAT bit 26, GP0 A0 parser, bounded GP1 commands,
DMA2/DPCR/DICR channel-2 state, tokenized explicit completion, and a manual
VBlank counter. Stored DICR excludes bit 31; physical reads derive it as
force bit 15 or master bit 23 plus any retained flag 24..30. Every DICR
transition may sticky-latch a separate false-to-true event, and normal DMA
completion creates a channel flag only when its enable and master are set.
It contains no retail command ring, callback, worker, or queue pump. B53C
wires its inert VSync query, B53D reads raw DMA2
CHCR/GPUSTAT, and B53E drives its bounded GP1/GP0/DMA2 issue APIs. B53E adds
one inert image-load preflight query; the hardware authority still owns no
guest worker identity, RECT pointer, ring, or callback state.

`platform/host_vram.[ch]` is a VIS1 host diagnostic over that same authority,
not a second VRAM and not a retail-function translation. It can copy every
word to RGB888 or write full raw/PPM artifacts, using only
`PE_GPU_ReadVRAM`. It exposes no guest-memory write, GP0/GP1 write, DMA,
callback, scheduler, or presentation operation. The diagnostic therefore
cannot create the visible pixels it reports and cannot advance execution.

`game/boot/func_80030894_port.c` is a translated retail prefix through
exclusive address `0x800311EC` (598 of 788 words). B54K-A covers the
prologue, bank record, and L2/L3 40-sprite array. B54K-B1 adds the complete
119-word L4 packet group: compound/standalone tiles, PolyG4, standalone
sprite, and four-entry sprite array. B54K-B2 adds the complete 32-word,
five-entry L5 compound-sprite array. B54K-B3 adds the 147-word L6 group: two
PolyG4 packets, three shaded compound sprites, and three font-byte-colored
tiles. B54K-B4 adds L7's compound sprite, standalone sprite pair, PolyF3,
and three-entry compound-sprite array. B54K-B5 adds L8's ten-entry compound
sprite loop. B54K-B6 adds L9's standalone compound sprite and four-entry
compound-sprite array. All reached calls use only the
translated B54I/GPU1 helper set. The
first excluded instruction begins a distinct packet group; strict execution
names that boundary `func_80030894_L9_cut`.

`platform/pe_irq.[ch]` is the B53I-B1 single native authority for 16-bit
I_STAT (`0x1F801070`) and I_MASK (`0x1F801074`). I_STAT writes use W0C
`status &= written`; hardware assertion ORs status independently of I_MASK;
raw reset clears both registers and advances a 64-bit stale-event generation.
Basic register operations contain no pending service and invoke no callback. CPU source
identities remain solely in the guest table `D_800945E8`; ResetCallback
clears the retail `0x41A`-word IRQ block, installs source 0 = `0x8007440C`
and source 3 = `0x80074520`, and finishes with I_MASK and registered mask
`D_80094614` both `0x0009`. Source 0 performs B(5Bh)(0), then C(0Ah)(3,0),
while I_MASK is zero and before restoration. Other source-specific setter
paths stop at `func_800740D0_source_cut`.

`platform/pe_irq_delivery.[ch]` is the separate B53I-B2 retail control-flow
layer. It consumes a latched DICR rise into source-3 I_STAT without running a
callback, implements the bounded low-to-high CPU scanner from
`func_80073F00`, and implements complete `func_80074520`. Both CPU and DMA
callback tables remain live guest identities. Zero is the only absent
callback; any nonzero unbound identity acknowledges in retail order and then
stops at a typed indirect-call boundary.

| B53I-B1 function | Implemented scope |
| --- | --- |
| `func_80073C94` | canonical ResetCallback target: controller clear, exact IRQ-block clear, source 0 then source 3 CPU registration |
| `func_80073CC4` | canonical SDK jump-table `+8` wrapper target |
| `func_800740D0` | complete execution-proven source-0/source-3 setter paths; returns previous 32-bit guest identity |

### B53C–B53I-D retail dispatcher, IRQ delivery, and two DMA completions

`game/boot/func_80076C34_port.c` translates complete `func_800773D0`,
complete `func_80073E10` (via the PE_IRQ authority), and the dispatcher
direct path through numeric worker identity `0x80076664`. B53E's new
`game/boot/func_80076664_port.c` translates the 143-word LoadImage issue
worker: signed RECT clamps, full guest-source preflight, inert GPUSTAT wait,
exact GP1/GP0 A0 stream, CPU remainder, and asynchronous DMA2 issue. The
worker result is discarded, the saved I_MASK is restored, and the direct
dispatcher returns zero. Canonical first DMA remains busy, so the second
request calls the completed `func_80073CF4(2,0x80076EE4)` wrapper and its
B53G-translated `func_800746A0` setter, then constructs and publishes the
retail ring entry. B53H translates the busy-DMA fast path of the queue pump
`func_80076EE4`: with the first transfer still in flight it returns 1 and
consumes nothing, so the dispatcher completes and returns its retail
pending count. A full ring and worker timeout recovery still expose
`func_80077404`. B53I-B2 adds explicit first-DMA completion, the DICR
source-3 edge bridge, the execution-proven CPU IRQ scanner, complete
96-word `func_80074520`, and typed dispatch to `0x80076EE4`. B53I-C now
completes the pump's exact 136-word idle suffix: save/disable I_MASK,
last/no-DrawSync channel-2 callback removal, inert GPUSTAT polling, live
guest-ring decode, typed `0x80076664` worker call, consumer advance only
after return at `0x80077054`, exact mask restore, and conditional marker /
DrawSync cleanup. The canonical worker issues the second DMA at
`0x8012B8B8/0x00020010/0x01000201`; the first checkpoint never recaptures it,
so it remains active and incomplete at the C boundary. B53I-D admits one
separate later invocation of the same token-capturing checkpoint. It exposes
all 64 pixels, clears CHCR to `0x00000201`, and leaves DICR `0x00800000`:
channel-2 enable is already clear, so there is no flag, source-3 service,
DMA dispatcher, pump, worker, or queue mutation. No new scheduler or callback
authority is introduced. B54K-S now translates the execution-proven
`func_80077294` DrawSync paths and admits one already-active DMA token at
each retail wait poll through that same checkpoint owner. Timeout recovery
and no-progress states remain named cuts. The retail ring and its
producer/consumer words stay authoritative in guest RAM; the DMA callback table `D_800956C0`
stays guest-backed and separate from the VBlank table `D_8009568C`; DICR
stays solely owned by B53B `pe_gpu`. All guest identities are `pe_addr_t`,
never native function pointers.

| B53I-B2 function/surface | Implemented scope |
| --- | --- |
| `func_80073F00` equivalent | sources 0..10, exact eligibility/ack/live lookup/resample and boundary-preserving active state |
| `func_80074520` | complete 96-word DMA flag scan, W1C, live table lookup, resampling, diagnostic tail |
| `PE_IRQ_BridgeDICRRisingEdge` | generation-safe DICR edge to I_STAT source 3 only; no callback |
| `PE_Port_ServiceDmaIrqCheckpoint` | one captured token, then separate bridge and eligible-pending CPU service; no recapture loop; B53I-D proves a later disabled-IRQ completion |
| `func_80076EE4` | complete 152-word pump: B53H busy return plus B53I-C idle consumer and shared epilogue |

| Function | PS1 role | Host implementation |
|----------|----------|---------------------|
| `func_80073A44` | VSync | `HostFB_VSync()` |
| `func_80074D28` | SetDispMask | `HostFB_SetDispMask()` |
| `func_80074DC0` | DrawSync | translated `func_80077294` queue/DMA drain plus `HostFB_DrawSync()` telemetry (B54K-S) |
| `func_80074F44` | ClearImage | `HostFB_ClearImage()` |
| `func_800755F0` | PutDispEnv/Present | `HostFB_PresentDispEnv()` (guest DISPENV disp RECT → VRAM window copy; retail GP1 body untranslated) |

## HOST_ADAPTED

| Function | Adaptation |
|----------|------------|
| `scratchpad_stack_handoff` | Native stack call, no MIPS scratchpad manipulation |

## BOOTSTRAP_RET (51 stubs invoked in first-clear path)

### func_8001220C callees
- `func_800725DC`
- ~~`func_8006A9E4`~~ — TRANSLATED (Phase 6E-B16; see the func_8003E680
  callees section)
- ~~`func_8006AD40`~~ — TRANSLATED COMPLETE (Phase 6E-B54K-M; all 391
  retail words through `jr ra; nop`). Its callee `func_8006E1C0` is
  TRANSLATED (Phase 6E-B51). Phase
  6E-B52 translates both `func_8007506C` (Psy-Q LoadImage) wrappers and the
  complete read-only `func_80074E28` RECT validator. Their indirect dispatch
  enters `jtb[2] = func_80076C34` with `a0 = jtb[8] = func_80076664`,
  `a2 = 8`, and `a3 = data`; B53C converts the transient RECT to two words
  by value. B53E translates that exact worker against B53B and restores the
  saved I_MASK after it returns. The first canonical request issues DMA2
  asynchronously; the next request observes busy, registers the channel-2
  callback through the completed wrapper and setter, publishes its ring
  entry, restores I_MASK, and calls the pump, whose busy-DMA path (B53H)
  returns 1 without consuming anything. The dispatcher then returns its
  retail pending count. B53I-C/D complete the pump and two caller DMA
  opportunities; B54B-M complete every remaining archive/poll/graphics
  group, F1/display synchronization, state reset, and bit-0 clear. The
  function now returns normally with no bootstrap provider.
  - `func_80076664` — TRANSLATED direct LoadImage issue worker (B53E), with
    timeout recovery suffix still exposed at `func_80077404`.
  - `func_80073CF4` — TRANSLATED complete 12-word retail wrapper (B53F
    prefix, completed by B53G).
  - `func_800746A0` — TRANSLATED complete 43-word / `0xAC` callback-slot and
    DICR setter (B53G); guest-backed table `D_800956C0`, unchecked channel
    index, enable bit `1 << ((channel+16) & 31)`, master bit 23 set on both
    paths, DICR bit 31 read then masked off, I_MASK untouched.
  - `func_80076EE4` — TRANSLATED COMPLETE (B53H + B53I-C): 152-word /
    `0x260` queue pump. Busy DMA returns 1 with zero guest writes. Idle DMA
    processes the guest ring with exact mask/callback/readiness/worker/
    consumer ordering, typed indirect boundaries, and no automatic hardware
    completion.
  - `func_80074520` — TRANSLATED complete 96-word DMA IRQ dispatcher
    (B53I-B2); channels 0..6, DICR W1C before live guest callback lookup,
    normal-return resampling, and nested non-return propagation.
  - `func_80077404` — controlled full-ring alternate boundary.
  - `func_80076C34_enqueue_span` — controlled boundary for a copy source or
    payload width this port cannot represent (retail checks neither).
- `func_8006ECEC`
- `func_8006F044`
- `func_80069B08(int)`
- `func_8003F3C4`
- `func_801235DC`
- `func_8019234C`
- `func_801909B4_80190D7C_cut` — current structural boundary at caller PC
  `0x80190D7C`, after complete 480-frame `func_80190660`

### func_8003E610 callees (10)
- `func_80073C94`
- `func_8003E754(int, int)`
- `func_8007D054`
- `func_80077F7C`
- `func_80079004(int, int)`
- `func_80079024(int)`
- `func_800409B4`
- `func_8003E944`
- `func_8007EC14`
- `func_80080CC8(int)`

### func_8006A5BC callees (7)
- `func_80085644`
- `func_80086FF8`
- `func_80087024`
- `func_8008682C(int)`
- `func_8007ED58` (returns 1)
- `func_8007F72C` (CdReady — strict-mode gate)
- `func_8007F7A8` (returns 0)

### func_800698D4 callees (5)
- `func_8007F72C` (shared with 6A5BC)
- `func_8007F778`
- `func_80082314`
- `DsSearchFile(PEDISC01.IDF)`
- `DsSearchFile(PE.IMG)`
- `func_80080C48`

### func_8003E680 callees (2000 + 16)
- ~~`func_80070D10`~~ — TRANSLATED (Phase 6E-B1): lagged-Fibonacci RNG table
  init, `game/boot/func_80070D10_port.c`
- ~~`func_80070D6C`~~ (×2000) — TRANSLATED (Phase 6E-B2): lagged-Fibonacci
  RNG advance, `game/boot/func_80070D6C_port.c`; read cursor cycles through
  the 2 index words + 14 retail code words below the table (verbatim
  `|= 0x40` wrap), so retail-exact output needs the exe image in guest RAM
  (`pe_guest_image`); ranged wrapper `func_80070DD0` also translated
- ~~`func_8003E974`~~ — TRANSLATED (Phase 6E-B3): state zeroing
  (5 $gp-relative globals + 32-word array D_800A76F0) + ordered series of 20
  `func_8003EAC8(mask,value)` registration calls, `game/boot/func_8003E974_port.c`
- ~~`func_8003EAC8`~~ — TRANSLATED (Phase 6E-B4): GTE LZCS/LZCR-indexed
  registration leaf (idx = (a0 == 0x80000000) ? 31 : 31 - LZCR(a0); word
  store D_800A76F0[idx]=a1, below-table write at 0x800A76EC for zero/
  all-ones inputs preserved, never clamped), `game/boot/func_8003EAC8_port.c`
  + narrow `PE_GTE_LZCR` in `platform/pe_gte.c`; oracle gate
  `--lzcr-oracle-dump` ≡ `tools/lzcr_oracle.py`.  Call-site correction:
  20 DISTINCT sites (all in func_8003E974, constant args) — the earlier
  "63" counted overlapping split files
- ~~`func_80036DC8`~~ — TRANSLATED (Phase 6E-B5): timer-record init
  dispatcher + leaves func_80036DF8/36E34/36E58 (11 word stores ROM-order;
  three 12-byte records at 0x800A76A0/AC/B8 = {1,0,x}, record 0 field2
  0x1499700; consumers divide field1 by 60 — 60 Hz tick counters),
  `game/boot/func_80036DC8_port.c`
- ~~`func_80073D24`~~ — IMPLEMENTED (Phase 6E-B6): libetc VBlank
  callback slot setter (jump-table field 0x14 → func_80074478 semantics:
  prev = D_8009568C[4]; store if different; return prev), guest-backed
  8-slot table + func_8007440C-faithful dispatcher in
  `platform/pe_callback.[ch]`, wrapper in `platform/pe_libetc.c`;
  oracle gate `--callback-oracle-dump` ≡ `tools/callback_oracle.py`.
  Both func_8003E680 call sites discard the return (retail-matching)
- ~~`func_800371A4(int)`~~ — TRANSLATED (Phase 6E-B7): 3-word
  $gp-relative byte setter (`sb $a0, 0x124($gp)` → D_8009CE94 =
  guest 0x8009CE94), exe-verified words; 2 call sites (3E680 arg 0,
  527C8 arg 1), return unused; `game/boot/func_800371A4_port.c`
- ~~`func_80029388(void)`~~ — TRANSLATED (Phase 6E-B8): slot-table clear
  + default-record init, with leaves func_8002F658 (copy exe-rodata
  records D_80010928→D_800B8A20 0x70 bytes, D_80010998→D_800B0CB0 0x18
  bytes; zero D_8009D1B0/D_8009D1B4) and func_80020EFC (matched decomp
  leaf: 5 $gp-relative byte clears).  7 in-use words at D_800A5D58 +
  i*220 (same SlotRecord table as the decomp's func_8002F9CC leaf) +
  bytes D_8009D2A0/D_8009D2EC; sole call site func_8003E680 @0x8003E700
  (nop delay slot); `game/boot/func_80029388_port.c`
- ~~`func_8005BCA8(void)`~~ — TRANSLATED (Phase 6E-B9): empty jr/nop
  stub (2 words / 0x8 at VRAM 0x8005BCA8, file 0x4C4A8; retail words
  0x03E00008 / 0x00000000).  Zero guest reads/writes; sole call site
  func_8003E680 @0x8003E708 (nop delay slot); `game/boot/func_8005BCA8_port.c`
- ~~`func_80068D28(void)`~~ — TRANSLATED (Phase 6E-B10): 63-word
  double-buffered display-record data initializer at D_800BCF88 (scalar
  block +0x60..0x70, two 16-byte records +0x30+i*0x10, two 8-byte
  records +0x50+i*0x8 with 0xE1000440 GP0-shaped data word; loop byte
  values are retail load-after-store from the just-written scalars;
  write extent 0x800BCFBB..0x800BCFF9); all 63 words exe-verified
  (live split 55430.s); sole call site func_8003E680 @0x8003E710
  (nop delay slot, $v0=0 unconsumed); idempotent incl. after
  PE_RamReset; `game/boot/func_80068D28_port.c`
- ~~`func_800124F8(void)`~~ — TRANSLATED (Phase 6E-B11): 31-word
  subsystem table clear (sw 0 -> 0x8009D300, sh 0 -> 0x8009D308 with
  0x8009D304 untouched, sw 0 -> 0x8009CDFC; 72x11-word matrix at
  D_8009D310, row stride 0x2C, span 0x8009D310..0x8009DF6F; sw 0 ->
  0x8009CE00; 16-word array at D_8009DF70 = 0x8009DF70..0x8009DFAF,
  contiguous with the table end; sw 0 -> 0x8009CE04); all 31 words
  exe-verified (live split 2A0C.s); pure zero-stores, no reads, no
  SDK/GTE/hardware/GPU work; sole call site func_8003E680 @0x8003E718
  (nop delay slot, $v0=0 unconsumed); idempotent incl. after
  PE_RamReset; `game/boot/func_800124F8_port.c`
- ~~`func_8001A890(void)`~~ — TRANSLATED (Phase 6E-B12): 34-word
  subsystem scalar/array clear (sw 0 -> 0x8009CE08; stride-2 halfword
  loop 0x8009CE0C..0x8009CE13; sw 0 -> 0x8009CE14; 20-word array at
  D_8009DFB0 = 0x8009DFB0..0x8009DFFC, contiguous above 124F8's array;
  six stride-4 halfwords 0x8009CE18/1C/20/24/28/2C — interleaved upper
  halfwords untouched, ROM order A8, B8, B4, B0, AC, BC; words
  0x8009D1D8/D1FC/D2F8/D248, ROM order 468, 48C, 588, 4D8; halfwords
  0x8009D264/D1CC, ROM order 4F4, 45C); all 34 words exe-verified
  (live split A404.s); pure zero-stores, no reads, no SDK/GTE/
  hardware/GPU work; sole call site func_8003E680 @0x8003E720
  (nop delay slot, $v0=0 unconsumed); idempotent incl. after
  PE_RamReset; `game/boot/func_8001A890_port.c`
- ~~`func_80034F10(void)`~~ — TRANSLATED (Phase 6E-B13): 45-word
  subsystem table clear + flag-bit clear (sw 0 -> 0x8009D2E8; 512-word
  array at D_800A77F0 = 0x800A77F0..0x800A7FEC; D_800B6A80 = 0 — retail
  stores the same word 64x via a delay-slot loop with no pointer
  advance, reproduced as one store with identical end state; 14x160-word
  matrix at D_800BEA90, row stride 0x280, span 0x800BEA90..0x800C0D8F;
  scalars sw 0 -> 0x8009D2AC/0x8009D20C/0x8009D2F0/0x8009D254/0x8009D224
  and sh 0 -> 0x8009D2A6, ROM order 53C, 49C, 580, 536, 4E4, 4B4; sole
  guest read + RMW D_800B0CD8 &= ~0x3000, store in the jr $ra delay
  slot); all 45 words exe-verified (live split 2422C.s); no SDK/GTE/
  hardware/GPU work; sole call site func_8003E680 @0x8003E728
  (nop delay slot, $v0=&D_800B0CD8 unconsumed); idempotent incl. after
  PE_RamReset; `game/boot/func_80034F10_port.c`
- ~~`func_8006536C(void)`~~ — TRANSLATED (Phase 6E-B14): 19-word
  subsystem record-table clear + index byte clear (28x3-word table at
  D_800A3180, row stride 0xC — contiguous 84 words, span
  0x800A3180..0x800A32CF; sb 0 -> 0x44($gp) = 0x8009CDB4, the
  current-record index byte — func_800653B8 below reads lbu 0x44($gp)
  and indexes D_800A3180 + byte*12, confirming the 28x12-byte record
  structure); all 19 words exe-verified (live split 55430.s); no
  reads, no SDK/GTE/hardware/GPU work; sole call site func_8003E680
  @0x8003E730 (nop delay slot, $v0=0 unconsumed); idempotent incl.
  after PE_RamReset; `game/boot/func_8006536C_port.c`
- ~~`func_800653B8(payload, dest_id, dest_type, sender, extra)`~~ —
  TRANSLATED (PE-CH1): 18-word mailbox append at
  `D_800A3180 + count*12` then `count++` (`0x44($gp)` =
  `0x8009CDB4`); stores payload@+3, id@+2, sender@+8, type@+0,
  extra@+4 in that ROM order; no clamp at 28; all 18 words
  exe-verified (`build/disc1.candidate.exe`); sole jal
  `func_80017764` @`0x80017794` (opcode `0x1C`, extra 0 in the jal
  delay slot); `game/boot/func_800653B8_port.c`
- ~~`func_80065954(index, enabled)`~~ — TRANSLATED (PE-CH2): 18-word
  opcode `0x75` camera-slot byte update; 16-byte stride, OR 6 / AND
  `0xF9`; all words and handler jal EXE-verified;
  `game/boot/func_80065954_port.c`
- ~~`func_800659C8(index, value)`~~ — TRANSLATED (PE-CH2): 12-word
  opcode `0x7B` camera-slot halfword update; stores `value >> 8` at
  slot+8; all words and handler jal EXE-verified;
  `game/boot/func_800659C8_port.c`
- ~~`func_80066800(index)`~~ — TRANSLATED (PE-CH2): 99-word opcode
  `0x82` 52-byte view-record apply; publishes H/MATRIX/index, calls the
  real native `func_80079024`, and ORs `D_800BCF88` with `0x80`; all
  words, stores, and both callers EXE-verified;
  `game/boot/func_80066800_port.c`
- ~~`func_80017764(args)`~~ — TRANSLATED (PE-CH1): 18-word opcode
  `0x1C` send; resolves type/id/payload from a guest pointer triple,
  sender = `lhu(*(D_8009D2F0)+0x24)`, extra 0, `jal func_800653B8`,
  return 1; jump table `0x80091110`; all 18 words exe-verified;
  `game/boot/func_80017764_port.c`
- ~~`func_80017BB4_btl1_cut(args)`~~ — TRANSLATED NORMAL PATH
  (PE-CH1): opcode `0x31` for Carnegie token `0xA80002C8`;
  `D_8009D1A0 |= 0x2000`, `D_8009D280=token`, return 0.
  Full 40-word handler is exe-verified; unrelated `A9400048` special
  path is outside this cut; `game/boot/func_80017BB4_port.c`
- ~~`func_80065400(void)`~~ — TRANSLATED (PE-CH1): 117-word mailbox
  drain; extra≠0 serial arm (stop after one); extra==0 type+id
  (continue); jal real `func_80012700`; count `sb 0`;
  records kept; sole site `0x8003F4E8`; all 117 words exe-verified;
  `game/boot/func_80065400_port.c`
- ~~`func_80012700(entry, a1)`~~ — TRANSLATED (PE-CH1): 29-word
  freelist pop at `D_8009CDFC` / serial `D_8009D308`; a1==0 mailbox
  unlink; a1!=0 insert at `a1+0x24`; 7 jal sites; all 29 words
  exe-verified; `game/boot/func_80012700_port.c`
- ~~`func_800177AC(args)`~~ — TRANSLATED (PE-CH1): 7-word opcode
  `0x1F` poll; `lw 0x590($gp)` current task `D_8009D300`, copy
  `task+0x14` through `*arg0`, return 1; no ACK; jump table
  `0x8009111C`; all 7 words exe-verified;
  `game/boot/func_800177AC_port.c`
- ~~`func_80019154(args)`~~ — TRANSLATED (PE-CH1): 7-word opcode
  `0x94` mode read; `lw D_8009D28C` through `*arg0`, return 1;
  no store-back; jump table `0x800912F0`; twin of matching
  `func_80017FF0` (`0x89`); all 7 words exe-verified;
  `game/boot/func_80019154_port.c`
- ~~`func_8002F7D8(actor)`~~ — TRANSLATED (PE-CH1): 102-word
  opcode `0x6F` slot alloc; first free `D_800A5D58` `SlotRecord`
  (7×220); copy 216B `D_800109B0`; `*actor=body`; `D_8009D2EC++`
  / `body+7`; `body+8=1<<i`; if `actor+0x98&0x2000==0` then
  `body+0x18=body+0x1C`, record `func_8001A680(actor,2)`,
  `D_8009D2A0++`; wrapper `0x80018954`; all 102 words
  exe-verified; `game/boot/func_8002F7D8_port.c`
- ~~`func_8002FA10(actor, …)`~~ — TRANSLATED (PE-CH1): 37-word
  opcode `0x70` formation; writes `*actor + i*16 + 0x1C` and
  `*actor + i*4 + 0x7C`; wrapper `0x8001897C`; all 37 words
  exe-verified; `game/boot/func_8002FA10_port.c`
- ~~`func_8002FAA4(actor, …)`~~ — TRANSLATED (PE-CH1): 13-word
  opcode `0xB7` formation; writes `*actor + i*16 + 0x1C` bytes
  `+0..+3` and half `+0xC` (0x70 subset; no `+0x7C`); wrapper
  `0x80018A48`; all 13 words exe-verified;
  `game/boot/func_8002FAA4_port.c`
- ~~`func_8002FF78(tag, value)`~~ — TRANSLATED (PE-CH1): 101-word
  opcode `0x5A` Aya tagged setter through `*D_8009D254`; wrapper
  `0x80018164` (else `func_80030220` for slot tags 40+); all 101
  words exe-verified; `game/boot/func_8002FF78_port.c`
- ~~`func_80030220(actor, tag, value)`~~ — TRANSLATED (PE-CH1):
  197-word opcode `0x5A` slot tagged setter; `D_80010C90[tag-40]`;
  m0005i 40/41/42/50–52; all 197 words exe-verified;
  `game/boot/func_80030220_port.c`
- ~~`func_800299CC_consume_cut(void)`~~ — TRANSLATED (PE-CH1):
  16-word named cut of the battle/field tick consume edge
  (`0x800299CC..0x80029A0C` exclusive, file `0x1A1CC`); both
  guards `record+0x4C & 0x00080000` and `D_8009D28C==6`; `sb 6`
  to `gp+0x10C` (`D_8009CE7C`) then `sw $zero` to `D_8009D28C`;
  all 16 words exe-verified; `game/boot/func_800299CC_port.c`
- ~~`func_800192B8(args)`~~ — TRANSLATED (PE-BTL98): 4-word
  opcode `0x95` `sw $zero, D_8009D28C` / `v0=1`; guest RAM
  so `0x89`/`0x94`/`299CC` agree; `game/boot/func_80017018_port.c`
- ~~`func_800299CC_damage_entry_cut(void)`~~ — TRANSLATED
  (PE-BTL98): `mode==0 && 4D4!=0` jal `1D340(1)` (`2A4FC`,
  `s1=1` from `299F0`); `game/boot/func_800299CC_port.c`
- ~~`func_8001D340` / `func_8001F4D4` / `func_8001F814` / `func_800305C8` / `func_8006DE80`~~
  — TRANSLATED (PE-BTL99): live ATB `+0x10+=+0x24`, `1F4D4`
  through `1F704` `HP-=s0` when record+0x4C bit `0x4000`,
  then `1F814` → `305C8`/`1A680`/`6DE80(0x46A)` if HP!=0;
  D1D0 gated. Death is not at `1F4D4`. `6DFA8`/`6DF50`
  parked; `71A54` = BIOS A(2Fh) rand;
  `game/boot/func_8001D340_port.c`,
  `game/boot/func_8006DE80_port.c`
- ~~`func_8002A7F8_mode3_cut` / `func_8002AA98` / `func_8002B29C` / `func_80053E6C`~~
  — TRANSLATED (PE-BTL100): after real `1F704` HP<=0, `1F078`
  stores mode 3 / `4D4=0` / `1A680(19)` / `6DE80(0x46B)`;
  next tick `2A7F8` mode 3 is `2AA98` xor `2B29C` (zero
  fixture → `2B29C` case 0). `2B0E8` is mode 2.
  `game/boot/func_8002AA98_port.c`,
  `game/boot/func_80053E6C_port.c`
- ~~`func_8006A25C` / `2B29C` cases 1-5~~ — TRANSLATED
  (PE-BTL101): player-death phase drain; case 5 mode=-1
  and dest `0xA9400048`. `game/boot/func_8002AA98_port.c`
- ~~`func_80027D14` / `func_80028E94` / `func_8002F970`~~ —
  TRANSLATED (PE-BTL105): combat-tick parent of `292EC`.
  `game/boot/func_80027D14_port.c`
- ~~`func_800292EC_victory_ready_cut` / `2F300_mode2_cut`~~ —
  TRANSLATED (PE-BTL104): remaining-enemy walk arms mode 2.
  `game/boot/func_8002AA98_port.c`
- ~~`func_80019798` / `func_800392EC`~~ — TRANSLATED
  (PE-BTL108): opcode `0xB2` persist scale.
  `game/boot/func_80017018_port.c`
- ~~`func_80021054` / `23008` / `236E8` / `28574`~~ —
  TRANSLATED (PE-BTL109): Attack arms `body 0x2000`
  then subtracts `body+0x10`.
  `game/boot/func_80027D14_port.c`,
  `game/boot/func_800299CC_port.c`
- ~~`func_80021DE0` / `21F38` / `2312C`~~ — TRANSLATED
  (PE-BTL110): Attack clip completion sets `D294`.
  `game/boot/func_80027D14_port.c`
- ~~`27D14` `0x4000` clear~~ — TRANSLATED (PE-BTL111):
  next tick after `28574` stores `body & ~0x6000`.
- ~~`4B70C` / `4B90C` / `4BB80` / `5E30C`~~ — TRANSLATED
  (PE-BTL112): mode-2 persist installs `4BB80`;
  `5C498` → `512AC(10)` → `534=1000`.
- ~~`1A4AC` `+0x16`~~ — TRANSLATED (PE-BTL113): clip
  high-half after ten `+0x1C` steps is the phase-0 wait.
- ~~`func_8005C498` / `512AC` case 10 / 27D14 DoT~~ —
  TRANSLATED (PE-BTL107): `body+0x10 -= body+0x96`;
  `29A68` stores `5C498` to `gp+0x534`.
  `game/boot/func_80027D14_port.c`,
  `game/boot/func_800299CC_port.c`
- ~~`func_8002A7F8_join_cut` / mode 9~~ — TRANSLATED
  (PE-BTL106): `2AA24` fallthrough after `bne` vs 8;
  no dest store. `34DE0`/`33A40`/`67CBC` deferred.
  `game/boot/func_800299CC_port.c`
- ~~`func_8002B0E8`~~ — TRANSLATED (PE-BTL103): mode-2
  victory phases → mode 9. `703F4`/`4B70C`/`67CBC`
  deferred. `game/boot/func_8002AA98_port.c`
- ~~`3F3C4` bit `0x100` join / `1220C` outer restart~~ —
  TRANSLATED (PE-BTL102): `andi 0x100` after `35558`
  skips draw, dest-change still runs; `1220C` clears
  `0x100` only and `j 1224C`. `game/boot/func_8003F3C4_port.c`,
  `bootstrap/func_8001220C_port.c`
- ~~`func_8002CF24_mode7_cut(void)`~~ — TRANSLATED (PE-BTL2):
  2-word inlined `D_8009D28C=7` store (`0x8002CF24..0x8002CF2C`
  exclusive; `addiu $v0,7` / `sw $v0,0x51C($gp)`); no jal site;
  battle-path only; M0367I dest-ready must not call it;
  `game/boot/func_8002CF24_port.c`
- ~~`func_80019AC0`~~ — TRANSLATED (PE-BTL94): 9-word opcode
  `0xC1` (`0x80019AC0..0x80019AE4`); `D2F0+0x98 |= 0x400`,
  `v0=1`; live M0367I type-2/3 after `0x79`;
  `game/boot/func_80017018_port.c`
- ~~`func_8003F074_dest_ready_cut(token)`~~ — TRANSLATED (PE-BTL90):
  named dest-enter: `6B35C` table clear, `6B4F8` (CE2=hdr+1,
  hdr+0x0C → `B0E70[idB]`, Writer A @ `0x8006B84C`), `6BE4C` /
  `6BECC` Writer B from CE2=10 (not CE2=14), poll `6C5BC` to 0,
  `1A918` (`B1620` → `D1FC`; M0367I `chunk2+0x1C6D8`,
  `lhu(+2)=1`), `125E0`
  type-1. Type-1 `+0x1AC/+0x1B0` stay 0. Does not write
  mode 7/9/10 and does not run 3D050;
  `game/boot/func_8006B4F8_port.c`
- ~~`func_800293F4_hp_cut(void)`~~ — TRANSLATED (PE-BTL2):
  21-word named cut of `func_800293F4` (`0x800293F4..0x80029448`
  exclusive); signed clamp `record+0x0C` to `+0x1C`, copy `+0x0C` to
  `+0x0E`; reached from `0x55` → `144FC` state `0x3A` → `29810`
  `jal` with `a0=0`; does not complete `0x55`;
  `game/boot/func_800293F4_port.c`
- ~~`func_80038D1C(void)`~~ — TRANSLATED (Phase 6E-B15): 11-word
  (0x2C, exe 0x80038D1C–0x80038D44, file 0x2951C) byte test-and-clear
  status leaf: lbu D_80091A20; if nonzero sb 0 → D_80091A20 and return
  0, else return 0xFF (255); conditional write only; all 11 words
  exe-verified (live split 2951C.s); ALSO a matched decomp C leaf
  (src/func_80038D1C.c); two exe call sites, both return-ignored:
  func_8003E680 @0x8003E738 (final call, void epilogue follows) and
  func_8006E9A0 @0x8006EB7C; with this leaf func_8003E680 is FULLY
  translated; `game/boot/func_80038D1C_port.c`
- ~~`func_8006A9E4(void)`~~ — TRANSLATED (Phase 6E-B16): 215-word
  (0x35C, exe 0x8006A9E4–0x8006AD3F, file 0x5B1E4, live split 5B1E4.s)
  PE.IMG streaming resource load: ClearImage({0,0,0x3FF,0x1FF},0,0,1)
  via REAL func_80074F44; four sector-read/poll cycles (table
  D_800930DC..E8, dests D_800A8028 and lw(D_800B0E6C); A/B polls
  restart-on-(-1), C/D sltu-clamped re-poll); 0x10A50-byte copy to
  D_800E2858; two archive lookups (keys 0x57D40D84/0x57D41D84, exact
  delay-slot order D_800B0E20→E18→E1C); 0x1400-byte copy to
  lw(D_800B0E08); all 215 words exe-verified; sole call site
  func_8001220C @0x80012284 (nop slot, return ignored);
  `game/boot/func_8006A9E4_port.c`.  Dependencies translated with it:
  func_8006E6A8 (11 words, retail sector-count forwarding wrapper;
  B54K-AC moves the 0x800-byte conversion into the generic host provider),
  func_8006E7E8 (19 words, poll + D_800B0CD8
  &= 0xFEFFBFFF on st∈{-1,0}), func_8006E498 (31 words, archive lookup,
  guest-address result).  func_80087090 remains UNRESOLVED via the
  centralized boundary.
  func_800527C8 is **TRANSLATED** (Phase 6E-B17):
  49 words / 0xC4 at 0x800527C8, live split 42FC8.s, all exe-verified.
  Multi-subsystem bootstrap dispatcher: 17 calls (16 distinct callees).
  7 translated leaves (func_8005B890, func_8005BC98, func_8004F808,
  func_80042B38, func_80051084, func_800371A4) + 3 direct sw clears;
  every direct callee is now translated, including func_80052C6C,
  func_8005BCBC, func_8005D6F4, func_80051CC4 (B39), and func_80042C78.
  B40 translates the first B39 nested dependency, func_8005332C. B43
  prefix-translates func_8005218C; B44 translates all seven exact
  func_8005B91C output calls, leaving conditional func_80052F24 untouched.
  Sole call site func_8006A9E4 @0x8006AAD0 ($s1-guarded one-shot in
  the cycle-B poll loop); void(void), return unconsumed.
  `game/boot/func_800527C8_port.c`, leaf implementations in
  `game/boot/func_800{5B890,5BC98,4F808,42B38,51084}_port.c`.
- `func_80052C6C`  ← TRANSLATED (B23: resource-table search + init)
- `func_8005BCBC`  ← TRANSLATED (B24: resource-state pointer/count selector)
- `func_80051CC4`  ← TRANSLATED (B39: 77-word resource command-state
  initializer; independent `tools/b39_oracle.py`)
- `func_8005332C`  ← TRANSLATED (B40: 42-word signed-ID resource-record
  lookup; independent `tools/b40_oracle.py`); its direct B39 path now reaches
  the later sibling boundary `func_8005218C`
- `func_80053968`  ← TRANSLATED (B41: 120-word archive-record materializer;
  exact first-free record/ID scans, two ordered 16-byte copy groups, primary
  table-state commit, and pointer-or-zero return; independent
  `tools/b41_oracle.py`)
- `func_80053B48`  ← TRANSLATED (B42: 121-word resource category registrar
  and accumulator; true ABI `int32_t(pe_addr_t record)`, exact ID-table and
  signed-threshold behavior, two executable callers; independent
  `tools/b42_oracle.py`)
- `func_8005218C`  ← PREFIX-TRANSLATED (B43: complete 155-word body and all
  five callers proven; B44 extends production through seven translated
  `func_8005B91C`/`func_8005DBAC` pairs and stops at conditional
  `func_80052F24`; independent `tools/b43_oracle.py`)
- `func_8005B91C`  ← TRANSLATED (B44: complete 87-word signed table-index
  and interpolation logic, 18 direct call sites, exact nullable word outputs;
  independent `tools/b44_oracle.py`)

### func_8006E834 callees (9)
- `func_80086FF8` (shared with 6A5BC)
- `func_8006E6D4(int,int,uchar*,int)`
- `func_800811E4(void*)`
- `func_80072714`
- `func_800726C4`
- `func_80072724`
- `func_800749D8(SetDefDispEnv)`

### func_8006E9A0 callees (7)
- `func_8005E588`
- `func_80066B60(int)`
- ~~`ClearOTagR`~~ — TRANSLATED (Phase 6E-OTC1:
  `game/boot/func_800752AC_port.c`; synchronous OTC fill + tail; the
  NULL-OT skip remains only as a guard — the 6E9A0 loop passes the real
  arena OT since Phase 6E-FD1)
- `func_80068E24`
- ~~`func_80070E54`~~ — TRANSLATED (Phase FTE1:
  `game/boot/func_80070E54_port.c`; frame tail DrawSync/42FE8/VSync/
  74A44/PutDispEnv/PutDrawEnv-or-DrawOTagEnv/CDDC flip)
- ~~`func_80038D1C`~~ — TRANSLATED (Phase 6E-B15, shared with 3E680;
  see the func_8003E680 callees section)

### Phase 6E-B22

`func_8005DE88` is translated retail logic: 23 words at executable
`0x8005DE88..0x8005DEE3`, file offset `0x4E688`, live split `4CC98.s`.
It has no callees, links 12-byte records from `0x800A2090` through
`0x800A2174`, then null-terminates the final link and initializes the six
`$gp` state words at `0x8009D0DC..0x8009D0F0`. Strict real-disc execution now
reaches `func_80052C6C` from `func_800527C8` (exit 1).

### Phase 6E-B24

`func_8005BCBC` is translated retail logic: 21 words at executable
`0x8005BCBC..0x8005BD0F`, file offset `0x4C4BC`, live split `4C4BC.s`.
Resource-state pointer/count selector: stores the incoming record pointer
into `D_8009D0C8` ($gp+0x358), selects a buffer base into `D_8009D0C0`
($gp+0x350) — a0 != 0 → 0x800C20A4 (+0x10 iff byte6(a0) == 9); a0 == 0 →
0x800C0DE0 (+0x10 iff D_8009D218 != 0) — and stores count 8 into
`D_8009D0C4` ($gp+0x354). Returns 8 on every path; both call sites
(func_800527C8 @0x80052834 with a0=0 in the delay slot, func_8004DD64
@0x8004DF28 with a0 = D_8009D004) discard the return. No callees; the
three state words are guest-RAM resident (shared with func_8005BD10,
func_8005BE1C, and the func_8005D6F4 region). Strict real-disc execution
now reaches `func_8005D6F4` from `func_800527C8` (exit 1).

### Phase 6E-B25

`func_8005D6F4` is translated retail logic: 147 words at executable
`0x8005D6F4..0x8005D93F`, file offset `0x4DEF4`, live split `4CC98.s`.
Resource-buffer + display-state initializer: REAL bzero
`0x800C0DE0..0x800C20C3` via func_80071A24; two 0xFF fills of
`0x800C0DF0..0x800C0DF7` (retail reload-per-iteration loops); two
0xFF-terminated string copies whose sources come from the REAL
func_8005DC4C (B26);
`D_8009D0C8/C0/C4` + `D_8009D218` state stores (block-1 order C8, C0,
C4; block-2 order flag, C4, C8, C0); `sh 0x0203 → 0x800C1F80`,
`sw 0x00404040 → 0x800C0E44`; timer-tick clears `0x800A76A4/B0/BC/C8`;
terminator bytes `0x800C20A4/B4`; returns 0xFF (sole call site
func_800527C8 @0x8005283C, return discarded).  After B27, SIX
unresolved callees route through the centralized bootstrap boundary in
retail ROM order: func_8005CCA4, func_800614AC,
func_8005E884, func_8005E850, func_800649D0, func_80052790 — plus the
statically dead func_8005DC9C arm (C8 is always 0 at the selection).

### Phase 6E-B26

`func_8005DC4C` is translated retail logic: 20 words at executable
`0x8005DC4C..0x8005DC9B` (exclusive end = func_8005DC9C), file offset
`0x4E44C`, live split `4CC98.s:1779-1802`.  A read-only PE.IMG
message/string-table lookup over a nested relative-offset archive based
at the cycle-A streaming destination `0x800A8028`:

```
ptr = mem32[0x800A802C] + 0x800A8028
tbl = ptr + mem32[ptr + 4]
return (idx <u mem16[tbl]) ? tbl + sext16(mem16[tbl + 2 + 2*idx]) : 0
```

Both header addresses are sign-extended from `lui 0x800B` + a bit-15-set
immediate, so they are `0x800A80xx`, NOT `0x800B80xx` (same finding as
B23's func_8005DB44).  Record stride is **2** (`sll $v0,$a0,1`) and the
entry is a **signed** 16-bit offset (`lh`), so records must lie within
`tbl ± 32 KiB`.  Zero is retail's own out-of-range return
(`addu $v0,$zero,$zero`), not an invented sentinel.

Signature: ONE argument.  No instruction in the body reads `$a1`; the
values retail leaves there at the call sites are residual (in
func_8005D6F4 the 0xFF fill constant, and at 0x8004685C the jal delay
slot is `sb $v0,0($s1)` — not argument setup at all).

Three guest reads on the failure path, four on the success path, **zero
guest writes**, no callees, no SDK/GTE/GPU/MDEC/SPU/disc/input activity,
cannot block, deterministic from guest state alone, repeated calls
stable.  39 call sites in 24 distinct callers exe-wide; observed constant
indices 3..117, all below the measured count of 120; no call site
compares the return to zero.

Measured against the real Disc 1 USA image: `R = 0x30`, `S = 0x14`,
`count = 120`, entries `242..1971` (records `0x800A815E..0x800A881F`);
`min(entry) == 2 + 2*count` exactly.  Index 30 resolves to `0x800A82A9`
(`10 48 30 FF`).  Independent oracle: `tools/b26_oracle.py`.

The archive is guest-resident and arrives from PE.IMG via cycle A of
func_8006A9E4 before the dispatcher runs.  With the region zeroed the
lookup correctly returns 0 and func_8005D6F4's copy loop dereferences
address 0 — surfaced by the checked-access layer, deliberately not
masked with a KUSEG mirror, a clamp, or a bypass.  Strict real-disc
execution now reaches `func_80053D2C` from `func_8005CCA4` (exit 1,
normal and sanitizer agree).

### Current Phase 6E-B32

`func_800614AC` is translated retail logic: 36 instructions / `0x90` bytes
at executable `0x800614AC..0x8006153B` (exclusive end `0x8006153C`, file
offset `0x51CAC`, live split `asm/disc1/51CAC.s`). It stores
`a0 & 0x00FFFFFF` at `0x8009D14C`, computes the three pairwise byte means
with the retail arithmetic and saturation branches, stores the packed result
at `0x8009D150`, and returns that result. It has no guest reads, direct
callees, hardware activity, blocking, pointer storage, clamping, or fallback.
The `func_8005D6F4` call is at `0x8005D8C8`, with `a0=0x00404040` formed by
the delay-slot `ori`; its return is discarded. The remaining sibling calls
are `func_8005E884`, `func_8005E850`, `func_800649D0`, and
`func_80052790` in raw ROM order. The independent `tools/b32_oracle.py`
verifies the executable SHA-1 and all 36 words, models delay slots, and
checks ordered writes and return values. Native and fresh ASan/UBSan suites
are 344/344. Real-disc strict now stops at `func_8005E884` from
`func_8005D6F4`; bootstrap strict remains at `func_8007F72C` from
`func_800698D4`. Nothing has been pushed and the next rung has not started.

### Historical Phase 6E-B30

`func_80042C78` is translated retail logic through its proven 16-instruction
prefix at executable `0x80042C78..0x80042CB4` (exclusive end `0x80042CB8`,
file offset `0x33478`). With retail `$gp = 0x8009CD70`, it writes zero to
`$gp+0x168/0x170/0x174`, writes `0x20` to `$gp+0x16C`, calls translated
`func_80042CC4` with `a0=0x90` and `a1=0xFF` after the delay slot, and writes
`0x48` to `$gp+0x17C`. The direct guest footprint is exactly
`0x8009CED8`, `0x8009CEDC`, `0x8009CEE0`, `0x8009CEE4`, and `0x8009CEEC`.
The two executable callers are `func_8005CCA4` at `0x8005CFF8` and
`func_800527C8` at `0x8005284C`; both discard the void return.

`tools/b30_oracle.py` independently transcribes and executes all 16 words,
checks every word against executable SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`, and verifies the delayed call
arguments, ordered writes, and return state. The final native and ASan/UBSan
suites were 339/339. B31 supplies the former dependency. No generated or
retail artifacts were added.

### Historical Phase 6E-B31

`func_80042CC4` is translated retail logic: 31 instructions / `0x7C` bytes
at executable `0x80042CC4..0x80042D3C` (exclusive end `0x80042D40`, file
offset `0x334C4`, live split `asm/disc1/334C4.s`). It is a void leaf taking
`(a0, a1)`. It clears `0x800A1878`, executes the initial color-base shift in
the branch delay slot, fills the termination-dependent byte ramp while signed
`lbu < a1` holds, and stores `(cursor - 0x800A1878) + 1` at `0x8009CEE0`
(`$gp+0x170`). The B30 call `(0x90, 0xFF)` produces
`00 90 CF EA F6 FB FD FE FF` and count 9. Direct writes are the actual
subset of `0x800A1878..0x800A1887` plus the word at `0x8009CEE0`; it has no
direct callees or SDK/GPU/disc/audio/input operations. Call sites are
`func_80042C78 @ 0x80042C98` and `func_8005D2B4 @ 0x8005D5E8`; both discard the void return,
and incoming a2/a3 are overwritten before use.

Independent oracle `tools/b31_oracle.py` checks executable SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`, all 31 words, delay slots,
ordered writes, threshold paths, and counts. Native and fresh ASan/UBSan
tests are 341/341. Real-disc strict now stops at `func_800614AC` from
`func_8005D6F4`; bootstrap strict remains at `func_8007F72C` from
`func_800698D4`. Nothing has been pushed and the next rung has not started.

### Historical Phase 6E-B29 (accepted commit `eedd456`)

`func_80053D2C` is translated retail logic: 80 instructions / `0x140` bytes
at executable `0x80053D2C..0x80053E6B` (exclusive end `0x80053E6C`, file
offset `0x4452C`). It scans the shared table at `D_8009D048` with count
`D_8009D050`, calls translated `func_8005DB44`, dispatches record types 1–18,
and returns the exact retail status. B41 completed the types 1–9
`func_80053968` call with `a0 = arg`. B42 proves that types 16–18 call
`func_80053B48` with `a0` equal to the selected guest record (not zero
arguments) and forward its result unchanged. Types 10 and 12–15 store `arg` as one halfword at
the first free table slot; `arg >= 0x100` uses the same store path without a
record lookup. Missing records, type 11, and unknown types return 0; a full
table returns 1. The only direct guest write is that exact halfword store;
there is no blocking, host pointer leakage, low-address mirror, clamping, or
address-zero fallback.

The independent `tools/b29_oracle.py` transcription contains all 80 words,
checks every word against executable SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`, and executes the transcription
with delay slots, table dispatch, exact writes, return paths, and controlled
unresolved calls. The final native and ASan/UBSan suites are 337/337. The
completed B28 provenance remains provisional `a1559ae` plus corrective
`cd2e375`; no generated or retail artifacts were added.

### Historical Phase 6E-B28 (corrective)

`func_8005CCA4` is translated retail logic: 223 words at executable
`0x8005CCA4..0x8005D01F` (exclusive end `0x8005D020`, file offset
`0x4D4A4`).  The provisional B28 implementation is commit `a1559ae`; the
oracle/bootstrap corrective is commit `cd2e375`. A resource-table initializer that: writes 7
halfwords via `*(u16*)func_8005DB8C(i)`, reads PE.IMG archive header via
func_8005DBAC, zeros 50 halfwords descending from **0x800C0EAA** to
**0x800C0E48** (v1 = D_8009D048 + 0x62) — this range is ABOVE the earlier
GA_E24/E28 stores, which survive — searches resource tables, sets the
shared host state globals (D_8009D048/50/58/64, $gp base 0x8009CD70), and
calls func_800438C0(0x3D).  GA_E40 (below the loop) receives a u16 0x3D
store after the loop; GA_E22 is written 1 unconditionally.
Coupled callees: func_800438C0 (8 words, masked-state setter),
func_8005DB8C (8 words, table base), func_8005DBAC (20 words,
clamped table base).  All 223 words are independently transcribed (W_5CCA4),
compared against the SHA-verified executable, and executed by the B28 oracle
(`tools/b28_oracle.py`).  Fixture correction: FxPattern offsets 0x10-0x17
return 0 (retail BSS state).  Split-brain fix: D_8009D018 + 7 globals
moved to extern with PE_Sdk_ResetState reset.  Bootstrap-disc fixture
seeds valid archive at D_800A8028 (R=0x30, S=0x14, count=120,
entry[30] → 0xFF record) so func_8005DC4C returns `0x800A8400`.
`PE_StoreU32(0x800A803C, 0xA49D968F)` makes malformed
`func_8005DBAC(0)` return `0x24A816B7`, without dereferencing that result.
Address zero remains invalid; no KUSEG or low-address mirror exists. 335/335
tests pass (normal + ASan/UBSan).

### Historical Phase 6E-B27

`func_80052594` is translated retail logic: 22 words at executable
`0x80052594..0x800525EB`, file offset `0x42D94`, live split
`42D94.s` (yaml segment `[0x42D94, asm]`).  A leaf string-copy function
that copies bytes from `$a0` (source pointer) into a fixed 8-byte
buffer at `D_80091694`, stopping at the `0xFF` terminator or buffer
full, stores the byte count at `D_8009169D`, and returns the count
(0-8).  The `0xFF` terminator is NOT copied.  No callees, no
`$gp` usage (all addresses via absolute `lui`/`addiu`), no
SDK/GTE/GPU/MDEC/SPU/disc/input activity.

Five executable call sites (`jal` word `0x0C014965`):
`func_8005D6F4` @ `0x8005D898` (delay: `addu $a0,$v0`; return
discarded), `func_8004DD64` @ `0x8004E28C/0x8004E428/0x8004E6A8`
(delay: `addu $a0,$v0`; return discarded), and `func_8005C46C` @
`0x8005C46C` (delay: `nop`; return discarded).  All call sites discard
the return value.

`D_80091694` (8-byte buffer) and `D_8009169D` (1-byte count) are
guest-RAM resident.  Exhaustive executable-wide `lui`/`addiu` scan
found NO readers — these are write-only stores.  Not `$gp`-relative
(`$gp = 0x8009CD70`, offset would be `-0xB6DC`, outside small-data
area).

Independent oracle: `tools/b27_oracle.py`.

## Remaining bootstrap providers

With Disc 1, strict mode now stops at `func_801909B4_80190D7C_cut` from
overlay-local `func_801909B4`, after complete `func_8006AD40`, generic
MoveImage/display setup, both table-derived overlay images, both frame-zero
DrawPrim packets and all 480 iterations of DrawSync, VSync, ResetGraph(1),
PutDrawEnv, and PutDispEnv.
It is the first
BOOTSTRAP_RET provider on the canonical path.
The `--bootstrap-disc` fixture still stops at `func_8007F72C` (CdReady) by
design. Normal and fresh sanitizer builds agree exactly on both frontiers.

Disc-path providers are REAL since Phase 6E-A (host disc model over the
read-only image): `func_8007F72C` (CdReady), `func_8007F778`,
`func_80082314` (PVD verify), `DsSearchFile`, `func_80080C48`
(CdPosToInt), `func_8006E6D4` (image read), `func_800811E4`
(completion poll).  Remaining unresolved providers for boot-to-logo:
- `func_8005DC9C` — dead-arm callee of func_8005D6F4 (C8 always 0);
  the same lookup as func_8005DC4C but reading `ptr+8` instead of
  `ptr+4`, i.e. a second table in the same sub-chunk
- `func_8005CCA4` — completed B28 translated rung
- `func_80053D2C` — completed B29 translated rung; B41 completes its
  `func_80053968` dependency and B42 completes `func_80053B48`
- `func_80042C78` — completed B30 translated prefix; its B31 dependency is
  complete
- `func_80051CC4` — completed B39 translated rung; B40 completed
  `func_8005332C`; B43 prefix-translated `func_8005218C`, and B44 translated
  all seven required-state `func_8005B91C` calls
- `func_80087090` — SPU upload retry wrapper
- `func_800749D8` — display environment setup (currently memset stub)
- ~~`func_800752AC` (ClearOTagR)~~ — TRANSLATED (Phase 6E-OTC1)
