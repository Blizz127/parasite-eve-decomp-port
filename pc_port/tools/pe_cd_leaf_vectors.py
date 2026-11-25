#!/usr/bin/env python3
"""Black-box vectors for small libcd leaves that drive registers or the
stream ring (original code from the user's EXE under the test-only
interpreter): CD_vol 0x8007B964, CD_initvol 0x8007BAC0, StFreeRing
0x8007C394 and StGetNext 0x8007C484.

Each case loads the EXE (its data includes the register pointer globals
0x8009B27C.. = 0x1F801800-0x1F801803, 0x1F801020, 0x1F801C00), seeds the
SPU / CD register blocks the code reads (0x1F801800-0x1F80180F,
0x1F801C00-0x1F801DFF) with seeded halfwords (the interpreter treats
registers as plain memory; reads see the seeded or last-written value),
fills a record zone 0x80100000-0x80101FFF, and calls one entry.

Ring state for StFreeRing / StGetNext: the ring base pointer 0x800C0DC8 is
an address in the record zone, the ring size word 0x800C20C4 is 1..40, the
current index 0x800BE9EC is 0..size, the flag word 0x800C0DBC is 0 or
seeded, and each 32-byte ring entry starts with a halfword from
(0, 1, 2, 4, seeded) and holds a sector count at +6 from (0, 1, 2, 5,
0x7FFF, 0x8000, seeded). StFreeRing gets an address inside or near the
ring; StGetNext gets two output pointers.

Each line: ENTRY seed pattern A args RET r L loads S stores (format of
pe_gpu_leaf_vectors.py). Register stores (0x1F80xxxx) must be reproduced
in exact order by pe_cd_leaf_vectors_check.c: a register write is a
command, not a memory value. Cases with any unlogged access other than
the real stack frame 0x801FE000-0x801FEFFF are dropped (faithful-only),
and cases that exhaust the instruction budget are skipped; both counts are
printed.

CD_flush 0x8007B9EC (`flush`): the interpreter treats registers as plain
memory, so the original's acknowledge-and-wait loop would never end. For
these cases only, the harness adds a CD interrupt-flag model after psx-spx
(CDROM controller): with index 1 selected (low two bits of 0x1F801800),
0x1F801803 reads the pending interrupt type in bits 0-2 with bits 5-7 set,
and writing 1 bits to it acknowledges them. A seeded queue of 0-3 further
responses (types 1-5) surfaces one at a time after each acknowledge clears
the current one. In the second half of the cases, bits 3-4 of the read
value are set at random (bit 4 is "command start" on hardware), which
separates a test of the type bits from a wider mask. Register stores are still logged as written (the
commands); the modelled read values appear in L.

data_ready_callback 0x8007C214 (`callback`): the callback word 0x800B0CC8
is 0 or one of two sentinel addresses (0x80130880 / 0x801308C0, outside
any real code). When the original jumps to a sentinel, the harness records
CALL fn,a0,a1,a2,a3 (the registers at the jump), returns 0 in v0 and
resumes at the return address. Ring state is seeded as for StGetNext, and
the current-index words 0x800BE9E4 / 0x800BE998 are seeded within the ring.
Lines carry a trailing CALL list (`-` when there was none).

_spu_setReverbAttr 0x8008D140 (`reverb`): the argument is a record in the
zone whose first word is a field mask and whose halfwords follow. Masks
are 0, all ones, every single bit, and seeded words; fields are seeded or
edge halfwords (0, 1, 0x7FFF, 0x8000, 0xFFFF). The SPU register block is
seeded as above, and register stores must match in order.

SpuSetReverbModeType 0x8008CF70 (`modetype`): a caller. Its callees
(_SpuIsInAllocateArea_ 0x80085BB4, _spu_setReverbAttr 0x8008D140,
SpuClearReverbWorkArea 0x8008D610, _spu_FsetRXX 0x8007DAE0) are not run:
reaching a callee's entry records CALL fn,a0,a1,a2,a3[,bytes] and returns a
scripted v0 (0/1 seeded for 0x80085BB4, 0 otherwise), then resumes at ra.
For 0x8008D140 the 68 bytes at its argument (a record the caller builds in
its own stack frame) are appended as hex, because the native builds that
record in a guest frame at a different address. The mode argument is
drawn from 0..12, the same with flag 0x100, and seeded words; the reverb
state words 0x8009B390-0x8009B3EF and the SPU block are seeded (the SPU
base pointer 0x8009B3FC keeps its EXE value); 0x80085BB4 returns 1 in a
quarter of the cases. Seeds 6001-8000 replace the 10-record preset table
0x8009C8C0 with synthetic words whose bytes are all nonzero (every retail
record's first (mask) word is 0, which hides whether the caller clears it).

_spu_FsetRXX 0x8007DAE0 and _SpuIsInAllocateArea_ 0x80085BB4
(`spucallees`): the shift word 0x8009B424 is drawn from 0..4 (mostly 3);
the allocation-list pointer 0x8009B464 points into the record zone, at a
list of 1-8 eight-byte entries (address word with flag bits 0x80000000 /
0x40000000 set at random, the last entry always ending the list, plus a
size word). FsetRXX gets index 0..0x1FF (its register stays inside the SPU
block), a seeded value and mode 0, 1 or a seeded word; IsInAllocateArea
gets addresses near and inside the listed ranges, and seeded words. Mode
words include 0x10000 and 0xFFFF0000 (zero low halfword); seeds 4001-5000
use wide shift words (5, 15, 16, 31, 32, 33, 0xFFFFFFFF).

_err_math 0x800739C4 (`errmath`): a caller of DeliverEvent 0x80073A34
(BIOS B(07h)), which is not run: reaching it records CALL fn,a0..a3@n
and returns a scripted v0 (0, 1, 0xFFFFFFFF or a seeded word). Both
arguments are drawn from 0..0x3F, edge words, seeded words and 0..0xFFF;
seeds 4001-6000 put a0 in 0x18..0x2F, alone or with high bits set.

SpuSetReverb 0x80085A64 (`spureverb`): a caller of _SpuIsInAllocateArea_
0x80085BB4 (recorded with a scripted return 0, 1 or -1). The argument is
0, 1, 2, -1, 0x100, 0x101, 0x80000001 or a seeded word; the reverb state
words 0x8009B390-0x8009B3EF and the SPU block are seeded.

_spu_FiDMA 0x8007D614 (`fidma`): the DMA4 completion handler. Callees
_spu_Fw1ts 0x8007DCAC and DeliverEvent 0x80073A34 are recorded; the
callback slot 0x8009B434 holds 0 or a sentinel, and reaching a sentinel
records the call (callback model). SPUCNT (0x1F801DAA) is seeded with each
bit-4/5 pattern or a seeded halfword; its bounded wait loop is replayed
through the register-read model. libspu state words 0x8009B390-0x8009B45F
(not the SPU base pointer) are seeded; 0x8009B44C is 0, 1, 2, -1 or seeded.
Seeds 6001-6400 make SPUCNT bits 4/5 sticky (a store cannot clear them), so
the wait loop's bound and timeout path are observable.

_card_clear 0x8007DD74 (`cardclear`): a caller of the BIOS card stubs
_new_card 0x8007DDC4 and _card_write 0x8007DDB4 (recorded; _card_write
returns 0, 1, -1 or a seeded word). The channel is 0x00/0x10, an edge word
or seeded.

qsort 0x800723A4 (`qsort`): base in the record zone, n 0..40, width 1..16;
each element's first byte is drawn from a 2/4/8/256-value alphabet
(duplicates), the rest are random. The comparator is one of two sentinels:
reaching it records CALL fn,a0,a1 and returns the harness-computed
difference of the two elements' first bytes (ascending, or descending for
the second sentinel), read from current guest RAM; those two byte reads
are logged as loads. The swap helper 0x800724F4 runs as original code.

libcard leaves 0x80082ADC / 0x80082CDC (listed as ENTRY arguments): no
arguments; the word 0x8009B78C is 0, 1, 2, 0x7FFFFFFF, 0x80000000, -1 or
seeded.

libpad RCNT2 pair 0x80084FC4 (setRC2wait, one argument) / 0x80084FE4
(chkRC2wait): the RCNT2 counter, mode (bit 9 toggled) and target at
0x1F801120 / 0x1F801124 / 0x1F801128; the start word 0x800A76D0 (equal to
the counter, just above it (wrap), below it, or seeded) and the limit word
0x800BD02C (small, edge, or at elapsed / elapsed >> 3 and +-1; also the
0x80084FC4 argument).

libcard 0x80083578 (ENTRY): polls SIO0 status through the pointer
0x8009B788 (EXE value 0x1F801040). The status halfword is seeded; an SIO
model (psx-spx bit 1, RX FIFO not empty) reads that bit clear for the first
0..6 reads, then set.

Generated libcd/libds callers (`cdgen`): 0x800813E8, 0x800822BC, 0x80082400,
0x80080F64, 0x8007FB04, 0x8007BBB0 and 0x8007C2A0, each run with its callees
stopped and recorded (scripted returns 0, 1, -1 or seeded). Arguments are
bytes, edge or seeded words, or record-zone pointers; the words each entry
reads (CDGEN_SEED, from a probe run) are seeded with 0, 1, 2, a byte or a
seeded word.

Second batch (`cdgen2`, separate table CDGEN2): CdRom_AbortCmd 0x800808BC,
DsStartReadySystem 0x80081D74, DsEndReadySystem 0x80081DF8 and text_5C4
(AbortCmd: state word 0x8009B574 0..4 and substate 0x8009B578 0..0x20, or seeded)
0x800819D8 (a table search over 0x2C-byte entries from 0x800A3CB4, whose
first word is the key: key 0 is 0..12, the rest of the table small or
seeded, a0 mostly 0..3; in 1 of 8 cases 48 nonzero non-matching keys expose
any fixed entry bound; it calls strcmp 0x80072A54).

Third batch (`cdgen3`, table CDGEN3): DsFlush 0x8007F69C and its caller
0x80081268 (words 0x8009B6CC / 0x8009B6B8 / 0x8009B6C0 seeded), one native file each.

Fifth batch (`cdgen5`, table CDGEN5): unreached small rows from the
2026-10-31 frontier: CdRom_TryIssueCmd 0x8007FB44, 0x8007FCBC, DsLastPos
0x80080CDC, text_508 0x8008227C, text_490 0x80082204, DsReady 0x8007F608
(libcd/libds) and text_2B04 0x80077548, text_2FBC 0x80077A00, text_21CC
0x80076C10 (libgpu). CdRom_GetCurrentPos 0x8007FC28 returns a record-zone
pointer (CDGEN_PTR_RET); text_490 also calls DsPacket 0x8007F0C8, whose fifth
argument (on the caller's stack) is appended to its CALL token
(CALLEE_STACK_ARGS); the state words each entry gates on are seeded;
DsLastPos's dst is NULL or a record-zone pointer; GPU GP0/GPUSTAT reads are seeded.

Sixth batch (`cdgen6`, table CDGEN6): text_130 0x80082444 (a0 == 1 in half
the cases). It passes pointers into its own stack frame (sp+0x10, sp+0x11)
to DsGetSector and strncmp; checkers compare such pointers as offsets from
the case's first frame pointer. cdgen5 (v4) and cdgen6 draw callee returns
independently;
`cdgen-indep` regenerates every cdgen table that way (3000 cases each) as a
cross-check of the earlier batches.

Seventh batch (`cdgen7`, table CDGEN7): text_13CC 0x8007F960, which jumps
through the CD data-callback word 0x800B8AB8. The word holds 0 or the
sentinel CD_CB_SENT (0x80130980); reaching it records CALL fn,a0..a3 like a
stopped callee. The port dispatches such callbacks with
PE_Cd_DispatchDataCallback(target, status, response).

Eighth batch (`cdgen8`, table CDGEN8): 0x800840DC, which calls the libcard
slot D_8009B72C. The slot always holds the sentinel CARD_CB_SENT
(0x801309C0), since the original calls it unguarded; the port dispatches
these slots with PE_Card_ICall (absent_cd_port.c's cd_icall). text_770
0x80083014 is not here (record a0, its +0x3C object pointer, the flag byte
(0x80..0x8F mostly) and the +0x36 byte are seeded for 0x800840DC): text_770
calls through the card-object table
0x8009B7A8, which needs a memory-card object model.

Ninth batch (`cdgen9`, table CDGEN9): the ResetGraph core text_2700
0x80077144 (independent and swept callee returns). Its mode argument covers
every low-3-bit value, with high bits set in some cases; the libgpu ring
words, the saved-mask word 0x80095884 and the DMA2 CHCR / DPCR / GPUSTAT
words are seeded. (Its siblings 0x80077294 / 0x80077404 are parked for
another author: see ACTIVE_HANDOFF.)

Tenth batch (`cdgen10`, table CDGEN10): DS_status 0x800809E0, a debug
printout. Its printf calls (BIOS stub 0x80071A74) are recorded as the
format address plus only the arguments the format consumes (_printf_arity:
registers a1..a3, then the outgoing stack words); the other argument
registers hold leftovers. The libds state words it prints
(0x8009B558..0x8009B59B, 0x800A36A0..A8) are seeded; 0x800822AC's result
selects one of two string addresses.

Eleventh batch (`cdgen11`, table CDGEN11): 0x80080F98 and 0x8008214C,
which jump through the libds callback words 0x8009B6D0 / 0x8009B6F4. Each
word holds 0 or a sentinel: DS_CB2 (0x801309E0, a two-argument site) or
DS_CB3 (0x80130A00, a three-argument site). In a quarter of the cases it
holds the sentinel's KUSEG alias (0x001309E0 / 0x00130A00), a positive
nonzero word, which separates a nonzero guard from a sign test. The port
dispatches these with PE_GuestCall. 0x80080F98's timeout base 0x8009B6C4 is seeded near the
scripted VSync(-1) result minus 1200; the state words each entry gates on
are seeded with edge values.

Twelfth batch (`cdgen12`, table CDGEN12): SetGraphDebug 0x80074BB8. It
calls the libgpu log vector 0x80095748 with four arguments, without a null
check, so the word holds the sentinel GPU_LOG_SENT 0x80130A40 or its KUSEG
alias, never 0; the state bytes
0x8009574C..0x8009574F are seeded with edge values.

Thirteenth batch (`cdgen13`, table CDGEN13): InitCARD 0x8007DDD4 and
StartCARD 0x8007DE40, pure call sequences (critical section, pad-clear,
the BIOS card entries and the libcard setup calls), with independent and
swept callee returns.

Fourteenth batch (`cdgen14`, table CDGEN14): the libcard / libpad slot
callers 0x80082B08 (slot 0x8009B74C, no arguments, null-checked; the record
at *0x8009B784 gates it) and 0x80083DF0 (slot 0x8009B728, one argument,
called without a null check; the object bytes +0x46 / +0x53 gate it).
Sentinels CARD_CB0 0x80130A80 / CARD_CB1 0x80130AC0 and their KUSEG
aliases.

Fifteenth batch (`cdgen15`, table CDGEN15): two small hand-port leaves,
0x800719C4 (the record word +4 seeded around bit 3) and 0x8007E4E0
(_copy_memcard_patch: the 28 source words 0x8007E344.. are randomized so
the vectors show a copy, not constant stores). 2000 cases per entry.

Sixteenth batch (`cdgen16`, table CDGEN16): libds text_904 0x80081D18. It
converts a sector number into a position record in its own stack frame
(0x80080B44), starts a read (0x80080E34) and polls 0x800811E4 while that
returns > 0. The poll is modelled: positive (SEQ_POS) for k calls, then a
final value (SEQ_FINAL); RETS carries k | pos << 8 | final << 12 for it.
Frame pointers are compared as offsets ("F+n").

Seventeenth batch (`cdgen17`, table CDGEN17): libcard _dirFailAuto
0x80084EB0, a card-object retry state machine. It calls the record-reset
slot 0x8009B728 (CARD_CB1 or its alias, never 0: there is no null check).
The object's counter, state bytes, its two record pointers (+0x30, +0x3C,
whose target byte is 0xF3 in a third of the cases) and +0xE8 are seeded.

Eighteenth batch (`cdgen18`, table CDGEN18): libpad text_34 0x80084B78, a
card-object state dispatch. It calls 0x80083E50 (obj, 0 or 1), the
object's own hook word +0x14 (0, CARD_CB1 or its alias; null-checked), or
0x800835C0 (obj). The +0x3C target byte, +0xE8 and the state byte +0x46
are seeded with edge values.

Nineteenth batch (`cdgen19`, table CDGEN19): libapi firstfile 0x800727B4
and text_19C 0x80072950, the hook firstfile installs. Both scan the BIOS
device table: the pointer word at kernel RAM 0x150 and the size in bytes
at 0x154 (80-byte entries; name pointer at +0, function word at +0x34).
These two words are logged as RAM loads (0x80000150..), not scratchpad.
The table (0-12 entries, sizes not a multiple of 80, a base whose end
wraps, base 0 when empty) sits in the record zone, or (some wrap cases)
just below 2^32, where base + size wraps but base + count * 80 may not.
Name pointers come from a pool of four, so entries can share one. strcmp 0x80072A54 is modelled:
0 when its first argument equals one pool pointer (or none), else 1 or -1;
RETS carries that pointer. firstfile gets a device name whose copy stops at
the first signed byte below ';' (59), and calls BIOS firstfile 0x80072A64
(scripted). text_19C gets a pointer to a word set to 1 when 0, and calls
the saved function word 0x800A32D0 (FF_HOOK or its alias, which returns
the same scripted value; no null check) with its three arguments.

Twentieth batch (`cdgen20`, table CDGEN20): libcard card-object methods
0x80083F04, 0x80083F44 and 0x8008401C (generated TUs). They call the slot
words 0x8009B73C / 0x8009B754 / 0x8009B750, which hold SLOT_3C / SLOT_54 /
SLOT_50 or their KUSEG aliases (no null check; an alias returns the same
scripted value as its sentinel), and the libcard SIO callees 0x800830DC /
0x800832B4 (scripted). The original loads no $a0 before the SLOT_54 /
SLOT_50 calls, so their recorded $a0 is leftover (the checker compares
only the target for them). Seeded: the state words 0x8009B760 / 64 / 70 / 74
(64 and 74 equal in half the cases) / 9C / A4, and the card object's
+0x0C word and +0x36 byte.

Twenty-first batch (`cdgen21`, table CDGEN21): libcard SIO wait
0x800834E8 (generated TU). It writes I_STAT and polls SIO_STAT through the
pointer words 0x8009B784 / 0x8009B788 (EXE values 0x1F801070 /
0x1F801040), calling the timeout check 0x80084FE4 while it waits. Model:
SIO_STAT bit 7 reads set for the first k reads (0-7), then clear
(SIO_ACK, separate from the SIO_RX bit-1 model; off for every other
table). 0x80084FE4 returns 0 for j calls (0-9), then WAIT_FINAL[f]; RETS
carries j | f << 8. SIO_STAT, SIO_CTRL and I_STAT are seeded. In a quarter
of the cases the two pointer words point into the record zone instead
(plain memory, no device model).

Twenty-second batch (`cdgen22`, table CDGEN22): libcard SIO byte exchange
0x800830DC (obj, byte; a negative byte selects a send path) and byte write
0x800832B4 (obj, byte), currently hand ports. They read and write the SIO
block through the pointer word 0x8009B788, I_STAT through 0x8009B784, and
(0x800832B4) the RCNT2 counter, mode and target directly, and call the
RCNT2 timeout pair 0x80084FC4 (one argument) / 0x80084FE4 (no argument;
0 for j calls, then WAIT_FINAL[f]; RETS j | f << 8). Register models
(REGBITS, REGCOUNT; off for every other table): SIO_STAT bit 0, SIO_STAT
bit 1 and I_STAT bit 7 each read clear for their first k reads (0-5), then
set; the RCNT2 counter advances by a seeded step (1, 8, 0x40, 0x100,
0x1000) on every read. Seeded: the card object (+0x3C buffer whose first
byte's high nibble is 8 in most cases, +0x40 pointer, +0x44 index, +0x45
count, +0xE8 byte), the state words 0x8009B768 / 0x800A76D0 / 0x800BD02C
(including values above 0xFFFF) and the other SIO / RCNT2 registers; the
RCNT2 target is 0 in a quarter of the cases. In a third of the 0x800832B4
cases the timeout is put on its boundary: RCNT2 mode bit 9 set, I_STAT
bit 7 clear for the first 3 reads, and the limit word equal to the first
loop pass's elapsed count minus 1, exactly, or plus 1, for counters below
the start (a wrap, with target 0 or not) and above it.

Twenty-third batch (`cdgen23`, table CDGEN23): libpad padif transfer
0x80084168 (obj; currently a hand port). It calls five libcard/libpad slot
words with no null check: 0x8009B72C with two arguments (sentinel
0x801309C0, the cdgen8 slot), 0x8009B730 / 0x8009B744 / 0x8009B748 /
0x8009B724 with one (sentinels 0x80130D00 / 0x80130D40 / 0x80130D80 /
0x80130DC0); each word holds its sentinel or the KUSEG alias, and both are
stopped. The port calls them through PE_Card_ICall. Direct callees: the
SIO pair 0x800832B4 / 0x800830DC (obj, byte), the RCNT2 timeout start
0x80084FC4 (one argument), the SIO acknowledge wait 0x800834E8 and the
RX-ready wait 0x80083578 (no arguments). Call sequences (RETS carries each
model's code): 0x800832B4 / 0x800830DC return XCH23_OK[ok] for their first
k calls (k = 15: every call), then XCH23_FIN[fin], and on call m write the
byte obj+0x44 = XCH23_IDX[bv] (code k | ok << 4 | fin << 6 | m << 8 |
bv << 12); 0x800834E8 returns ACK23_POS[p] for its first k calls, then 0,
and on call m writes the counter 0x8009B79C, both pair words 0x8009B77C /
0x8009B780 and (bit 20) rewrites every slot word to sentinel or alias as bits 21..25
say (code k | p << 4 | m << 8 | cv << 12 | pv << 16 | slots << 20 |
forms << 21; absolute, v2). v3 discriminators: the slot 0x8009B744 writes
both pair words on its call m (RETS for 0x80130D40: m | pv << 4; the
caller re-reads the pair word after that call), and in an eighth of the
cases the byte pointer 0x8009B7A0 points at obj+0x44. These
callee writes appear in the store list in call order. Seeded: the object
(+0x0C word, +0x36 byte, +0x3C buffer whose first byte's high nibble is 8
or not, +0x44 index), the state words 0x8009B764 / 0x8009B770, the counter
0x8009B79C (dense small values, 0x80000000, rarely 0x7FFFFFFF), the pair
words (-2..6 and the word extremes), the record base 0x8009B758 (a zone
pointer; its +0x0C / +0xFC words seeded) and the byte pointer 0x8009B7A0.

Twenty-fourth batch (`cdgen24`, table CDGEN24): libpad text_770 0x80083014
(obj; currently a hand port), the card transfer step. It post-increments
the index word 0x8009B768 and calls the step-table entry it selects
(0x8009B7A8 + 4 * index, no bound check) with obj; then the RCNT2 timeout
start 0x80084FC4 (one argument) and the SIO acknowledge wait 0x800834E8 (no
argument), and the error hook 0x8009B724 (one argument). The table words
0x8009B7A4 + 4 * k (k = 0..10; index -1..9) each hold their own sentinel
STEP24 + 0x40 * k or its KUSEG alias, and the index word takes -1..9 and
0x7FFFFFFF / 0x80000000 (4 * index wraps onto entries -1 / 0). Every
entry shares one model: it returns STEP24_RET[r] and, with bit 3, writes the
index word = STEP24_IDX[v] (RETS for STEP24: r | mut << 3 | v << 4). The
hook (sentinel 0x80130DC0) writes the index word = STEP24_IDX[v] with bit 0
(RETS mut | v << 1). The function's return value is a leftover register
that its only caller (0x80082C94) overwrites; the port declares it void.

Twenty-fifth batch (`cdgen25`, table CDGEN25): libcard MemCard_TimerCallback
0x80082B70 (no arguments; currently a hand port). It sets 0x8009B78C,
bumps the 150-capped counters 0x800A5AC0 / 0x800A5AC4 under the record words
0x8009B774 / 0x8009B778, and, when 0x8009B75C is set, calls the sector
routine 0x80082E00 (record; stays a hand port, stopped here), the error
hook 0x8009B724 (one argument) and, in a loop over records at 0x8009B758 +
0xF0 * index, the transfer step 0x80083014 (record). It finally stores a
halfword through the SIO pointer 0x8009B788 (the register block or a zone
word). Callee models (RETS carries each code), all writes absolute:
0x80083014 rewrites 0x8009B764 on every call (SEQ25 values, then
0x7FFFFFFF on its call m, which ends the loop) and 0x8009B778 on call j;
0x80082E00 returns RET25[r] and may rewrite 0x8009B764; the hook may
rewrite 0x8009B764.

Twenty-sixth batch (`cdgen26`, table CDGEN26): libpad text_108 0x80084C4C
(obj; currently a hand port), the record step installed in slot
0x8009B744. It copies the record's received bytes (+0x3C buffer) into its
+0x30 buffer, consults the busy query 0x80084F8C, calls the record-reset
slot 0x8009B728 (sentinel 0x801310C0, one argument, no null check), and
steps the state byte +0x46 through the object's +0x18 hook (sentinel
0x80131100 or 0; one argument) or 0x80083644. The reset slot, the hook and
0x80083644 may each rewrite obj+0x46 (and obj+0x36) with absolute values
(RETS r | mut << 3 | mut36 << 4 | v << 5 | b36 << 9); the hook and
0x80083644 return RET26[r]. Its return value is unused by every caller
(the slot calls at 0x80082ED4 / 0x80082F38 / 0x800842EC.. ignore $v0); the
port declares it void.

Twenty-eighth batch (`cdgen28`, table CDGEN28): libds text_FC 0x80081E70
(intr byte, result pointer; void; currently a hand port), the DS ready
callback that 0x80081D74 / 0x80082204 install through 0x800824C8. It works
on the ready-system block 0x8009B6EC.. and 0x8009B6F8.., passes a buffer in
its own stack frame to getsector 0x80080AE4, CdPosToInt 0x80080C48 and the
three-argument callback word 0x8009B6F4 (DS_CB3; compared frame-relative,
"F+n", as in cdgen6 / cdgen16), and calls the still-hand-ported DsCommand
0x8007EE84 and DsDataCallback 0x800824F0. Models (RETS codes): CdPosToInt
returns the live position word *0x8009B6EC plus a coded delta, so the
equality branch is exercised; the callback may rewrite 0x8009B6F8 /
0x8009B6FC and text_490 0x80082204 may rewrite 0x8009B6FC (absolute).
v2 adds counter seeds -2 / -3 / -100 / 0x80000001 (negatives other than
-1), interrupt bytes that equal 1 or 4 only in their low bits, and
pending-word values 3 / 0x101.

Twenty-ninth batch (`cdgen29`, table CDGEN29): libcd CD_getsector2
0x8007C044 (buffer, word count; currently a generated TU). It reaches every
register through the pointer words 0x8009B27C (index/status 0x1F801800),
0x8009B288 (0x1F801803), 0x8009B28C (0x1F801020), 0x8009B2B0 (0x1F801018),
0x8009B2B4 (DPCR 0x1F8010F0) and 0x8009B2B8 / B2BC / B2C0 (DMA3 MADR / BCR /
CHCR), waits on a status bit and starts DMA3; it calls nothing. Register
model (REGBITS): status bit 6 (0x40) reads clear for its first k reads
(k = 0..5, 8, 12), then set; in half the cases other status bits (a seeded
nonzero subset of 0xBF) read set on every read, which separates a test of
bit 6 from a wider one. Seeded: the arguments (edge and seeded words; word
counts with bit 16 already set, 0xFFFF0000, 0xFFFFFFFF), the initial DPCR
word (bit 15 set and clear, 0, all ones) and the other register words.
Discriminators: in a quarter of the cases one pointer word is moved into the
record zone (plain memory): the DPCR, BCR, 0x1F801018 or 0x1F801020 word, or
the MADR word aimed at the status word 0x8009B27C (with a0 a zone byte that
has bit 6 set), or the BCR word aimed at the CHCR word 0x8009B2C0 (a1 a zone
address); the original reloads 0x8009B27C before its wait and 0x8009B2C0
before its final read, so these decide whether a pointer may be cached.
v2: the checker seeds RAM from each address's first load, so a pointer
word rewritten by an earlier store only discriminates when the original
also loads its old value first. v2 therefore aims the DPCR word (read, then
rewritten with bit 15 set) at the MADR word 0x8009B2B8 (which holds a zone
address), and in the BCR -> CHCR case at the CHCR word, and adds a MADR ->
BCR case (a0 a zone address) with the DPCR word aimed at the BCR word too;
these decide the order of the MADR, BCR and CHCR pointer-word loads against
the stores before them. The order of the first four (constant) stores
against later pointer-word loads stays undecided: their values cannot be
aimed at plain memory.
Cases whose wait never ends spin out and are skipped.

Thirtieth batch (`cdgen30`, table CDGEN30): libds text_360 0x8007E8F4 (no
arguments; currently a host adapter in pe_libcd.c). It calls the status-lane
getter 0x8007FBF0 (pe_plat_ds_status_lane) and, depending on its result and
on a pending-command record, CdRom_TryIssueCmd 0x8007FB44
(pe_plat_cd_try_issue_cmd); both are stopped with independent scripted
returns (RETS in table order). Seeded: the queue index word 0x800A3604
(small, edge and negative values; index 8 puts a record over the index word
itself) and the 24-byte records around 0x800A3540 (first word 0, small,
sign-bit or all-ones; full-range bytes; seeded words). v2: 0x8007FB44 joins
CDGEN_HIGH_RETS (some returns set only bits 16..31), which decides a
full-word versus low-half nonzero test of its result.

Thirty-first batch (`cdgen31`, table CDGEN31): libds text_5F4 0x8007EB88
(sequence word, status, response pointer; void; currently a hand port in
cd_stream_port.c), the command-queue push. It works on the ring index word
0x800A3690 and 16-byte entries at 0x800A3610 and calls copy8 0x80080998
(pe_plat_cd_copy8; stopped, recorded with its two arguments). Seeded: the
index word (0..7, 8 (entry 8's first word is the index word itself, so the
sequence store rewrites the index; the sequence is then small), 9, 15,
-1, 0x7FFFFFFF, 0x80000000), the sequence (edge and seeded words), the
status (bytes and words above 0xFF) and the response (zone pointer or seeded
word).

Thirty-second batch (`cdgen32`, table CDGEN32): StSetStream 0x8007C304.
Five independent full-width arguments with signed/unsigned edges; the fifth
is seeded in the o32 incoming argument area (initial sp+0x10), not $t0.
The range setter 0x8007C544 is stopped and its three arguments recorded;
its return is ignored by the void host API. Global words around the output
locations are nonzero seeded values. No target hand-port or test bodies
were used in designing these vectors.

Thirty-third batch (`cdgen33`, table CDGEN33; sw-cd 2026-11-25): remaining
libcd/libds rows currently hand ports.  DsDataCallback 0x800824F0 (one
argument; calls 0x80073CF4), DsSystemStatus 0x8007F72C (no arguments; calls
the status-lane getter 0x8007FBF0 and the queue-length query 0x8007F778;
both stopped with independent, swept returns) and Cd_SetIntrMask / text_1688
0x8007C13C (no arguments; polls the acknowledge worker 0x8007AAB4 through
the CD register pointer word 0x8009B27C).  0x8007AAB4 returns POS33[p] for
its first k calls, then FIN33[f]; on its first call it also writes the byte
at the register pointer's target = IDX33[w] (w == 8: no write; in production
the worker selects register index 1, so the register byte changes under the
caller).  While 0x8007AAB4 returns nonzero the entry dispatches the callback
words 0x8009AFB8 (result bit 2; sentinel 0x80131180, or its alias) and 0x8009AFB4
(bit 1; sentinel 0x80131140) with the status byte at 0x8009B295 / 0x8009B294
and the response address 0x800A3468 / 0x800A3460 (null words are skipped); the
CD index byte read before the loop is restored (& 3) after it.  Every nonzero
final result of 0x8007AAB4 spins forever, so FIN33 is all zero.  RETS code k | p << 4 | f << 8 | w << 12.  In a quarter of the
cases the register pointer word is moved into the record zone (plain
memory).  For 0x8007F72C a third of the results are drawn from discriminator sets (0x8007FBF0: 1 and
values that only share low bits with 1; 0x8007F778: positive values with a zero low half, 0x7FFFFFFF,
0x80000000, -1).

Thirty-fifth batch (`cdgen35`, table CDGEN35): the SIO0 start routine
0x80082E00 (guest name `_padInitSioMode` in the classification; one argument,
a 0xF0-byte pad/card record).  Fresh-author batch (sw-pad, 2026-11-25): the
design came only from running the original under the harness with the callees
reported by `_check_callees_complete` (0x80083578, 0x80084FC4) stopped and
the slot words 0x8009B744 / 0x8009B748 pointing at sentinels
(SLOT23_44 / SLOT23_48 or their KUSEG aliases), then observing which words it
reads and writes.  No target hand-port, test body or disassembly was used.
Seeds: the record (+0xC word, +0x36 / +0x50 bytes, +0x3C pointer, +0xE8 byte,
filler), the index word 0x8009B764 (0, 1, -1, 4..8, -2 and wrap values) and
every counter cell 0x8009B774..0x8009B79C that 0x8009B77C + 4 * index can
reach (small, negative and edge words; positive counts stay <= 6), the
interrupt-status pointer word 0x8009B784 and the SIO base word 0x8009B788
(EXE value, or a record-zone pointer in some cases).  SIO status
(0x1F801044) and control (0x1F80104A) reads follow per-case value sequences
(SIO35_SEQ), so the first and second status reads differ; the data-register
read after the byte write is seeded in half of the cases.  The two slots
return scripted values and, on their m-th call, may rewrite one word the
routine reads later (record +0xC / +0x36 / +0x50 / +0x3C, a counter cell,
the index word, the status pointer), absolutely, so a native that hoists a
read fails.  0x80083578 and 0x80084FC4 return scripted values the original
ignores.

Thirty-fourth batch (`cdgen34`, table CDGEN34): the libpad communication
start routine 0x80082CF0 (guest name PadStartCom in the classification; no
argument).  Fresh-author batch (sw-padstart, 2026-11-25), designed from harness
runs only: `_check_callees_complete` reported the five static callees
(0x80072714, 0x80072724, 0x80073C84, 0x8007E1E4, 0x8007E1F4), all stopped with
scripted returns, and one-variable grids over the guest words the routine reads
showed which words matter (the slot word 0x8009B728 and the argument word
0x8009B758 are read twice, the interrupt-status pointer word 0x8009B784 once)
and where each read sits relative to the callees (a callee write that changes a
later read proves the read comes after that call).  The callees' scripted
returns are ignored by the original; each callee's RETS value is instead a
code for an absolute write it makes on its call (the checker's stub repeats the
write): the interrupt-status pointer word (the EXE value or a record-zone
pointer), the mask word behind a zone pointer, the argument word, the slot word
(0, a sentinel, its KUSEG alias, a second sentinel), an unread guest word, or
the counter words.  The slot sentinels 0x80131240 / 0x80131280 return and may
write on their first or second call.  The mask register (0x1F801074) is seeded
per case.  No target hand-port, test body or disassembly was used.

Usage: pe_cd_leaf_vectors.py OUT.txt [ENTRY ... | flush | callback | reverb | modetype | spucallees | errmath | spureverb | fidma | cardclear | qsort | cdgen | cdgen2 | cdgen3 | cdgen5 | cdgen6 | cdgen7 | cdgen8 [sweep] | cdgen9 | cdgen10 | cdgen11 | cdgen12 | cdgen13 | cdgen14 | cdgen15 | cdgen16 | cdgen17 | cdgen18 | cdgen19 | cdgen20 | cdgen21 | cdgen22 | cdgen23 | cdgen24 | cdgen25 | cdgen26 | cdgen28 | cdgen29 | cdgen30 | cdgen31 | cdgen32 | cdgen33 | cdgen34 | cdgen35 | cdgen-indep | cdgen-sweep]
Thirty-eighth batch (`cdgen38`, table CDGEN38; sw-cd 2026-11-25): libds
DsGetDiskType 0x80082314 (no arguments; currently a host adapter).  Callees
stopped and recorded: DsPacket 0x8007F0C8 (five arguments; its record pointer
is in the caller's frame, compared as "F+n"), DsSystemStatus 0x8007F72C,
the lane getter 0x8007FBF0 (a0 recorded; first read LANE38[i0], later ones LANE38B[i1], RETS code i0 | i1 << 4; a second read of 0x10 makes the function return 0x10), CdIntToPos 0x80080B44,
DsEndReadySystem 0x80081DF8 and 0x800822AC.  DsSystemStatus follows a coded
sequence (RETS code m | idx_i << 4i): m calls whose values come from SV38
(odd calls never 1, even calls never 3) and end with 1 at an odd call or 3 at
an even one (every other sequence waits forever and is not generated).  The
lane getter returns one value (2 in about a third of the cases); DsPacket
returns 0 or a nonzero word.  When DsPacket is nonzero the original polls the
plain word 0x800B28F8 (which the command-complete callback text_EC writes in
production): MEMSEQ, a harness model of that asynchronous event (psx-spx: the
drive answers some frames after the command), feeds k zeros, then a nonzero V
and then W for the final load; the checker replays those reads in order.

Thirty-ninth batch (`cdgen39`, table CDGEN39, first entry; sw-cd 2026-11-25): libds text_170
0x8007E704 (status, response; void; currently a hand port in cd_stream_port.c),
the command-queue flush.  It walks the ring of 0x18-byte entries at
0x800A3540 (head word 0x800A3600, count word 0x800A3608), calls the queue
push 0x8007EB88 (stopped, three arguments recorded) with (word 0, the low byte of status, response) for every entry whose
word 0 is nonzero, clears the queue block (75 zero stores) and then calls each
entry's callback word (+0x10) with (status byte, response).  Callback words are 0, a
sentinel (0x80131200 / 0x80131240) or its KUSEG alias; the port dispatches them
through PE_Cd_DispatchDataCallback.  Seeded: head (0..9, 15, -1, 0x7FFFFFFF,
0x80000000: the wrap test is signed), count (0..16, -1, 0x80000000), the entry
words and the response (zone pointer or word).  0x8007EB88 may rewrite the count
word on its call m (RETS code m | v << 4; absolute, logged), which decides
whether the loop reloads the count each pass.

Thirty-ninth batch, second entry (`cdgen39`, table CDGEN39; sw-cd 2026-11-25): libds text_3D0
0x8007E964 (status, response; void; currently a hand port in cd_stream_port.c),
the command-complete / error handler.  It works on the ring of 0x18-byte
entries at 0x800A3540 (read index 0x800A3604, write index 0x800A3600, count
0x800A3608; entry +0 sequence, +4 byte, +0xC word, +0x10 callback word, +0x14
retry word), records the sequence and status at 0x800A3510 / 0x800A3514
(copy8 0x80080998 of the response to 0x800A3515), handles status 2 (advance the
read index when the next entry has the same sequence, else push 0x8007EB88,
run the entry callback, pop 0x8007E5C4) and status 5 (retry: read index :=
write index and decrement the retry word, or the same push / callback / pop),
dispatches the global callback word 0x800B8AB0, and while the lane getter
0x8007FBF0 reads 1 and the count is positive issues 0x8007FB44(byte +4, word
+0xC) for the current entry.  Callback words (entry: 0x80131300 / 0x80131340,
global: 0x80131380 / 0x801313C0, or the KUSEG alias) go through
PE_Cd_DispatchDataCallback.  The lane getter returns LV39[i1] on its first call
and LV39[i2] on later ones (RETS code i1 | i2 << 4); 0x8007E5C4 may rewrite the
count word (RETS code bit 0, value CW39[code >> 4 & 7]; absolute, logged), which
shows that the tail reads it after the pop.

Usage: pe_cd_leaf_vectors.py OUT.txt [ENTRY ... | flush | callback | reverb | modetype | spucallees | errmath | spureverb | fidma | cardclear | qsort | cdgen | cdgen2 | cdgen3 | cdgen5 | cdgen6 | cdgen7 | cdgen8 [sweep] | cdgen9 | cdgen10 | cdgen11 | cdgen12 | cdgen13 | cdgen14 | cdgen15 | cdgen16 | cdgen17 | cdgen18 | cdgen19 | cdgen20 | cdgen21 | cdgen22 | cdgen23 | cdgen24 | cdgen25 | cdgen26 | cdgen28 | cdgen29 | cdgen30 | cdgen31 | cdgen32 | cdgen33 | cdgen38 | cdgen39 | cdgen35 | cdgen-indep | cdgen-sweep]
"""
import os
import re
import struct
import sys

import pe_gpu_leaf_vectors as gv

v = gv.v
pl = gv.pl
ENTRIES = (0x8007B964, 0x8007BAC0, 0x8007C394, 0x8007C484)
FRAME = (0x1FE000, 0x1FF000)
UNFAITHFUL = [False]
_logged = pl._logged
RING_BASE, RING_SIZE, RING_CUR, RING_FLAG = 0xC0DC8, 0xC20C4, 0xBE9EC, 0xC0DBC
ZONE = 0x80100000


def _faithful_logged(a):
    r = _logged(a)
    if not r and not FRAME[0] <= a < FRAME[1]:
        UNFAITHFUL[0] = True
    return r


pl._logged = _faithful_logged


class CdIrq:
    """psx-spx CD interrupt flags (index 1, 0x1F801803): write-1-to-ack,
    queued responses surface after the current one is cleared."""
    active = False
    flags = 0
    queue = []

    noise = 0          # bits 3-4 (psx-spx: bit 4 "command start", bit 3 unknown)

    @classmethod
    def refresh(cls, buf):
        if buf[0x1800] & 3 == 1:
            buf[0x1803] = (cls.flags & 7) | (cls.noise & 0x18) | 0xE0


_base_store = v._store


def _cd_store(fmt, buf, a, val):
    _base_store(fmt, buf, a, val)
    if not CdIrq.active or not 0x1800 <= a < 0x1804:
        return
    if a == 0x1803 and buf[0x1800] & 3 == 1:
        CdIrq.flags &= ~val & 0xFF
        if CdIrq.flags & 7 == 0 and CdIrq.queue:
            CdIrq.flags = CdIrq.queue.pop(0)
    CdIrq.refresh(buf)


v._store = _cd_store

SPU_STICKY = [0]       # fidma discriminator: SPUCNT bits that a store cannot clear

# SIO0 status model (0x1F801044): bit 1 (RX FIFO not empty, psx-spx) reads
# clear for the first SIO_RX[0] reads, then set; None = plain memory.
SIO_RX = [None, 0]
_pl_load = pl._ns['LOAD']


def _sio_load(fmt, buf, a):
    if SIO_RX[0] is not None and (a & ~1) == 0x1044:
        SIO_RX[1] += 1
        h = struct.unpack_from('<H', buf, 0x1044)[0]
        struct.pack_into('<H', buf, 0x1044, (h | 2) if SIO_RX[1] > SIO_RX[0] else (h & ~2))
    return _pl_load(fmt, buf, a)


pl._ns['LOAD'] = _sio_load

# Plain-memory wait model (cdgen38; a harness model after the CD driver, not
# behaviour of the original): MEMSEQ[offset] = [values, reads].  A word that
# guest code polls and that only an asynchronous event (a command callback run
# from the CD interrupt, psx-spx: the drive answers some frames after the
# command) can change reads `values[i]` on its i-th read (the last value
# repeats), written into RAM just before the load.  Empty = off.  Like the
# SIO / CD register models it is a hardware-event stand-in, so the checker
# replays these reads in order (mmio_read) and the native's wait loop needs
# the host wait contract (tick per poll, stop budget).
MEMSEQ = {}
_sio_load_chain = pl._ns['LOAD']


def _mem_load(fmt, buf, a):
    m = MEMSEQ.get(a & ~3)
    if m is not None:
        vals = m[0]
        struct.pack_into('<I', buf, a & ~3, vals[min(m[1], len(vals) - 1)])
        m[1] += 1
    return _sio_load_chain(fmt, buf, a)


pl._ns['LOAD'] = _mem_load


_cd_store_chain = v._store


def _spu_sticky_store(fmt, buf, a, val):
    _cd_store_chain(fmt, buf, a, val)
    if SPU_STICKY[0] and a == 0x1DAA:
        struct.pack_into('<H', buf, a, (struct.unpack_from('<H', buf, a)[0] | SPU_STICKY[0]) & 0xFFFF)


v._store = _spu_sticky_store


def prep(base, seed):
    ram, x = gv.prep(base, seed)
    for a in list(range(0x1800, 0x1810, 2)) + list(range(0x1C00, 0x1E00, 2)):
        x = gv.xs32(x)
        struct.pack_into('<H', ram, a, (x >> 8) & 0xFFFF if x % 4 else 0)
    x = gv.xs32(x)
    size = 1 + x % 40
    struct.pack_into('<I', ram, RING_SIZE, size)
    x = gv.xs32(x)
    ring = ZONE + (x % 0x400) * 4
    struct.pack_into('<I', ram, RING_BASE, ring)
    x = gv.xs32(x)
    struct.pack_into('<I', ram, RING_CUR, x % (size + 1))
    x = gv.xs32(x)
    struct.pack_into('<I', ram, RING_FLAG, 0 if x & 1 else x)
    for i in range(size + 1):
        e = (ring & 0x1FFFFF) + 32 * i
        x = gv.xs32(x)
        struct.pack_into('<H', ram, e, (0, 1, 2, 4, x >> 16)[x % 5])
        x = gv.xs32(x)
        struct.pack_into('<H', ram, e + 6, (0, 1, 2, 5, 0x7FFF, 0x8000, x >> 16)[x % 7])
    return ram, x, ring, size


def flush_cases(base):
    out, spun = [], 0
    for seed in range(1, 6001):
        s = seed * 7919
        ram, x, ring, size = prep(base, s)
        x = gv.xs32(x)
        CdIrq.active = True
        CdIrq.flags = (0, 1, 2, 3, 4, 5, x >> 8)[x % 7] & 0xFF
        x = gv.xs32(x)
        CdIrq.queue = [1 + (x >> (3 * i)) % 5 for i in range(x % 4)]
        CdIrq.noise = ((x >> 12) & 0x18) if seed > 3000 else 0     # second half: bits 3-4 set at random
        x = gv.xs32(x)
        ram[0x1800] = x & 0xFF           # whatever index was selected before
        CdIrq.refresh(ram)
        UNFAITHFUL[0] = False
        try:
            res = gv.run(ram, 0x8007B9EC, [])
        except AssertionError as e:
            if 'instruction budget' not in str(e):
                raise
            spun += 1
            continue
        finally:
            CdIrq.active = False
        if not UNFAITHFUL[0]:
            out.append(gv.line(0x8007B9EC, s, 0, [], res))
    return out, spun


SENTINELS = (0x80130880, 0x801308C0)


class Stop:
    def __init__(self, pcs):
        self.pcs, self.hit = pcs, None

    def __contains__(self, pc):
        if pc in self.pcs:
            self.hit = pc
            return True
        return False


def run_with_callbacks(ram, entry, args):
    pl.LOADS.clear()
    pl.STORES.clear()
    pl.CUR[0] = entry
    calls = []
    regs = {i: a for i, a in enumerate(args, 4)}
    regs[31] = v.RET
    pc = entry
    for _ in range(8):
        st = Stop((v.RET,) + SENTINELS)
        r = v.execute(ram, pc, (), stop_at=st, initial_regs=regs, instruction_budget=10000)
        if st.hit not in SENTINELS:
            return r[2] & 0xFFFFFFFF, list(pl.LOADS), list(pl.STORES), calls
        calls.append((st.hit,) + tuple(x & 0xFFFFFFFF for x in r[4:8]))
        r[2] = 0
        regs = dict(enumerate(r))
        pc = r[31]
    raise AssertionError('instruction budget (callbacks)')


def callback_cases(base):
    out, spun = [], 0
    for seed in range(1, 6001):
        s = seed * 7919
        ram, x, ring, size = prep(base, s)
        x = gv.xs32(x)
        struct.pack_into('<I', ram, 0xB0CC8, (0, SENTINELS[0], SENTINELS[1])[x % 3])
        x = gv.xs32(x)
        struct.pack_into('<I', ram, 0xBE9E4, x % (size + 1))
        x = gv.xs32(x)
        struct.pack_into('<I', ram, 0xBE998, x % (size + 1) if x & 1 else x)
        UNFAITHFUL[0] = False
        try:
            ret, loads, stores, calls = run_with_callbacks(ram, 0x8007C214, [])
        except AssertionError as e:
            if 'instruction budget' not in str(e):
                raise
            spun += 1
            continue
        if UNFAITHFUL[0]:
            continue
        out.append(gv.line(0x8007C214, s, 0, [], (ret, loads, stores)) + ' CALL ' +
                   (' '.join('%08X,%08X,%08X,%08X,%08X' % c for c in calls) or '-'))
    return out, spun


def reverb_cases(base):
    out = []
    masks = [0, 0xFFFFFFFF] + [1 << b for b in range(32)]
    for seed in range(1, 4001):
        s = seed * 7919
        ram, x, ring, size = prep(base, s)
        x = gv.xs32(x)
        rec = gv.ptr(x >> 3)
        x = gv.xs32(x)
        mask = masks[seed % len(masks)] if seed % 3 else x
        struct.pack_into('<I', ram, rec & 0x1FFFFF, mask)
        for k in range(40):
            x = gv.xs32(x)
            struct.pack_into('<H', ram, (rec & 0x1FFFFF) + 4 + 2 * k,
                             (0, 1, 0x7FFF, 0x8000, 0xFFFF)[x % 5] if x & 0x10000 else x >> 16)
        UNFAITHFUL[0] = False
        res = gv.run(ram, 0x8008D140, [rec])
        if not UNFAITHFUL[0]:
            out.append(gv.line(0x8008D140, s, 0, [rec], res))
    return out


CALLEES = {0x80085BB4: None, 0x8008D140: 68, 0x8008D610: None, 0x8007DAE0: None}


CALLEE_STACK_ARGS = {0x8007F0C8: 1}   # DsPacket(mode, loc, count, a3, buf): buf is on the stack


PRINTF = 0x80071A74   # BIOS printf stub


def _printf_arity(ram, fmt):
    """Number of arguments a printf format consumes: each conversion
    (flags, width, precision and length skipped; "%%" consumes none)."""
    a, n = fmt & 0x1FFFFF, 0
    while ram[a]:
        if ram[a] == 0x25:
            a += 1
            if ram[a] == 0x25:
                a += 1
                continue
            while ram[a] and chr(ram[a]) in '-+ #0123456789.hlL':
                a += 1
            if not ram[a]:
                break
            n += 1
        a += 1
    return n


CALL_CAP = [8192]   # callee stops per case before a poll loop counts as spun (cdgen36 lowers it)


def run_with_callees(ram, entry, args, rets, callees=CALLEES):
    """Run entry; stop at callee entries, record and return scripted v0."""
    pl.LOADS.clear()
    pl.STORES.clear()
    pl.CUR[0] = entry
    calls = []
    regs = {i: a for i, a in enumerate(args[:4], 4)}
    # o32: incoming arguments beyond a3 occupy sp+0x10 onward.
    # Write the fixture directly so this is not logged as a target store.
    for i, a in enumerate(args[4:]):
        struct.pack_into('<I', ram, 0x1FF010 + 4 * i, a & 0xFFFFFFFF)
    regs[31] = v.RET
    pc = entry
    for _ in range(CALL_CAP[0]):
        st = Stop((v.RET,) + tuple(callees))
        r = v.execute(ram, pc, (), stop_at=st, initial_regs=regs, instruction_budget=100000)
        if st.hit not in callees:
            return r[2] & 0xFFFFFFFF, list(pl.LOADS), list(pl.STORES), calls
        rec = [st.hit] + [x & 0xFFFFFFFF for x in r[4:8]]
        for k in range(CALLEE_STACK_ARGS.get(st.hit, 0)):   # arguments 5.. are in the outgoing area at sp+0x10
            rec.append(struct.unpack_from('<I', ram, (r[29] + 0x10 + 4 * k) & 0x1FFFFF)[0])
        if st.hit == PRINTF:            # record the format and only the arguments it consumes
            n = _printf_arity(ram, r[4])
            words = [x & 0xFFFFFFFF for x in r[5:8]] + \
                [struct.unpack_from('<I', ram, (r[29] + 0x10 + 4 * k) & 0x1FFFFF)[0] for k in range(max(0, n - 3))]
            rec = [st.hit, r[4] & 0xFFFFFFFF] + words[:n]
        n = callees[st.hit]
        if callable(n):                 # modelled callee: its return comes from guest RAM
            calls.append((rec, None, len(pl.STORES)))
            r[2] = n(ram, r) & 0xFFFFFFFF
            regs = dict(enumerate(r))
            pc = r[31]
            continue
        blob = bytes(ram[(r[4] & 0x1FFFFF):(r[4] & 0x1FFFFF) + n]).hex().upper() if n else None
        calls.append((rec, blob, len(pl.STORES)))
        r[2] = rets.get(st.hit, 0)
        regs = dict(enumerate(r))
        pc = r[31]
    raise AssertionError('instruction budget (callees)')


def modetype_cases(base):
    out = []
    for seed in range(1, 8001):
        s = seed * 7919
        ram, x, ring, size = prep(base, s)
        if seed > 6000:                  # discriminators: synthetic preset table (retail mask words are all 0)
            for a in range(0x9C8C0, 0x9C8C0 + 10 * 0x44, 4):
                x = gv.xs32(x)
                struct.pack_into('<I', ram, a, x | 0x01010101)
        x = gv.xs32(x)
        mode = (x >> 8) % 13
        mode = mode | 0x100 if x & 2 else mode
        if x % 7 == 0:
            mode = gv.xs32(x)
        for a in range(0x9B390, 0x9B3F0, 4):        # reverb state words (not the SPU base pointer at 0x9B3FC)
            x = gv.xs32(x)
            struct.pack_into('<I', ram, a, x if x & 1 else x & 0xFF)
        x = gv.xs32(x)
        rets = {0x80085BB4: 1 if x % 4 == 0 else 0}
        UNFAITHFUL[0] = False
        res = run_with_callees(ram, 0x8008CF70, [mode & 0xFFFFFFFF], rets)
        if UNFAITHFUL[0]:
            continue
        ret, loads, stores, calls = res
        call_text = ' '.join(','.join('%08X' % y for y in rec) + (',' + blob if blob else '') + '@%d' % n
                             for rec, blob, n in calls) or '-'
        out.append(gv.line(0x8008CF70, s, 0, [mode & 0xFFFFFFFF], (ret, loads, stores)) +
                   ' RETS %X CALL %s' % (rets[0x80085BB4], call_text))
    return out


def spureverb_cases(base):
    out = []
    for seed in range(1, 6001):
        s = seed * 7919
        ram, x, ring, size = prep(base, s)
        x = gv.xs32(x)
        mode = (0, 1, 1, 0, 2, 0xFFFFFFFF, 0x100, 0x101, 0x80000001, gv.xs32(x))[(x >> 4) % 10]
        for a in range(0x9B390, 0x9B3F0, 4):        # reverb state words (not the SPU base pointer at 0x9B3FC)
            x = gv.xs32(x)
            struct.pack_into('<I', ram, a, x if x & 1 else x & 0xFF)
        x = gv.xs32(x)
        rets = {0x80085BB4: (0, 1, 0, 0xFFFFFFFF)[x % 4]}
        UNFAITHFUL[0] = False
        res = run_with_callees(ram, 0x80085A64, [mode], rets, {0x80085BB4: None})
        if UNFAITHFUL[0]:
            continue
        ret, loads, stores, calls = res
        call_text = ' '.join(','.join('%08X' % y for y in rec) + '@%d' % n
                             for rec, blob, n in calls) or '-'
        out.append(gv.line(0x80085A64, s, 0, [mode], (ret, loads, stores)) +
                   ' RETS %X CALL %s' % (rets[0x80085BB4], call_text))
    return out


def fidma_cases(base):
    out = []
    callees = {0x8007DCAC: None, 0x80073A34: None, SENTINELS[0]: None, SENTINELS[1]: None}
    for seed in range(1, 6401):
        s = seed * 7919
        ram, x, ring, size = prep(base, s)
        SPU_STICKY[0] = (0x10, 0x20, 0x30)[seed % 3] if seed > 6000 else 0
        for a in range(0x9B390, 0x9B460, 4):        # libspu state words (not the SPU base pointer at 0x9B3FC)
            if a == 0x9B3FC:
                continue
            x = gv.xs32(x)
            struct.pack_into('<I', ram, a, x if x & 1 else x & 0xFF)
        x = gv.xs32(x)
        struct.pack_into('<I', ram, 0x9B434, (0, SENTINELS[0], SENTINELS[1])[x % 3])
        x = gv.xs32(x)
        struct.pack_into('<I', ram, 0x9B44C, (0, 0, 1, 2, 0xFFFFFFFF, gv.xs32(x))[x % 6])
        x = gv.xs32(x)
        cnt = (x >> 8) & 0xFFFF
        struct.pack_into('<H', ram, 0x1DAA, (cnt & ~0x30) | (0, 0x10, 0x20, 0x30)[x % 4] if x & 4 else cnt)
        UNFAITHFUL[0] = False
        res = run_with_callees(ram, 0x8007D614, [], {}, callees)
        SPU_STICKY[0] = 0
        if UNFAITHFUL[0]:
            continue
        ret, loads, stores, calls = res
        call_text = ' '.join(','.join('%08X' % y for y in rec) + '@%d' % n
                             for rec, blob, n in calls) or '-'
        out.append(gv.line(0x8007D614, s, 0, [], (ret, loads, stores)) + ' RETS 0 CALL %s' % call_text)
    return out


def cardclear_cases(base):
    out = []
    callees = {0x8007DDC4: None, 0x8007DDB4: None}
    edges = (0, 1, 2, 0x10, 0x11, 0x20, 0x30, 0xFF, 0x7FFFFFFF, 0x80000000, 0xFFFFFFFF)
    for seed in range(1, 4001):
        s = seed * 7919
        ram, x, ring, size = prep(base, s)
        x = gv.xs32(x)
        chan = (0, 0x10, 0x00, 0x10, edges[(x >> 8) % len(edges)], gv.xs32(x))[x % 6]
        x = gv.xs32(x)
        rets = {0x8007DDB4: (0, 1, 0xFFFFFFFF, x)[(x >> 3) % 4]}
        UNFAITHFUL[0] = False
        res = run_with_callees(ram, 0x8007DD74, [chan & 0xFFFFFFFF], rets, callees)
        if UNFAITHFUL[0]:
            continue
        ret, loads, stores, calls = res
        call_text = ' '.join(','.join('%08X' % y for y in rec) + (',' + blob if blob else '') + '@%d' % n
                             for rec, blob, n in calls) or '-'
        out.append(gv.line(0x8007DD74, s, 0, [chan & 0xFFFFFFFF], (ret, loads, stores)) +
                   ' RETS %X CALL %s' % (rets[0x8007DDB4], call_text))
    return out


CMP_ASC, CMP_DESC = 0x80130900, 0x80130940      # qsort comparator sentinels


def _cmp_bytes(sign):
    def cmp(ram, r):
        a, b = r[4] & 0x1FFFFF, r[5] & 0x1FFFFF
        # logged as loads so a checker seeds the bytes the comparator saw
        pl.LOADS.append((0x80000000 + a, 1, ram[a]))
        pl.LOADS.append((0x80000000 + b, 1, ram[b]))
        return sign * (ram[a] - ram[b])
    return cmp


def qsort_cases(base):
    out = []
    callees = {CMP_ASC: _cmp_bytes(1), CMP_DESC: _cmp_bytes(-1)}
    for seed in range(1, 3001):
        s = seed * 7919
        ram, x, ring, size = prep(base, s)
        x = gv.xs32(x)
        n = (0, 1, 2, 3, 5, 8, 13, 24, 40)[x % 9] if x & 0x100 else x % 41
        x = gv.xs32(x)
        width = (1, 2, 3, 4, 4, 4, 7, 8, 12, 16)[x % 10]
        x = gv.xs32(x)
        arr = 0x100000 + (x % 0x3000) * 4
        alpha = (2, 4, 8, 256)[(x >> 16) % 4]   # small alphabets make duplicates
        for i in range(n * width):
            x = gv.xs32(x)
            ram[arr + i] = (x >> 8) % alpha if i % width == 0 else (x >> 8) & 0xFF
        x = gv.xs32(x)
        cmpfn = CMP_ASC if x & 1 else CMP_DESC
        args = [0x80000000 | arr, n, width, cmpfn]
        UNFAITHFUL[0] = False
        res = run_with_callees(ram, 0x800723A4, args, {}, callees)
        if UNFAITHFUL[0]:
            continue
        ret, loads, stores, calls = res
        call_text = ' '.join(','.join('%08X' % y for y in rec) + '@%d' % m
                             for rec, blob, m in calls) or '-'
        out.append(gv.line(0x800723A4, s, 0, args, (ret, loads, stores)) + ' RETS 0 CALL %s' % call_text)
    return out


CDGEN = {   # entry: (argument kinds, callees) -- generated libcd/libds TUs
    0x800813E8: ('', (0x8007C564,)),
    0x800822BC: ('', (0x800824C8, 0x800824DC)),
    0x80082400: ('b', (0x80081D74,)),
    0x80080F64: ('w', (0x80081D74,)),
    0x8007FB04: ('', (0x8007B9EC, 0x80073D58)),
    0x8007BBB0: ('', (0x80073C94, 0x80073CC4)),
    0x8007C2A0: ('p', (0x8007AA34, 0x8007A930)),
}
CDGEN_SEED = {0x8007C2A0: (0x800A8020, 0x800A3494), 0x800822BC: (0x8009B70C, 0x8009B700, 0x8009B704)}   # words each entry reads
CDGEN_SEED.update({0x800808BC: (0x8009B574, 0x8009B578), 0x80081D74: (0x8009B70C,), 0x80081DF8: (0x8009B70C, 0x8009B700, 0x8009B704)})
CDGEN_SEED.update({0x80081268: (0x8009B6CC, 0x8009B6B8, 0x8009B6C0)})
CDGEN_SEED.update({0x8007FB44: (0x8009B598, 0x8009B554, 0x8009B574), 0x8007FCBC: (0x8009B598,), 0x8007F608: (0x800A3530, 0x800A3520, 0x800A3534, 0x800A3524)})
CDGEN_SPUN = {}       # entry -> cases that exceeded the instruction budget


CDGEN2 = {  # second batch (cdgen2): separate table so cdgen's output is unchanged
    0x800808BC: ('', (0x8007B9EC,)),
    0x80081D74: ('ww', (0x800824C8, 0x800824DC)),
    0x80081DF8: ('', (0x800824C8, 0x800824DC, 0x8007EE84)),
    0x800819D8: ('wp', (0x80072A54,)),
}


CDGEN3 = {  # third batch (cdgen3): DsFlush and its caller 0x80081268 (separate native files)
    0x8007F69C: ('', (0x800808BC, 0x80081DF8, 0x80080930)),
    0x80081268: ('', (0x8007F69C, 0x800822BC, 0x800824F0, 0x8007FCBC, 0x8007EE84)),
}


_FUNC_STARTS = []


def _func_end(entry):
    """Next function start after entry (configs/USA/psyq_classification.tsv
    plus disc1.yaml span starts), i.e. the true end of entry's body."""
    if not _FUNC_STARTS:
        starts = set()
        for line in open(v.ROOT / 'configs/USA/psyq_classification.tsv'):
            f = line.split('\t')
            if f and f[0].startswith('0x'):
                starts.add(int(f[0], 16))
        for line in open(v.ROOT / 'configs/USA/disc1.yaml'):
            m = re.match(r'\s*- \[0x([0-9A-Fa-f]+),', line)
            if m:
                starts.add(0x80010000 + int(m.group(1), 16) - 0x800)
        _FUNC_STARTS.extend(sorted(starts))
    return next((a for a in _FUNC_STARTS if a > entry), entry + 0x1000)


def _check_callees_complete(base, entry, callees, inline=()):
    """A callee missing from the table runs inline as original code and the
    vectors capture its body (text_490 / DsPacket, 2026-10-31).  Scan the
    whole body [entry, next function) and require every jal target, and
    every j target outside the body (tail call), to be listed or in
    `inline` (deliberately run as original code)."""
    end, targets = _func_end(entry), set()
    for a in range(entry, end, 4):
        w = struct.unpack_from('<I', base, a & 0x1FFFFF)[0]
        t = (a & 0xF0000000) | ((w & 0x3FFFFFF) << 2)
        if w >> 26 == 3 or (w >> 26 == 2 and not entry <= t < end):
            targets.add(t)
    missing = sorted(targets - set(callees) - set(inline))
    if missing:
        raise SystemExit('cdgen: entry %08X calls unstopped %s' % (entry, ' '.join('%08X' % m for m in missing)))


CDGEN_PTR_RET = {0x8007FC28}   # cdgen5: CdRom_GetCurrentPos returns a pointer
CDGEN5 = {  # fifth batch (cdgen5): unreached small libcd/libds/libgpu rows (frontier-20261031)
    0x8007FB44: ('ww', (0x8007FCFC,)),
    0x8007FCBC: ('ww', (0x8007FCFC,)),
    0x80080CDC: ('p', (0x8007FC28,)),
    0x8008227C: ('b', (0x800824C8,)),
    0x80082204: ('', (0x800824C8, 0x8007FC28, 0x80080C48, 0x8007FC18, 0x8007FC44, 0x8007F0C8)),
    0x8007F608: ('w', (0x8007FC88, 0x80080998)),
    0x80077548: ('w', ()),
    0x80077A00: ('', (0x80073CF4,)),
    0x80076C10: ('www', (0x80076C34,)),
}


CDGEN6 = {  # sixth batch (cdgen6): text_130, which passes pointers into its own stack frame
    0x80082444: ('b', (0x80080AE4, 0x80071A04, 0x80081DF8)),
}


CD_CB_SENT = 0x80130980        # cdgen7: a CD data callback (dispatched by the host's PE_Cd_DispatchDataCallback)
CDGEN7 = {  # seventh batch (cdgen7): callers that jump through a CD data-callback word
    0x8007F960: ('ww', (CD_CB_SENT,)),
}
CDGEN_CB_WORD = {0x8007F960: 0xB8AB8}   # callback word seeded with 0 or the sentinel


CARD_CB_SENT = 0x801309C0      # cdgen8: a libcard slot callback (host dispatcher PE_Card_ICall)
CDGEN8 = {  # eighth batch (cdgen8): libcard callers that jump through the D_8009B724.. slot table
    0x800840DC: ('p', (CARD_CB_SENT, 0x800832B4)),
}
CDGEN_CB_FIXED = {0x800840DC: (0x9B72C,)}   # slot words always hold the sentinel

CDGEN9 = {  # ninth batch (cdgen9): ResetGraph core text_2700 0x80077144 (generated TU)
    0x80077144: ('w', (0x80073E10, 0x80077A28, 0x80077548)),
}
CDGEN_RET_MASK = {0x80073E10: 0xFFFF}   # SetIntrMask returns the old 16-bit mask (port: uint16_t)
CDGEN10 = {  # tenth batch (cdgen10): DS_status 0x800809E0 (debug printout of the libds state)
    0x800809E0: ('', (PRINTF, 0x800822AC)),
}
CDGEN_HIGH_RETS = {0x800822AC, 0x8007DEB0, 0x8007FB44}   # results tested for zero: some draws set only bits 16..31 (0x8007FB44: cdgen30 v2)
DS_CB2 = 0x801309E0            # cdgen11: a libds callback called with two arguments (port: PE_GuestCall)
DS_CB3 = 0x80130A00            # cdgen11: a libds callback called with three arguments
CDGEN11 = {  # eleventh batch (cdgen11): libds callers that jump through a callback word
    0x80080F98: ('ww', (0x80073A44, 0x80080B04, 0x80081268, 0x80080AE4, DS_CB2)),
    0x8008214C: ('bw', (0x8007F778, 0x80082204, 0x800824C8, 0x800824DC, DS_CB3)),
}
CDGEN_CB_WORDS = {0x80080F98: (0x9B6D0, DS_CB2), 0x8008214C: (0x9B6F4, DS_CB3)}   # word: 0 or the sentinel
GPU_LOG_SENT = 0x80130A40      # cdgen12: the libgpu log vector (four arguments; port: PE_GuestCall)
CDGEN12 = {  # twelfth batch (cdgen12): SetGraphDebug 0x80074BB8
    0x80074BB8: ('b', (GPU_LOG_SENT,)),
}
CDGEN_CB_WORDS[0x80074BB8] = (0x95748, GPU_LOG_SENT)
CDGEN_CB_NONNULL = {0x80074BB8}   # called without a null check: a 0 word would jump to address 0
CDGEN16 = {  # sixteenth batch (cdgen16): libds text_904 0x80081D18 (CdIntToPos, read, poll)
    0x80081D18: ('www', (0x80080B44, 0x80080E34, 0x800811E4)),
}
SEQ_POS = (1, 0x7FFFFFFF, 0x100)            # cdgen16 poll model: positive results ...
SEQ_FINAL = (0, 0xFFFFFFFF, 0x80000000)     # ... then the final one


def _poll_model(k, pos, final):
    """A poll callee that returns `pos` for its first k calls, then `final`."""
    n = [0]

    def f(ram, r):
        n[0] += 1
        return pos if n[0] <= k else final
    return f


CDGEN15 = {  # fifteenth batch (cdgen15): small hand-port leaves
    0x800719C4: ('p', ()),            # libgpu: flag-tested record field address
    0x8007E4E0: ('', ()),             # libcard _copy_memcard_patch: a 28-word copy into low RAM
}
CARD_CB0 = 0x80130A80          # cdgen14: a libcard slot called with no arguments (port: PE_GuestCall)
CARD_CB1 = 0x80130AC0          # cdgen14: a libcard slot called with one argument
CDGEN14 = {  # fourteenth batch (cdgen14): libcard / libpad slot callers
    0x80082B08: ('', (CARD_CB0,)),
    0x80083DF0: ('p', (CARD_CB1,)),
}
CDGEN_CB_WORDS[0x80082B08] = (0x9B74C, CARD_CB0)
CDGEN_CB_WORDS[0x80083DF0] = (0x9B728, CARD_CB1)
CDGEN_CB_NONNULL.add(0x80083DF0)   # called without a null check
CDGEN18 = {  # eighteenth batch (cdgen18): libpad text_34 0x80084B78 (card-object state dispatch)
    0x80084B78: ('p', (0x80083E50, 0x800835C0, CARD_CB1)),
}
CDGEN17 = {  # seventeenth batch (cdgen17): libcard _dirFailAuto 0x80084EB0 (retry state, record-reset slot)
    0x80084EB0: ('p', (CARD_CB1,)),
}
CDGEN_CB_WORDS[0x80084EB0] = (0x9B728, CARD_CB1)   # cdgen17: the same record-reset slot
CDGEN_CB_NONNULL.add(0x80084EB0)
CDGEN13 = {  # thirteenth batch (cdgen13): libcard InitCARD / StartCARD (call sequences)
    0x8007DDD4: ('w', (0x80073C74, 0x80072714, 0x8007DEB0, 0x8007E314, 0x8007E4E0, 0x8007E3DC,
                       0x8007E470, 0x80072724)),
    0x8007DE40: ('', (0x80072714, 0x8007E324, 0x80073C74, 0x80072724)),
}


FF_HOOK = 0x80130B00           # cdgen19: the device's saved firstfile function, called with three arguments (port: PE_GuestCall)
CDGEN19 = {  # nineteenth batch (cdgen19): libapi firstfile 0x800727B4 and its device hook text_19C 0x80072950
    0x800727B4: ('pw', (0x80072A54, 0x80072A64)),
    0x80072950: ('pww', (0x80072A54, FF_HOOK)),
}
CDGEN_CB_WORDS[0x80072950] = (0xA32D0, FF_HOOK)   # the saved function word (no null check)
CDGEN_CB_NONNULL.add(0x80072950)
LOWRAM = (0x150, 0x158)        # cdgen19: BIOS device-table pointer 0x150 and size 0x154 (kernel RAM)
LOWRAM_ON = [False]
SIO_ACK = [None, 0]          # cdgen21: SIO_STAT (0x1F801044) bit 7 read-count model; None = off
REGBITS = {}                 # cdgen22: (addr, mask) -> [size, unused, k, reads]; empty = off
REGCOUNT = {}                # cdgen22: addr -> [step, value]; empty = off
_pre19_load = pl._ns['LOAD']   # the loader installed so far (_sio_load); cdgen19 wraps it


def _lowram_load(fmt, buf, a):
    """cdgen19 only: the interpreter maps kernel RAM 0x150.. and the
    scratchpad 0x1F800150.. to the same offset, and logs offsets below
    0x2000 as scratchpad.  These entries read the device-table words with
    `lw rt,0x150($zero)`, so log them as RAM (0x80000150..)."""
    n = struct.calcsize(fmt)
    for key, m in REGBITS.items():   # cdgen22: per-bit read-count models
        if key[0] == a and m[0] == n:
            m[3] += 1
            w = struct.unpack_from(fmt, buf, a)[0]
            on = m[3] > m[2]            # the bit becomes set after k reads
            struct.pack_into(fmt, buf, a, (w | key[1]) if on else (w & ~key[1]))
    if a in REGCOUNT and n == 2:     # cdgen22: a counter that advances on every read
        c = REGCOUNT[a]
        c[1] = (c[1] + c[0]) & 0xFFFF
        struct.pack_into('<H', buf, a, c[1])
    if SIO_ACK[0] is not None and a == 0x1044 and struct.calcsize(fmt) == 2:
        SIO_ACK[1] += 1                  # cdgen21: SIO_STAT bit 7 set for the first k reads, then clear
        h = struct.unpack_from('<H', buf, 0x1044)[0]
        struct.pack_into('<H', buf, 0x1044, (h | 0x80) if SIO_ACK[1] <= SIO_ACK[0] else (h & ~0x80))
    if LOWRAM_ON[0] and LOWRAM[0] <= a < LOWRAM[1]:
        val = struct.unpack_from(fmt, buf, a)
        pl.LOADS.append((0x80000000 + a, struct.calcsize(fmt), val[0] & 0xFFFFFFFF))
        return val
    return _pre19_load(fmt, buf, a)


pl._ns['LOAD'] = _lowram_load


def _ff_strcmp_model(match):
    """cdgen19 strcmp 0x80072A54: 0 when the first argument (an entry's
    name pointer) equals `match`, else 1 or -1 (by bit 2 of the pointer).
    The callers only test == 0; the checker's stub uses the same rule with
    `match` from RETS."""
    def f(ram, r):
        a = r[4] & 0xFFFFFFFF
        return 0 if a == match else (0xFFFFFFFF if a & 4 else 1)
    return f


def _ff_seed(ram, x, entry, args):
    """cdgen19: the device table, its low-RAM pointer and size, the name
    pointers (a pool of four, so entries can share one), and for firstfile
    a device-name string.  Returns (x, pool)."""
    pool = [ZONE + 0x1800 + 0x40 * k for k in range(4)]
    for p in pool:                   # name strings (contents are not read by the callers)
        ram[p & 0x1FFFFF] = 0x61
    x = gv.xs32(x)
    k = x % 16
    count = (0, 1, 2, 3, 4, 5, 8, 12)[k % 8] if k < 13 else (x >> 8) % 13
    base = ZONE + 0x800 + 4 * ((x >> 4) % 0x40)
    x = gv.xs32(x)
    r = x % 10
    size = count * 80 + ((0, 0, 0, 0, 0, 1, 79, 40, 0, 0)[r])
    top = r == 8 and x & 0x10000
    if top:                          # a table just below 2^32 (masked to 0x1FFxxx, above the stack):
        count = 1 + (x >> 8) % 5     # base + size may wrap while base + count * 80 does not
        base = (1 << 32) - count * 80 - (4, 8, 40, 76, 80, 160)[(x >> 12) % 6]
        size = count * 80 + (4, 40, 79)[(x >> 16) % 3]
    elif r == 8:                     # a base whose end wraps past 2^32: no entries are visited
        base, size = 0xFFFFFF80 - 4 * ((x >> 8) % 8), 80 * (2 + (x >> 12) % 4)
    elif r == 9 and count == 0:      # base 0 with an empty table
        base = 0
    struct.pack_into('<I', ram, 0x150, base)
    struct.pack_into('<I', ram, 0x154, size)
    if ZONE <= base < ZONE + 0x1000 or top:
        for i in range(count + 1 if top else 13):
            if top and base + 80 * i + 0x38 > 1 << 32:
                break
            e = (base + 80 * i) & 0x1FFFFF
            x = gv.xs32(x)
            struct.pack_into('<I', ram, e, 0 if x % 5 == 0 else pool[(x >> 4) % 4])
            x = gv.xs32(x)
            struct.pack_into('<I', ram, e + 0x34, (0, 0x80072950, x)[x % 3])
    if entry == 0x800727B4:          # device name: copied while the signed byte is >= ';' (59)
        name = ZONE + 0x1C00 + (x >> 20) % 0x40
        args[0] = name
        x = gv.xs32(x)
        n = (0, 1, 2, 3, 4, 8, 15, 20)[x % 8]
        chars = []
        for i in range(n):
            x = gv.xs32(x)
            chars.append((0x62, 0x75, 0x41, 0x7A, 0x3B, 0x3C, 0x7F, 0x5F)[x % 8] if x & 0x30 else 0x3B + (x >> 8) % 0x45)
        x = gv.xs32(x)
        chars.append((0x3A, 0x00, 0x30, 0x39, 0x80, 0xFF, 0x3A, 0x3A)[x % 8])
        chars += [0x63, 0x64, 0x00]
        for i, c in enumerate(chars):
            ram[(name + i) & 0x1FFFFF] = c
    else:                            # text_19C: a0 points at a word that is set to 1 when 0
        x = gv.xs32(x)
        args[0] = ZONE + 0x1E00 + 4 * (x % 0x40)
        x = gv.xs32(x)
        struct.pack_into('<I', ram, args[0] & 0x1FFFFF, (0, 0, 1, 2, 0xFFFFFFFF, x)[x % 6])
    return x, pool


SLOT_3C, SLOT_54, SLOT_50 = 0x80130C00, 0x80130C40, 0x80130C80   # cdgen20: libcard slot words (port: PE_GuestCall)
CDGEN20 = {  # twentieth batch (cdgen20): libcard card-object methods (generated TUs)
    0x80083F04: ('p', (SLOT_3C, 0x800830DC)),
    0x80083F44: ('p', (SLOT_54, SLOT_50, SLOT_3C, 0x800832B4)),
    0x8008401C: ('p', (SLOT_3C, 0x800832B4)),
}
CDGEN20_SLOTS = ((0x9B73C, SLOT_3C), (0x9B754, SLOT_54), (0x9B750, SLOT_50))


def _card20_seed(ram, x, args):
    """cdgen20: slot words (sentinel or alias; called without a null check),
    the libcard state words 0x8009B760..0x8009B7A4 and the card object's
    +0x0C word and +0x36 byte."""
    for word, sent in CDGEN20_SLOTS:
        x = gv.xs32(x)
        struct.pack_into('<I', ram, word, sent if x % 3 else sent & 0x1FFFFFFF)
    x = gv.xs32(x)
    a = (0, 1, 2, 0xFFFFFFFF, x)[x % 5]
    x = gv.xs32(x)
    struct.pack_into('<I', ram, 0x9B764, a)
    struct.pack_into('<I', ram, 0x9B774, a if x & 1 else (a + 1 + (x >> 8) % 3) & 0xFFFFFFFF)
    for w, ch in ((0x9B760, (0, 0, 1, 0xFFFFFFFF)), (0x9B7A4, (0, 0, 1, 0x80000000)),
                  (0x9B770, (0, 1, 0x42, 0xFFFFFFFF)), (0x9B79C, (0, 1, 0x20))):
        x = gv.xs32(x)
        struct.pack_into('<I', ram, w, ch[x % len(ch)] if x & 0x100 else gv.xs32(x))
    x = gv.xs32(x)
    obj = ZONE + (x % 0x300) * 4
    args[0] = obj
    x = gv.xs32(x)
    struct.pack_into('<I', ram, (obj + 0xC) & 0x1FFFFF, ZONE + 0x1000 + (x % 0x200) * 4 if x & 1 else gv.xs32(x))
    x = gv.xs32(x)
    ram[(obj + 0x36) & 0x1FFFFF] = (0, 0, 1, 0x42, 0x7F, 0x80, 0xFF, x & 0xFF)[x % 8]
    return x


CDGEN21 = {  # twenty-first batch (cdgen21): libcard SIO wait 0x800834E8 (generated TU)
    0x800834E8: ('', (0x80084FE4,)),
}
WAIT_FINAL = (1, 0xFFFFFFFF, 0x200)   # cdgen21: 0x80084FE4 returns 0 for j calls, then one of these


def _sio21_seed(ram, x):
    """cdgen21: SIO_STAT bit 7 is set for the first k reads (0-7), then
    clear; SIO_STAT / SIO_CTRL / I_STAT seeded; in a quarter of the cases
    the I_STAT and SIO pointer words 0x8009B784 / 0x8009B788 point into the
    record zone instead (plain memory, no device model there)."""
    x = gv.xs32(x)
    SIO_ACK[0], SIO_ACK[1] = x % 8, 0
    for a in (0x1044, 0x104A, 0x1070):
        x = gv.xs32(x)
        struct.pack_into('<H', ram, a, (x >> 8) & 0xFFFF)
    x = gv.xs32(x)
    if x % 4 == 0:
        struct.pack_into('<I', ram, 0x9B784, ZONE + 0x100 + 4 * ((x >> 4) % 0x40))
        x = gv.xs32(x)
        sio = ZONE + 0x400 + 4 * ((x >> 4) % 0x40)
        struct.pack_into('<I', ram, 0x9B788, sio)
        for k in (4, 10):
            x = gv.xs32(x)
            struct.pack_into('<H', ram, (sio + k) & 0x1FFFFF, (x >> 8) & 0xFFFF)
    return x


CDGEN22 = {  # twenty-second batch (cdgen22): libcard SIO byte exchange 0x800830DC / write 0x800832B4 (hand ports)
    0x800830DC: ('pw', (0x80084FE4,)),
    0x800832B4: ('pw', (0x80084FC4, 0x80084FE4)),
}


def _sio22_seed(ram, x, entry, args):
    """cdgen22: the card object (+0x3C buffer pointer whose first byte's
    high nibble is 8 or not, +0x40 pointer, +0x44 index, +0x45 count, +0xE8
    byte), the state words 0x8009B768 / 0x800A76D0 / 0x800BD02C, SIO data,
    and the register models: SIO_STAT bits 0 and 1 and I_STAT bit 7 each
    set after k reads; the RCNT2 counter advances by a seeded step on every
    read (mode and target seeded)."""
    x = gv.xs32(x)
    obj = ZONE + (x % 0x200) * 4
    args[0] = obj
    x = gv.xs32(x)
    if entry == 0x800830DC:          # a1: negative (the send path) or not
        args[1] = (0xFFFFFFFF, 0xFFFFFF00, 0x80000000, 0xFFFFFFFE, 0, 1, 0x42, 0x7FFFFFFF, 0xFF, x)[x % 10]
    else:
        args[1] = (0, 1, 0x42, 0x80, 0xFF, x)[x % 6]
    buf = ZONE + 0x1000 + (x >> 8) % 0x100 * 4
    struct.pack_into('<I', ram, (obj + 0x3C) & 0x1FFFFF, buf)
    x = gv.xs32(x)
    ram[buf & 0x1FFFFF] = ((0x80 | (x & 0xF)) if x & 0x30 else (x >> 8) & 0xFF)
    x = gv.xs32(x)
    struct.pack_into('<I', ram, (obj + 0x40) & 0x1FFFFF, ZONE + 0x1800 + (x % 0x100) * 4)
    for k, ch in ((0x44, (0, 1, 7, 8, 9, 10, 0xFE, 0xFF)), (0x45, (0, 1, 0xFF)), (0xE8, (8, 8, 0, 7, 9))):
        x = gv.xs32(x)
        ram[(obj + k) & 0x1FFFFF] = ch[x % len(ch)] if x & 0x100 else (x >> 16) & 0xFF
    for w, ch in ((0x9B768, (2, 2, 0, 1, 3, 0x10002, 0x102, 0xFFFF0002)),
                  (0xA76D0, (0, 0x100, 0xFFF0, 0xFFFF, 0x10000, 0x1FFF0, 0xFFFFFFFF)),
                  (0xBD02C, (0, 1, 0x10, 0x1AE, 0x40, 0x10000, 0x10010, 0xFFFFFFFF))):
        x = gv.xs32(x)                   # words with high bits set decide a word vs halfword compare
        struct.pack_into('<I', ram, w, ch[x % len(ch)] if x & 0x100 else (x >> 16) & 0xFFFF)
    for a in (0x1040, 0x1044, 0x104A, 0x104E, 0x1070, 0x1124, 0x1128):
        x = gv.xs32(x)
        struct.pack_into('<H', ram, a, (x >> 8) & 0xFFFF)
    x = gv.xs32(x)
    if x % 4 == 0:                   # the RCNT2 target 0 (a wrap adds 0x10000 then)
        struct.pack_into('<H', ram, 0x1128, 0)
    REGBITS.clear()
    REGCOUNT.clear()
    for key, size in (((0x1044, 1), 2), ((0x1044, 2), 2), ((0x1070, 0x80), 4)):
        x = gv.xs32(x)
        REGBITS[key] = [size, 0, x % 6, 0]
    x = gv.xs32(x)
    REGCOUNT[0x1120] = [(1, 8, 0x40, 0x100, 0x1000)[x % 5], (x >> 8) & 0xFFFF]
    x = gv.xs32(x)
    if entry == 0x800832B4 and x % 3 == 0:
        # Boundary cases: mode bit 9 set (elapsed not shifted), I_STAT bit 7
        # clear for the first 3 reads, and the limit word set to exactly the
        # elapsed value of the first loop pass, or one more, both for a
        # counter below the start (wrap: + target, or + 0x10000 when the
        # target is 0) and above it.
        step, init = REGCOUNT[0x1120]
        c1 = (init + step) & 0xFFFF
        REGBITS[(0x1070, 0x80)][2] = 3
        mode = struct.unpack_from('<H', ram, 0x1124)[0] | 0x200
        struct.pack_into('<H', ram, 0x1124, mode)
        x = gv.xs32(x)
        if x & 1 and c1 < 0xFFFF:        # wrap: start above the counter
            start = c1 + 1 + (x >> 8) % (0xFFFF - c1)
            x = gv.xs32(x)
            tgt = 0 if x & 1 else 1 + (x >> 8) % 0xFFFF
            struct.pack_into('<H', ram, 0x1128, tgt)
            elapsed = (c1 + (tgt if tgt else 0x10000) - start) & 0xFFFFFFFF
        else:
            start = (x >> 8) % (c1 + 1)
            elapsed = c1 - start
        x = gv.xs32(x)
        struct.pack_into('<I', ram, 0xA76D0, start)
        struct.pack_into('<I', ram, 0xBD02C, (elapsed + (x % 3) - 1) & 0xFFFFFFFF)   # elapsed - 1, elapsed, elapsed + 1
    return x


# Twenty-third batch (cdgen23): libpad padif transfer 0x80084168 (hand port).
SLOT23_30, SLOT23_44, SLOT23_48, SLOT23_24 = 0x80130D00, 0x80130D40, 0x80130D80, 0x80130DC0
CDGEN23_SLOTS = ((0x9B730, SLOT23_30), (0x9B72C, CARD_CB_SENT), (0x9B744, SLOT23_44),
                 (0x9B748, SLOT23_48), (0x9B724, SLOT23_24))
CDGEN23 = {  # slot words go through PE_Card_ICall (0x9B72C with two arguments, the others with one)
    0x80084168: ('p', (SLOT23_30, CARD_CB_SENT, 0x800832B4, 0x80084FC4, 0x800834E8,
                       SLOT23_44, SLOT23_48, 0x800830DC, 0x80083578, SLOT23_24)),
}
ACK23_POS = (1, 0xFFFFFFFF, 0x80000000, 0x100)
ACK23_CNT = (0, 1, 2, 3, 5, 0xFFFFFFFF, 0x80000000, 8)
ACK23_PAIR = (0xFFFFFFFE, 0xFFFFFFFF, 0, 1, 2, 3, 4, 5, 6, 0x7FFFFFFF, 0x80000000, 3, 4, 1, 0, 2)
XCH23_OK = (0, 0x42, 0xFF, 0x7FFFFFFF)
XCH23_FIN = (0xFFFFFFFF, 0xFFFFFFFE, 0xFFFFFFEC, 0x80000000)
XCH23_IDX = (0, 1, 7, 8, 0x7F, 0x80, 0xFE, 0xFF)


def _store23(ram, a, n, val):
    """A callee's own memory write: applied to RAM and logged as a store
    (the checker's stub repeats it through PE_Store*, so both sides agree)."""
    if n == 4:
        struct.pack_into('<I', ram, a, val & 0xFFFFFFFF)
    else:
        ram[a] = val & 0xFF
    pl.STORES.append((0x80000000 + a, n, val & ((1 << (8 * n)) - 1)))


def _ack23_model(code):
    """0x800834E8 (no argument): ACK23_POS[p] for the first k calls (k == 15:
    every call), then 0.  On call m (1..15; 0 = never) it also writes the
    counter 0x8009B79C = ACK23_CNT[cv], both pair words 0x8009B77C /
    0x8009B780 = ACK23_PAIR[pv], and with bit 20 rewrites every slot word
    to an absolute form: bit 21 + i set = the KUSEG alias, clear = the
    sentinel (i = index in CDGEN23_SLOTS).  Absolute, so a checker that
    seeds RAM from first loads still starts the write from the right value.
    code = k | p << 4 | m << 8 | cv << 12 | pv << 16 | slots << 20 |
    forms << 21."""
    k, p, m = code & 15, (code >> 4) & 3, (code >> 8) & 15
    n = [0]

    def f(ram, r):
        n[0] += 1
        if n[0] == m:
            _store23(ram, 0x9B79C, 4, ACK23_CNT[(code >> 12) & 7])
            pv = ACK23_PAIR[(code >> 16) & 15]
            _store23(ram, 0x9B77C, 4, pv)
            _store23(ram, 0x9B780, 4, pv)
            if code >> 20 & 1:
                for i, (word, sent) in enumerate(CDGEN23_SLOTS):
                    _store23(ram, word, 4, sent & 0x1FFFFFFF if code >> (21 + i) & 1 else sent)
        return ACK23_POS[p] if k == 15 or n[0] <= k else 0
    return f


def _xch23_model(code):
    """0x800832B4 / 0x800830DC (obj, byte): XCH23_OK[ok] for the first k calls
    (k == 15: every call), then XCH23_FIN[fin].  On call m (1..15; 0 = never)
    it writes the byte obj+0x44 = XCH23_IDX[bv] (obj = its $a0).
    code = k | ok << 4 | fin << 6 | m << 8 | bv << 12."""
    k, ok, fin, m = code & 15, (code >> 4) & 3, (code >> 6) & 3, (code >> 8) & 15
    n = [0]

    def f(ram, r):
        n[0] += 1
        if n[0] == m:
            _store23(ram, (r[4] + 0x44) & 0x1FFFFF, 1, XCH23_IDX[(code >> 12) & 7])
        return XCH23_OK[ok] if k == 15 or n[0] <= k else XCH23_FIN[fin]
    return f


def _blk23_model(code):
    """The one-argument slot 0x8009B744 (sentinel and alias share one
    counter): on its call m (1..15; 0 = never) it writes both pair words
    0x8009B77C / 0x8009B780 = ACK23_PAIR[pv] (v3 discriminator: the caller
    re-reads the pair word after this call).  code = m | pv << 4."""
    m, n = code & 15, [0]

    def f(ram, r):
        n[0] += 1
        if n[0] == m:
            pv = ACK23_PAIR[(code >> 4) & 15]
            _store23(ram, 0x9B77C, 4, pv)
            _store23(ram, 0x9B780, 4, pv)
        return 0
    return f


def _card23_seed(ram, x, args):
    """cdgen23: the card object (+0x0C base word, +0x36 byte, +0x3C buffer
    whose first byte's high nibble is 8 or not, +0x44 index), the slot words
    (sentinel or KUSEG alias; no null check), the state words 0x8009B764 /
    0x8009B770, the counter 0x8009B79C (dense small values), the pair words
    0x8009B77C / 0x8009B780 (the branch edges -2..6 and the word extremes),
    the record base 0x8009B758 (a zone pointer: base + {0, 0xF0} + 0xC is
    read) and the byte pointer 0x8009B7A0."""
    for word, sent in CDGEN23_SLOTS:
        x = gv.xs32(x)
        struct.pack_into('<I', ram, word, sent if x % 3 else sent & 0x1FFFFFFF)
    x = gv.xs32(x)
    obj = ZONE + (x % 0x200) * 4
    args[0] = obj
    x = gv.xs32(x)
    struct.pack_into('<I', ram, (obj + 0xC) & 0x1FFFFF, ZONE + 0x1000 + (x % 0x200) * 4 if x & 1 else gv.xs32(x))
    x = gv.xs32(x)
    buf = ZONE + 0x1800 + (x >> 8) % 0x100 * 4
    struct.pack_into('<I', ram, (obj + 0x3C) & 0x1FFFFF, buf)
    x = gv.xs32(x)
    ram[buf & 0x1FFFFF] = (0x80 | (x & 0xF)) if x & 0x30 else (x >> 8) & 0xFF
    for k, ch in ((0x36, (0, 0, 1, 0x80, 0xFF)), (0x44, (0, 1, 7, 8, 0xFE, 0xFF))):
        x = gv.xs32(x)
        ram[(obj + k) & 0x1FFFFF] = ch[x % len(ch)] if x & 0x100 else (x >> 16) & 0xFF
    for w, ch in ((0x9B770, (0, 1, 0xFFFFFFFF, 0x80000000)), (0x9B764, (0, 0, 1, 0xFFFFFFFF))):
        x = gv.xs32(x)
        struct.pack_into('<I', ram, w, ch[x % len(ch)] if x & 0x100 else gv.xs32(x))
    x = gv.xs32(x)
    cnt = (0xFFFFFFFF, 0, 1, 2, 3, 4, 5, 6, 7, 8, 10, 12, 0x80000000)
    struct.pack_into('<I', ram, 0x9B79C, cnt[x % len(cnt)] if x % 16 else 0x7FFFFFFF)
    for w in (0x9B77C, 0x9B780):
        x = gv.xs32(x)
        struct.pack_into('<I', ram, w, (0xFFFFFFFE, 0xFFFFFFFF, 0, 1, 2, 3, 4, 5, 6, 0x7FFFFFFF, 0x80000000)[x % 11]
                         if x & 0x700 else gv.xs32(x))
    x = gv.xs32(x)
    rec = ZONE + 0x2000 + (x % 0x100) * 4
    struct.pack_into('<I', ram, 0x9B758, rec)
    for k in (0xC, 0xFC):
        x = gv.xs32(x)
        struct.pack_into('<I', ram, (rec + k) & 0x1FFFFF, ZONE + 0x3000 + (x % 0x100) * 4 if x & 1 else gv.xs32(x))
    x = gv.xs32(x)
    bp = ZONE + 0x3800 + x % 0x400
    if x % 8 == 0:                   # v3: the byte pointer aliases the index byte obj+0x44
        bp = obj + 0x44
    struct.pack_into('<I', ram, 0x9B7A0, bp)
    x = gv.xs32(x)
    ram[bp & 0x1FFFFF] = x & 0xFF
    return x


def _card23_models(x, rets, stops):
    """cdgen23: scripted call sequences (RETS carries each model's code).
    A large counter (0x7FFFFFFF) is only paired with a finite acknowledge
    sequence, so the case ends."""
    big = False
    for c in (0x800832B4, 0x800830DC):
        x = gv.xs32(x)
        code = x & 0x7FFF
        rets[c] = code
        stops[c] = _xch23_model(code)
    x = gv.xs32(x)
    code = x & 0x3FFFFFF
    if x % 3 == 0:
        code &= ~0x100F00        # no mutation, no flip: the plain sequences
    rets[0x800834E8] = code
    stops[0x800834E8] = _ack23_model(code)
    for word, sent in CDGEN23_SLOTS:  # the alias is stopped and returns the sentinel's value
        stops[sent & 0x1FFFFFFF] = None
        rets[sent & 0x1FFFFFFF] = rets[sent]
    x = gv.xs32(x)                    # v3: the 0x8009B744 slot writes the pair words on its call m
    code = (x & 0xFF) if x % 2 else 0
    blk = _blk23_model(code)
    rets[SLOT23_44] = rets[SLOT23_44 & 0x1FFFFFFF] = code
    stops[SLOT23_44] = stops[SLOT23_44 & 0x1FFFFFFF] = blk
    x = gv.xs32(x)
    rets[CARD_CB_SENT] = (0, 0x42, 0x1FF, 0xFFFFFF80, gv.xs32(x))[x % 5]   # passed on masked to a byte
    rets[CARD_CB_SENT & 0x1FFFFFFF] = rets[CARD_CB_SENT]
    return x


# Twenty-fourth batch (cdgen24): libpad text_770 0x80083014 (hand port), the card transfer step.
STEP24 = 0x80130E00            # table entry k (index k - 1) holds STEP24 + 0x40 * k, k = 0..10
CDGEN24 = {  # the step table 0x8009B7A4 + 4 * k and the error hook go through PE_Card_ICall with one argument
    0x80083014: ('p', (STEP24, SLOT23_24, 0x80084FC4, 0x800834E8)),
}
STEP24_RET = (0, 1, 0xFFFFFFFF, 0xFFFFFFFD, 0x7FFFFFFF, 0x80000000, 0x42, 0xFFFFFFEC)
STEP24_IDX = (0, 1, 2, 3, 4, 5, 6, 7, 8, 0xFFFFFFFF, 0x7FFFFFFF, 0x80000000, 5, 6, 0, 4)


def _step24_model(code):
    """A table entry (all entries and aliases share it): returns
    STEP24_RET[r]; with bit 3 it writes the index word 0x8009B768 =
    STEP24_IDX[v] (absolute).  code = r | mut << 3 | v << 4."""
    def f(ram, r):
        if code >> 3 & 1:
            _store23(ram, 0x9B768, 4, STEP24_IDX[(code >> 4) & 15])
        return STEP24_RET[code & 7]
    return f


def _hook24_model(code):
    """The error hook 0x8009B724: with bit 0 it writes the index word =
    STEP24_IDX[v] (absolute); returns 0.  code = mut | v << 1."""
    def f(ram, r):
        if code & 1:
            _store23(ram, 0x9B768, 4, STEP24_IDX[(code >> 1) & 15])
        return 0
    return f


def _card24_seed(ram, x, args):
    """cdgen24: the index word 0x8009B768 (-1..9, the word extremes whose
    4 * index wraps onto entries -1 / 0), every table word 0x8009B7A4 +
    4 * k (k = 0..10) as its own sentinel or the KUSEG alias, and the error
    hook word 0x8009B724 (sentinel or alias)."""
    x = gv.xs32(x)
    idx = (0xFFFFFFFF, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 4, 5, 6, 0x7FFFFFFF, 0x80000000)[x % 16]
    struct.pack_into('<I', ram, 0x9B768, idx)
    for k in range(11):
        x = gv.xs32(x)
        sent = STEP24 + 0x40 * k
        struct.pack_into('<I', ram, 0x9B7A4 + 4 * k, sent if x % 3 else sent & 0x1FFFFFFF)
    x = gv.xs32(x)
    struct.pack_into('<I', ram, 0x9B724, SLOT23_24 if x % 3 else SLOT23_24 & 0x1FFFFFFF)
    return x


def _card24_models(x, rets, stops):
    x = gv.xs32(x)
    code = x & 0xFF
    rets[STEP24] = code
    step = _step24_model(code)
    for k in range(11):
        sent = STEP24 + 0x40 * k
        stops[sent] = stops[sent & 0x1FFFFFFF] = step
    x = gv.xs32(x)
    code = x & 0x1F
    rets[SLOT23_24] = code
    stops[SLOT23_24] = stops[SLOT23_24 & 0x1FFFFFFF] = _hook24_model(code)
    return x


# Twenty-fifth batch (cdgen25): libcard MemCard_TimerCallback 0x80082B70 (hand port).
CDGEN25 = {  # 0x80082E00 stays a hand port (stopped); 0x80083014 is pe_plat_pad_transfer_step; the hook goes through PE_Card_ICall
    0x80082B70: ('', (0x80082E00, SLOT23_24, 0x80083014)),
}
SEQ25 = (0xFFFFFFFF, 0, 1, 2, 3, 4, 5, 6)
B25 = (0xFFFFFFFF, 0, 1, 2, 3, 5, 0x80000000, 4)
RET25 = (0, 1, 0xFFFFFFFF, 0x80000000, 0x42, 0x7FFFFFFF, 0x100, 0xFFFFFFFE)


def _xfer25_model(code):
    """0x80083014 (obj; void in the port), called in the caller's loop.  On
    its call n < m it writes 0x8009B764 = SEQ25[(base + n) % 8]; on call m
    (1..8) it writes 0x8009B764 = 0x7FFFFFFF, which ends the loop.  On call
    j (1..7; 0 = never) it also writes 0x8009B778 = B25[bv].  All absolute.
    code = (m - 1) | base << 3 | j << 6 | bv << 9."""
    m, base, j, bv = (code & 7) + 1, (code >> 3) & 7, (code >> 6) & 7, (code >> 9) & 7
    n = [0]

    def f(ram, r):
        n[0] += 1
        if n[0] == j:
            _store23(ram, 0x9B778, 4, B25[bv])
        _store23(ram, 0x9B764, 4, 0x7FFFFFFF if n[0] >= m else SEQ25[(base + n[0]) % 8])
        return 0
    return f


def _sector25_model(code):
    """0x80082E00 (record): returns RET25[r]; with bit 3 writes 0x8009B764 =
    SEQ25[v].  code = r | mut << 3 | v << 4."""
    def f(ram, r):
        if code >> 3 & 1:
            _store23(ram, 0x9B764, 4, SEQ25[(code >> 4) & 7])
        return RET25[code & 7]
    return f


def _hook25_model(code):
    """The error hook 0x8009B724: with bit 0 writes 0x8009B764 = SEQ25[v].
    code = mut | v << 1."""
    def f(ram, r):
        if code & 1:
            _store23(ram, 0x9B764, 4, SEQ25[(code >> 1) & 7])
        return 0
    return f


def _card25_seed(ram, x, args):
    """cdgen25: the record words 0x8009B774 / 0x8009B778 (small signed values,
    0x80000000; never 0x7FFFFFFF), the 150-capped counters 0x800A5AC0 /
    0x800A5AC4 (around 150 and the word extremes), the enable word
    0x8009B75C, the record base 0x8009B758 (a zone pointer or any word: it is
    only added to), the stored-only words 0x8009B764 / 68 / 6C / 8C, the SIO
    pointer 0x8009B788 (the SIO register block or a zone word) and the hook
    word 0x8009B724 (sentinel or alias)."""
    for w in (0x9B774, 0x9B778):
        x = gv.xs32(x)
        struct.pack_into('<I', ram, w, (0xFFFFFFFE, 0xFFFFFFFF, 0, 1, 2, 3, 4, 5, 6, 0x80000000)[x % 10])
    for w in (0xA5AC0, 0xA5AC4):
        x = gv.xs32(x)
        struct.pack_into('<I', ram, w, (0, 1, 148, 149, 150, 151, 0xFFFFFFFF, 0x80000000, 0x7FFFFFFF, x)[x % 10])
    x = gv.xs32(x)
    struct.pack_into('<I', ram, 0x9B75C, (0, 1, 1, 0xFFFFFFFF, x)[x % 5])
    x = gv.xs32(x)
    struct.pack_into('<I', ram, 0x9B758, ZONE + (x % 0x400) * 4 if x & 1 else gv.xs32(x))
    for w in (0x9B764, 0x9B768, 0x9B76C, 0x9B78C):
        x = gv.xs32(x)
        struct.pack_into('<I', ram, w, x)
    x = gv.xs32(x)
    struct.pack_into('<I', ram, 0x9B788, 0x1F801040 if x % 3 == 0 else ZONE + 0x2000 + (x % 0x100) * 4)
    x = gv.xs32(x)
    struct.pack_into('<I', ram, 0x9B724, SLOT23_24 if x % 3 else SLOT23_24 & 0x1FFFFFFF)
    return x


def _card25_models(x, rets, stops):
    x = gv.xs32(x)
    rets[0x80083014] = x & 0xFFF
    stops[0x80083014] = _xfer25_model(x & 0xFFF)
    x = gv.xs32(x)
    rets[0x80082E00] = x & 0x7F
    stops[0x80082E00] = _sector25_model(x & 0x7F)
    x = gv.xs32(x)
    rets[SLOT23_24] = rets[SLOT23_24 & 0x1FFFFFFF] = x & 0xF
    stops[SLOT23_24] = stops[SLOT23_24 & 0x1FFFFFFF] = _hook25_model(x & 0xF)
    return x


# Twenty-sixth batch (cdgen26): libpad text_108 0x80084C4C (hand port), the record step
# installed in slot 0x8009B744 ("card sector flush"); void in the port (no caller uses $v0).
RESET26 = 0x801310C0           # slot 0x8009B728, the record reset (PE_Card_ICall, one argument)
HOOK26 = 0x80131100            # the object's +0x18 hook (PE_GuestCall, one argument)
CDGEN26 = {
    0x80084C4C: ('p', (0x80084F8C, RESET26, HOOK26, 0x80083644)),
}
ST26 = (0, 1, 2, 3, 0x7F, 0x80, 0xFD, 0xFE, 0xFF, 4, 0x10, 0xFC, 2, 1, 0, 0xFF)
RET26 = (0, 1, 2, 0xFFFFFFFF, 0xFF, 0x100, 0x80000000, 0xFFFFFFFE)


def _state26_model(code, ret=True):
    """A callee of 0x80084C4C that may rewrite the state byte obj+0x46 (and,
    with bit 5, the byte obj+0x36) before returning RET26[r].  The writes are
    absolute.  code = r | mut << 3 | mut36 << 4 | v << 5 | b36 << 9."""
    def f(ram, r):
        obj = r[4] & 0x1FFFFF
        if code >> 3 & 1:
            _store23(ram, (obj + 0x46) & 0x1FFFFF, 1, ST26[(code >> 5) & 15])
        if code >> 4 & 1:
            _store23(ram, (obj + 0x36) & 0x1FFFFF, 1, (0, 1, 0x80, 0xFF)[(code >> 9) & 3])
        return RET26[code & 7] if ret else 0
    return f


def _card26_seed(ram, x, args):
    """cdgen26: the pad record (zone pointer): +0x3C source buffer (first byte
    with high nibble 0xF or not, incl. 0xF3; second byte 0 or not), +0x30
    destination buffer, +0x44 count, +0x46 state (every edge of its compare
    chain), +0xE8, +0x36, +0x37, +0x4A, +0x50 bytes, +0x14 word, the +0x18
    hook (0, the sentinel or its alias) and the reset slot 0x8009B728
    (sentinel or alias; no null check)."""
    x = gv.xs32(x)
    obj = ZONE + (x % 0x200) * 4
    args[0] = obj
    x = gv.xs32(x)
    src = ZONE + 0x1000 + (x % 0x100) * 4
    struct.pack_into('<I', ram, (obj + 0x3C) & 0x1FFFFF, src)
    x = gv.xs32(x)
    struct.pack_into('<I', ram, (obj + 0x30) & 0x1FFFFF, ZONE + 0x1800 + (x % 0x100) * 4)
    for k in range(0x24):
        x = gv.xs32(x)
        ram[(src + k) & 0x1FFFFF] = x & 0xFF
    x = gv.xs32(x)
    ram[src & 0x1FFFFF] = (0xF0 | (x & 0xF), 0xF3, 0xF3, (x >> 8) & 0xFF, 0x41, 0x73)[x % 6]
    x = gv.xs32(x)
    ram[(src + 1) & 0x1FFFFF] = 0 if x % 3 else (x >> 8) & 0xFF
    for k, ch in ((0x44, (0, 1, 2, 3, 4, 8, 0x22, 0xFF)), (0x46, ST26), (0xE8, (0, 0xF, 4, 7, 0xFF)),
                  (0x36, (0, 0, 1, 0xFF)), (0x37, (0, 0, 1, 0x80)), (0x4A, (0, 0, 1, 0xFF)), (0x50, (0, 0, 1, 0x80))):
        x = gv.xs32(x)
        ram[(obj + k) & 0x1FFFFF] = ch[x % len(ch)] if x & 0x100 else (x >> 16) & 0xFF
    x = gv.xs32(x)
    struct.pack_into('<I', ram, (obj + 0x14) & 0x1FFFFF, (0, 0, 1, x)[x % 4])
    x = gv.xs32(x)
    struct.pack_into('<I', ram, (obj + 0x18) & 0x1FFFFF, (0, HOOK26, HOOK26 & 0x1FFFFFFF)[x % 3])
    x = gv.xs32(x)
    struct.pack_into('<I', ram, 0x9B728, RESET26 if x % 3 else RESET26 & 0x1FFFFFFF)
    return x


def _card26_models(x, rets, stops):
    for c, ret in ((RESET26, False), (HOOK26, True), (0x80083644, True)):
        x = gv.xs32(x)
        code = x & 0x7FF
        rets[c] = code
        stops[c] = _state26_model(code, ret)
        if c & 0x80000000 and c != 0x80083644:
            rets[c & 0x1FFFFFFF] = code
            stops[c & 0x1FFFFFFF] = stops[c]
    return x


# Twenty-eighth batch (cdgen28): libds text_FC 0x80081E70 (hand port), the DS ready callback
# (installed through 0x800824C8 by 0x80081D74 / 0x80082204; void in the port).  cdgen27 is the
# parked 0x80081110 work (outside the tree).
CDGEN28 = {
    0x80081E70: ('bp', (0x8007FC28, 0x80080C48, 0x8007FC18, 0x800824F0, 0x80080AE4, DS_CB3,
                        0x800824C8, 0x800824DC, 0x8007F778, 0x80082204, 0x8007EE84)),
}
POS28_D = (0, 0, 1, 0xFFFFFFFF, 0x7FFFFFFF, 0x80000000, 2, 0)
BLK28 = (0, 1, 2, 0xFFFFFFFF, 0x80000000, 0x7FFFFFFF, 1, 0)
CNT28 = (0, 1, 2, 0xFFFFFFFF, 0xFFFFFFFE, 0xFFFFFFFD, 0xFFFFFF9C, 0x80000000, 0x80000001, 0x7FFFFFFF)   # v2: negatives other than -1


def _pos28_model(code):
    """0x80080C48 (CdPosToInt, pe_plat_cd_pos_to_int): call n returns the
    position word *0x8009B6EC as it is at that call, plus POS28_D[d_n]
    (d_1 = code & 7, d_2 = code >> 3 & 7), so the equality test against
    the reloaded word is exercised."""
    n = [0]

    def f(ram, r):
        n[0] += 1
        d = POS28_D[(code >> (3 * (n[0] - 1))) & 7] if n[0] <= 2 else 0
        return (struct.unpack_from('<I', ram, 0x9B6EC)[0] + d) & 0xFFFFFFFF
    return f


def _blk28_model(code, word):
    """A callee that may write `word` = BLK28[v] (absolute; bit 0)."""
    def f(ram, r):
        if code & 1:
            _store23(ram, word, 4, BLK28[(code >> 1) & 7])
        return 0
    return f


def _cb28_model(code):
    """The DS_CB3 callback: with bit 0 writes 0x8009B6F8 = BLK28[v]; with bit 4
    writes 0x8009B6FC = BLK28[w].  code = m1 | v << 1 | m2 << 4 | w << 5."""
    def f(ram, r):
        if code & 1:
            _store23(ram, 0x9B6F8, 4, BLK28[(code >> 1) & 7])
        if code >> 4 & 1:
            _store23(ram, 0x9B6FC, 4, BLK28[(code >> 5) & 7])
        return 0
    return f


def _ds28_seed(ram, x, args):
    """cdgen28: a0 the interrupt byte (1, 4 or another value), a1 a zone
    pointer to the result bytes (byte 0 bits 0x10 / 0x20 / 0x80 seeded); the
    ready-system block 0x8009B6EC.. (+0 position, -1 in some cases; +4 last
    position; +8 the DS_CB3 callback word, 0 / sentinel / alias; +0x14 / +0x18
    callback arguments; +0x1C mode, 1 or not); 0x8009B6F8 (1 or not) and its
    counter 0x8009B6FC (positive, 0, -1, negative); 0x8009B70C (1 or not) and
    0x8009B700 / 0x8009B704."""
    x = gv.xs32(x)
    args[0] = (1, 1, 4, 4, 0, 2, 0xFF, x & 0xFF, 3, 5, 0x11, 0x81, 0x41, 0x84)[x % 14]   # v2: bytes that differ from 1 / 4 only above bit 2
    x = gv.xs32(x)
    res = ZONE + 0x1000 + (x % 0x100) * 4
    args[1] = res
    x = gv.xs32(x)
    ram[res & 0x1FFFFF] = (0, 0x10, 0x20, 0x80, 0xA0, 0x30, 0x90, x & 0xFF)[x % 8]
    for k, ch in ((0x00, (0xFFFFFFFF, 0xFFFFFFFF, 0, 0x100, 0x7FFFFFFF)), (0x04, (0, 0xFF, 0x100, 0x101, 0x80000000)),
                  (0x08, (0, DS_CB3, DS_CB3, DS_CB3 & 0x1FFFFFFF)), (0x0C, (0, 1)), (0x14, (0, 0x80131140)),
                  (0x18, (0, 0x80131180)), (0x1C, (1, 1, 0, 2))):
        x = gv.xs32(x)
        struct.pack_into('<I', ram, 0x9B6EC + k, ch[x % len(ch)] if (x & 0x700 or k == 0x08) else gv.xs32(x))   # the callback word is never wild
    for w, ch in ((0x9B6F8, (1, 1, 0, 2)), (0x9B6FC, CNT28), (0x9B70C, (1, 1, 0, 2, 3, 0x101)),
                  (0x9B700, (0, 0x801311C0)), (0x9B704, (0, 0x80131200)), (0x9B6F0 + 0x20, (0, 1, x))):
        x = gv.xs32(x)
        struct.pack_into('<I', ram, w, ch[x % len(ch)])
    return x


def _ds28_models(x, rets, stops):
    x = gv.xs32(x)
    code = x & 0x3F
    rets[0x80080C48] = code
    stops[0x80080C48] = _pos28_model(code)
    x = gv.xs32(x)
    code = x & 0xFF
    rets[DS_CB3] = rets[DS_CB3 & 0x1FFFFFFF] = code
    stops[DS_CB3] = stops[DS_CB3 & 0x1FFFFFFF] = _cb28_model(code)
    x = gv.xs32(x)
    code = x & 0xF
    rets[0x80082204] = code
    stops[0x80082204] = _blk28_model(code, 0x9B6FC)
    rets[0x8007FC18] &= 0xFF          # pe_plat_cd_sys_command_mode returns a byte
    return x



# Twenty-ninth batch (cdgen29): libcd CD_getsector2 0x8007C044 (generated TU): no callees.
CDGEN29 = {
    0x8007C044: ('ww', ()),
}
GS29_PTRS = (0x9B27C, 0x9B288, 0x9B28C, 0x9B2B0, 0x9B2B4, 0x9B2B8, 0x9B2BC, 0x9B2C0)
GS29_K = (0, 0, 1, 1, 2, 3, 4, 5, 8, 12)


def _gs29_seed(ram, x, args):
    """cdgen29: arguments, DPCR and register words, the status-bit model and
    the pointer-word discriminators (see the docstring)."""
    x = gv.xs32(x)
    args[0] = (0, 1, 0xFFFFFFFF, 0x80000000, 0x7FFFFFFF, 0x80100000, 0x00100000, 0xA0100000,
               ZONE + (x >> 8) % 0x400 * 4, gv.xs32(x))[x % 10]
    x = gv.xs32(x)
    args[1] = (0, 1, 3, 0x200, 0xFFFF, 0x10000, 0x10200, 0xFFFF0000, 0xFFFFFFFF, 0x80000000,
               0x7FFFFFFF, (x >> 8) & 0xFFFF, gv.xs32(x))[x % 13]
    x = gv.xs32(x)
    struct.pack_into('<I', ram, 0x10F0, (0, 0x8000, 0xFFFFFFFF, 0xFFFF7FFF, 0x07654321, 0x00888888,
                                         gv.xs32(x))[x % 7])
    for a in (0x1018, 0x1020, 0x10B0, 0x10B4, 0x10B8):
        x = gv.xs32(x)
        struct.pack_into('<I', ram, a, (0, 0xFFFFFFFF, 0x01000000, gv.xs32(x))[x % 4])
    x = gv.xs32(x)
    ram[0x1800] = x & 0xFF
    REGBITS.clear()
    REGCOUNT.clear()
    x = gv.xs32(x)
    REGBITS[(0x1800, 0x40)] = [1, 0, GS29_K[x % len(GS29_K)], 0]
    x = gv.xs32(x)
    if x & 1:
        noise = ((x >> 8) & 0xBF) or 0x80
        REGBITS[(0x1800, noise)] = [1, 0, 0, 0]
    x = gv.xs32(x)
    if x % 4 == 0:
        x = gv.xs32(x)
        kind = x % 8                     # v2: kinds 6 and 7
        x = gv.xs32(x)
        z = ZONE + 0x1800 + (x >> 8) % 0x100 * 4
        if kind == 0:                    # DPCR word -> zone (read-modify-write on plain memory)
            struct.pack_into('<I', ram, 0x9B2B4, z)
        elif kind == 1:                  # BCR word -> zone
            struct.pack_into('<I', ram, 0x9B2BC, z)
        elif kind == 2:                  # 0x1F801018 word -> zone
            struct.pack_into('<I', ram, 0x9B2B0, z)
        elif kind == 3:                  # 0x1F801020 word -> zone
            struct.pack_into('<I', ram, 0x9B28C, z)
        elif kind == 4:                  # MADR word -> the status word; a0 a zone byte with bit 6 set
            struct.pack_into('<I', ram, 0x9B2B8, 0x8009B27C)
            args[0] = z
            x = gv.xs32(x)
            ram[z & 0x1FFFFF] = 0x40 | (x & 0xBF)
        elif kind == 5:                  # BCR word -> the CHCR word; a1 | 0x10000 a zone address
            struct.pack_into('<I', ram, 0x9B2BC, 0x8009B2C0)
            args[1] = z & ~0x10000
            # v2: the DPCR word aims at the CHCR word too (which holds another zone
            # address), so its pre-store value is loaded first and is visible to
            # the checker (which seeds RAM from each address's first load)
            struct.pack_into('<I', ram, 0x9B2B4, 0x8009B2C0)
            struct.pack_into('<I', ram, 0x9B2C0, z ^ 0x400)
        elif kind == 6:                  # v2: DPCR word -> the MADR word (a zone address; its bit 15 set by the store)
            struct.pack_into('<I', ram, 0x9B2B4, 0x8009B2B8)
            struct.pack_into('<I', ram, 0x9B2B8, z)
        else:                            # v2: MADR word -> the BCR word; a0 a zone address; the DPCR
            struct.pack_into('<I', ram, 0x9B2B8, 0x8009B2BC)   # word aims at the BCR word (another zone
            args[0] = z                                         # address), so its old value is loaded first
            struct.pack_into('<I', ram, 0x9B2B4, 0x8009B2BC)
            struct.pack_into('<I', ram, 0x9B2BC, z ^ 0x400)
    return x



# Thirtieth batch (cdgen30): libds text_360 0x8007E8F4 (host adapter in pe_libcd.c).
CDGEN30 = {
    0x8007E8F4: ('', (0x8007FBF0, 0x8007FB44)),
}


def _ds30_seed(ram, x, args):
    """cdgen30: the queue index word 0x800A3604 and the 24-byte records
    around 0x800A3540 (see the docstring)."""
    for a in range(0xA3540 - 24 * 8, 0xA3540 + 24 * 16, 4):
        x = gv.xs32(x)
        struct.pack_into('<I', ram, a, (0, 0, 1, 2, 0x80000000, 0xFFFFFFFF, x & 0xFF, gv.xs32(x))[x % 8])
    x = gv.xs32(x)
    idx = (0, 0, 1, 2, 3, 5, 7, 8, 9, 15, 0xFFFFFFFF, 0xFFFFFFF8, (x >> 8) % 16)[x % 13]
    struct.pack_into('<I', ram, 0xA3604, idx)
    rec = (0xA3540 + 24 * idx) & 0xFFFFFFFF
    if 0xA3540 - 24 * 8 <= rec < 0xA3540 + 24 * 16 and rec != 0xA3600:
        x = gv.xs32(x)                   # the record's first word: zero or not, sign bit
        struct.pack_into('<I', ram, rec, (0, 0, 1, 0x80000000, 0xFFFFFFFF, 0x100, gv.xs32(x))[x % 7])
        x = gv.xs32(x)                   # byte +4: full range
        ram[rec + 4] = x & 0xFF
    return x



# Thirty-first batch (cdgen31): libds text_5F4 0x8007EB88 (hand port in cd_stream_port.c).
CDGEN31 = {
    0x8007EB88: ('www', (0x80080998,)),
}


def _ds31_seed(ram, x, args):
    """cdgen31: the ring index word 0x800A3690 and the arguments (see the docstring)."""
    for a in range(0xA3600, 0xA36A0, 4):
        x = gv.xs32(x)
        struct.pack_into('<I', ram, a, gv.xs32(x))
    x = gv.xs32(x)
    idx = (0, 1, 2, 3, 4, 5, 6, 7, 7, 8, 9, 15, 0xFFFFFFFF, 0x7FFFFFFF, 0x80000000)[x % 15]
    struct.pack_into('<I', ram, 0xA3690, idx)
    x = gv.xs32(x)
    args[0] = (0, 1, 7, 8, 0xFF, 0x7FFFFFFF, 0x80000000, 0xFFFFFFFF, gv.xs32(x))[x % 9]
    if idx == 8:                     # the sequence store rewrites the index word
        x = gv.xs32(x)
        args[0] = (0, 1, 3, 6, 7, 8, 9, 0xFFFFFFFF)[x % 8]
    x = gv.xs32(x)
    args[1] = (x & 0xFF) if x & 1 else (0x100, 0x1FF, 0xFFFFFFFF, 0x80, 0x7F, gv.xs32(x))[(x >> 8) % 6]
    x = gv.xs32(x)
    args[2] = ZONE + (x % 0x400) * 4 if x % 4 else gv.xs32(x)
    return x



# Thirty-second batch: StSetStream, five o32 arguments.
CDGEN32 = {0x8007C304: ('wwwww', (0x8007C544,))}


def _stream32_seed(ram, x, args):
    for a in (0xC0DB8, 0xB0CC8, 0xA801C, 0xB8620,
              0xB6914, 0xA8018, 0xA5D54, 0xB0CCC):
        x = gv.xs32(x)
        struct.pack_into('<I', ram, a, x | 0x01010101)
    return x


# Thirty-third batch (sw-cd): see the docstring.
CDGEN33 = {
    0x800824F0: ('w', (0x80073CF4,)),
    0x8007F72C: ('', (0x8007F778, 0x8007FBF0)),
    0x8007C13C: ('', (0x8007AAB4,)),
}
POS33 = (1, 2, 4, 6, 3, 7, 0x102, 0x104, 0x7FFFFFFF, 0x100, 0xFFFFFFFF, 0x80000000, 0x10000, 0xFF, 0x10002, 8)
FIN33 = (0, 0, 0, 0, 0, 0, 0, 0)   # any nonzero final result makes 0x8007C13C loop forever (pilot: 104 of 120 cases spun)
IDX33 = (0, 1, 2, 3, 0x10, 0x21, 0xFF, 0x80)


def _ack33_model(code):
    """0x8007AAB4 (no argument): POS33[p] for the first k calls, then FIN33[f];
    on call 1 it writes the byte at the register pointer's target (the word
    0x8009B27C) = IDX33[w] (w == 8: none), absolute and logged as a store."""
    k, p, f, w = code & 15, (code >> 4) & 15, (code >> 8) & 15, (code >> 12) & 15
    n = [0]

    def fn(ram, r):
        n[0] += 1
        if n[0] == 1 and w < 8:
            ptr = struct.unpack_from('<I', ram, 0x9B27C)[0]
            ram[ptr & 0x1FFFFF] = IDX33[w]
            pl.STORES.append((ptr, 1, IDX33[w]))
        return POS33[p] if n[0] <= k else FIN33[f]
    return fn


CD33_CB = ((0x9AFB4, 0x80131140), (0x9AFB8, 0x80131180))   # callback words and their sentinels (alias: & 0x1FFFFFFF)


def _cd33_seed(ram, x, entry, args):
    if entry == 0x8007C13C:           # the register pointer word: the CD block, or plain memory in a quarter
        x = gv.xs32(x)
        if x % 4 == 0:
            struct.pack_into('<I', ram, 0x9B27C, ZONE + ((x >> 8) % 0x400) * 4)
        for word, sent in CD33_CB:    # callback words: 0, the sentinel, or its KUSEG alias (positive nonzero)
            x = gv.xs32(x)
            struct.pack_into('<I', ram, word, (0, sent, sent, sent & 0x1FFFFFFF)[x % 4])
        x = gv.xs32(x)
        struct.pack_into('<I', ram, 0x9B294, x)      # the status bytes handed to the callbacks (+0, +1)
    return x


# Thirty-fifth batch (cdgen35): SIO0 start routine 0x80082E00.
CDGEN35 = {0x80082E00: ('p', (0x80083578, 0x80084FC4, SLOT23_44, SLOT23_48))}
SIO35_SEQ = {}            # register address -> remaining values for successive reads (the last one repeats)
_pre35_load = pl._ns['LOAD']


def _sio35_load(fmt, buf, a):
    q = SIO35_SEQ.get(a)
    if q:
        n = struct.calcsize(fmt)
        val = q.pop(0) if len(q) > 1 else q[0]
        struct.pack_into(fmt, buf, a, val & ((1 << (8 * n)) - 1))
    return _pre35_load(fmt, buf, a)


pl._ns['LOAD'] = _sio35_load
W35_CELLS = (0x9B774, 0x9B778, 0x9B77C, 0x9B780, 0x9B78C, 0x9B790, 0x9B794, 0x9B798, 0x9B79C)
W35_VALS = (0xFFFFFFFF, 0xFFFFFFFD, 0, 0, 1, 2, 3, 4, 6, 5, 0x80000000, 0xFFFFFFFB, 0)
IDX35 = (0, 0, 0, 1, 1, 0xFFFFFFFF, 4, 5, 6, 7, 8, 0xFFFFFFFE, 0x40000000, 0x40000001, 0xC0000000, 0)
RC35 = (0, 1, 7, 0xF0, 0x100, 0x12345678, 0xFFFFFFFF, 0x80000000)
RV35 = (0, 1, 0xFFFFFFFF, 0x42)


def _mut35(ram, rec, code, n_call):
    """Slot model: the write made on the slot's m-th call (code bits: 0-1
    return value, 2-4 write kind, 5-8 value index, 9-10 cell choice, 11-13
    m).  Absolute writes, applied to RAM and logged as stores."""
    kind, vi, m = (code >> 2) & 7, (code >> 5) & 15, (code >> 11) & 7
    if n_call != m or kind == 7:
        return
    if kind == 0:
        _store23(ram, (rec + 0xC) & 0x1FFFFF, 4, RC35[vi & 7])
    elif kind == 1:
        _store23(ram, W35_CELLS[1 + ((code >> 9) & 3)] & 0x1FFFFF, 4, W35_VALS[vi % len(W35_VALS)])
    elif kind == 2:
        _store23(ram, 0x9B764, 4, IDX35[vi])
    elif kind == 3:
        _store23(ram, (rec + 0x50) & 0x1FFFFF, 1, (0, 1, 2, 0xFF)[vi & 3])
    elif kind == 4:
        _store23(ram, (rec + 0x36) & 0x1FFFFF, 1, (0, 1, 2, 0x80)[vi & 3])
    elif kind == 5:
        _store23(ram, (rec + 0x3C) & 0x1FFFFF, 4, (ZONE + 0x1E10, ZONE + 0x1E00, ZONE + 0x1E01, ZONE + 0x1E20)[vi & 3])
    else:
        _store23(ram, 0x9B784, 4, (0x1F801070, ZONE + 0x1F00, 0x1F801070, ZONE + 0x1F10)[vi & 3])


def _sio35_slot(code, rec):
    cnt = [0]

    def f(ram, r):
        cnt[0] += 1
        _mut35(ram, rec, code, cnt[0])
        return RV35[code & 3]
    return f


def _sio35_seed(ram, x, args):
    x = gv.xs32(x)
    rec = ZONE + (x % 0x200) * 4
    args[0] = rec
    for k in range(0, 0x100, 4):             # filler
        x = gv.xs32(x)
        struct.pack_into('<I', ram, (rec + k) & 0x1FFFFF, x)
    x = gv.xs32(x)
    struct.pack_into('<I', ram, (rec + 0xC) & 0x1FFFFF, RC35[x % len(RC35)] if x & 0x100 else gv.xs32(x))
    x = gv.xs32(x)                            # +0x3C is stored through unconditionally: a record-zone pointer (low
    # pointers touch unlogged RAM and the harness drops those cases)
    struct.pack_into('<I', ram, (rec + 0x3C) & 0x1FFFFF,
                     (ZONE + 0x1E10, ZONE + 0x1E00, ZONE + 0x1E01, ZONE + 0x1000 + (x >> 8) % 0x600)[x % 4])
    for off, ch in ((0x36, (0, 0, 1, 2, 0x80, 0xFF)), (0x50, (0, 0, 1, 2, 0xFF))):
        x = gv.xs32(x)
        ram[(rec + off) & 0x1FFFFF] = ch[x % len(ch)] if x & 0x100 else (x >> 16) & 0xFF
    x = gv.xs32(x)                            # the byte at +0xE8: 8 selects the RCNT2 limit 0x50 instead of 0x91
    ram[(rec + 0xE8) & 0x1FFFFF] = 8 if x % 3 == 0 else (x >> 8) & 0xFF
    x = gv.xs32(x)
    struct.pack_into('<I', ram, 0x9B764, IDX35[x % len(IDX35)] if x & 0x100 else (0, 1, 0xFFFFFFFF)[x % 3])
    for a in W35_CELLS:
        x = gv.xs32(x)
        struct.pack_into('<I', ram, a, W35_VALS[x % len(W35_VALS)])
    x = gv.xs32(x)
    base = 0x1F801040
    if x % 6 == 0:                            # SIO block in the record zone
        base = ZONE + 0x1C00
        struct.pack_into('<I', ram, 0x9B788, base)
        for off in (0x4, 0xA):
            x = gv.xs32(x)
            struct.pack_into('<H', ram, (base + off) & 0x1FFFFF, x & 0xFFFF)
    x = gv.xs32(x)
    if x % 4 == 0:                            # the interrupt-status pointer word
        struct.pack_into('<I', ram, 0x9B784, ZONE + 0x1F00)
    SIO35_SEQ.clear()
    if base == 0x1F801040:
        for a, n in ((0x1044, 3), (0x104A, 2)):
            seq = []
            for _ in range(n):
                x = gv.xs32(x)
                if a == 0x1044:
                    seq.append(((x >> 8) & 0xFDFF) | (0x200 if x & 1 else 0))
                else:
                    seq.append((0, 0x1003, 0x3003, 0xFFFF, 0x10, 0x2000, (x >> 8) & 0xFFFF)[x % 7])
            SIO35_SEQ[a] = seq
        x = gv.xs32(x)
        if x & 1:                             # the data register read after the byte write: any value
            SIO35_SEQ[0x1040] = [(x >> 8) & 0xFF]
    return x


def _sio35_models(x, rets, stops, ram, args):
    for sent in (SLOT23_44, SLOT23_48):
        x = gv.xs32(x)
        code = x & 0x3FFF
        rets[sent] = rets[sent & 0x1FFFFFFF] = code
        stops[sent] = stops[sent & 0x1FFFFFFF] = _sio35_slot(code, args[0])
    for a, sent in ((0x9B744, SLOT23_44), (0x9B748, SLOT23_48)):
        x = gv.xs32(x)
        struct.pack_into('<I', ram, a, sent if x % 3 else sent & 0x1FFFFFFF)

# Thirty-sixth batch (cdgen36): libcd CD_ready 0x8007B290 (mode, result pointer; BIOS-side CD
# readiness poll) and 0x80081110 (VSync-timed gate on the libds block 0x8009B6AC..).  Their
# callees are stopped and recorded; VSync, CheckCallback and getintr follow scripted models
# whose parameters are the RETS entries (pseudo callees 1 and 2 carry the extra codes):
#   CD_ready  RETS = printf, VSync base, puts, CheckCallback code, getintr code, CD_flush,
#             VSync step, getintr memory-effect code, sync / ready callback results (the words
#             0x8009AFB4 / 0x8009AFB8 hold 0, a sentinel or its alias; getintr results are bit sets)
#   0x80081110 RETS = VSync base, 0x80081268 return, VSync step, callback return, pump effect
#             (bit 0 on: the pump rewrites the counter word 0x8009B6B4 absolutely with value
#             index bits 1-2: -1, 0, 5, 0x80000000)
# VSync (the n-th call) returns base + n * step.  CheckCallback / getintr return `pos` for
# their first k calls, then `final` (code = k | pos index << 8 | final index << 12;
# CheckCallback is masked to 16 bits).  The getintr effect code (bit 0 on) makes its m-th call
# (bits 1-2, 1..3) store the ready byte 0x8009B296 (bits 3-4) and the sync byte 0x8009B295
# (bits 5-6) absolutely and 8 response bytes at 0x800A3470 / 0x800A3468 (bit 7); bits 8-11 (< 8)
# make its first call store RD36_IDX[v] through the register pointer word 0x8009B27C.
VSYNC, CHKCB, GETINTR, PUTS = 0x80073A44, 0x80073DE8, 0x8007AAB4, 0x80073C5C
RD36_CB = 0x80130B40       # 0x80081110: the callback word 0x8009B6D0 holds 0 or this sentinel
RD36_SYNC_CB, RD36_READY_CB = 0x80130C40, 0x80130C80   # CD_ready: the callback words 0x8009AFB4 / 0x8009AFB8 hold 0 or these
CDGEN36 = {
    0x8007B290: ('wp', (PRINTF, VSYNC, PUTS, CHKCB, GETINTR, 0x8007B9EC, 1, 2, RD36_SYNC_CB, RD36_READY_CB)),
    0x80081110: ('ww', (VSYNC, 0x80081268, 1, RD36_CB, 2)),
}
RD36_STEPS = (0, 1, 3, 7, 7, 50, 300, 0x10000, 0x80000000, 0xFFFFFFFF,
              959, 960, 961, 480, 479, 481, 320, 321, 319, 2, 1000)
RD36_WORDS = {0x8007B290: tuple(range(0xA3470, 0xA3490, 4)) + tuple(range(0xA3468, 0xA3470, 4)) +
              tuple(range(0x9AFD0, 0x9AFE0, 4)) + (0x9B05C, 0x9B290, 0x9B294, 0x9B298),
              0x80081110: tuple(range(0x9B6AC, 0x9B6C4, 4)) + (0x9B6C4, 0x9B6C8, 0x9B6CC)}
RD36_CHOICES = (0, 0, 1, 2, 3, 0xFF, 0x100, 0x7FFFFFFF, 0x80000000, 0xFFFFFFFF)
RD36_BYTES = (0, 0, 0, 1, 2, 0x55, 0x80, 0xFF)
RD36_IDX = (0, 1, 2, 3, 0x10, 0x21, 0xFF, 0x80)   # the index byte getintr's first call writes
RD36_POS = (0, 1, 2, 0xFFFFFFFF, 4, 6, 3, 8, 0x10000, 0x100)   # poll model values (getintr results are bit sets)
RD36_FIN = (0, 1, 2, 3, 5, 0xFFFFFFFF, 0x10000, 4, 6, 7)


def _rd36_seed(entry, ram, x, args, seed):
    for a in RD36_WORDS[entry]:
        x = gv.xs32(x)
        struct.pack_into('<I', ram, a, RD36_CHOICES[(x >> 8) % len(RD36_CHOICES)] if x & 1 else gv.xs32(x))
    REGBITS.clear()
    if entry == 0x8007B290:
        for a in (0x9B295, 0x9B296):        # the sync / ready bytes: often zero, sometimes small / edge
            x = gv.xs32(x)
            if x & 1:
                ram[a] = RD36_BYTES[(x >> 8) % len(RD36_BYTES)]
        for a, sent in ((0x9AFB4, RD36_SYNC_CB), (0x9AFB8, RD36_READY_CB)):   # installed callbacks: none, sentinel, its alias
            x = gv.xs32(x)
            struct.pack_into('<I', ram, a, (0, sent, sent, sent & 0x1FFFFFFF)[x % 4])
        for _ in range(2):
            x = gv.xs32(x)
            REGBITS[(0x1800, 1 << (x % 8))] = [1, 0, (x >> 8) % 6, 0]
        x = gv.xs32(x)
        args[0] = (0, 1, 2, 3, 0xFF, 0x100, 0x80000000, gv.xs32(x))[x % 8]
        x = gv.xs32(x)                       # result pointer: NULL (no response copy) or a record-zone pointer
        args[1] = 0 if x % 5 == 0 else (0x80000000, 0x00000000, 0xA0000000)[(x >> 8) % 3] | (ZONE & 0x1FFFFF) | (x >> 12) % 0x400 * 4
    else:
        x = gv.xs32(x)
        struct.pack_into('<I', ram, 0x9B6D0, (0, 0, 0, RD36_CB, RD36_CB, RD36_CB & 0x1FFFFFFF)[x % 6])
    return x


def _rd36_models(entry, x, rets, stops, ram, seed):
    """See the batch comment: VSync, CheckCallback and getintr models, the sweep of the
    deadline distance for 0x80081110, and the getintr memory effect for CD_ready."""
    x = gv.xs32(x)
    base = (0, 1, 100, 0x7FFFFFFF, 0x80000000, 0xFFFFFFF0, x & 0xFFFF, gv.xs32(x),
            0x7FFFFFFF - (x >> 8) % 2000, 0x80000000 - (x >> 8) % 2000)[x % 10]
    x = gv.xs32(x)
    step = RD36_STEPS[x % len(RD36_STEPS)]
    if entry == 0x80081110 and seed % 2 == 0:
        # sweep: the first VSync result lies delta ticks beyond the word 0x8009B6C4
        j = seed // 2
        step = (1, 1, 2, 7, 40, 700)[(j // 7) % 6]
        delta = 380 + (j * 7) % 2400
        if j < 126:                          # boundary block: distances 1190..1210 with every step
            step = (1, 1, 2, 7, 40, 700)[j // 21]
            delta = 1190 + j % 21
        if j >= 126 and j % 4 in (1, 3):     # words near a signed wrap: T + 1200 crosses it
            struct.pack_into('<I', ram, 0x9B6C4, ((0x7FFFFFFF, 0xFFFFFFFF)[j % 4 // 2] - (j * 13) % 3000) & 0xFFFFFFFF)
        T = struct.unpack_from('<I', ram, 0x9B6C4)[0]
        base = (T + delta - step) & 0xFFFFFFFF
        c = struct.unpack_from('<I', ram, 0x9B6B4)[0]
        if c == 1:
            struct.pack_into('<I', ram, 0x9B6B4, 2 + (seed % 97))
    rets[VSYNC], rets[1] = base, step
    n = [0]

    def vsync(ram_, r, base=base, step=step):
        n[0] += 1
        return (base + n[0] * step) & 0xFFFFFFFF
    stops[VSYNC] = vsync
    if entry == 0x8007B290:
        for c in (CHKCB, GETINTR):
            x = gv.xs32(x)
            k, pi, fi = x % 5, (x >> 4) % 10, (x >> 8) % 10
            pos, final = RD36_POS[pi], RD36_FIN[fi]
            if c == CHKCB:
                pos &= 0xFFFF
                final &= 0xFFFF
            rets[c] = k | pi << 8 | fi << 12
            model = _poll_model(k, pos, final)
            if c == GETINTR:
                x = gv.xs32(x)
                eff = (x & 0xFF) if x & 0x100 else 0
                x = gv.xs32(x)
                eff |= (x % 12) << 8             # bits 8-11: index byte written on the first call (>= 8: none)
                calls = [0]

                def gi(ram_, r, model=model, eff=eff, calls=calls):
                    calls[0] += 1
                    if calls[0] == 1 and (eff >> 8) < 8:   # production getintr selects CD index 1 under the caller
                        ptr = struct.unpack_from('<I', ram_, 0x9B27C)[0]
                        ram_[ptr & 0x1FFFFF] = RD36_IDX[eff >> 8]
                        pl.STORES.append((ptr, 1, RD36_IDX[eff >> 8]))
                    if eff & 1 and calls[0] == 1 + ((eff >> 1) & 3) % 3:
                        _store23(ram_, 0x9B296, 1, RD36_BYTES[(eff >> 3) & 3 | 4 * ((eff >> 3) & 1)])
                        _store23(ram_, 0x9B295, 1, RD36_BYTES[(eff >> 5) & 3 | 4 * ((eff >> 5) & 1)])
                        if eff & 0x80:
                            for j in range(8):
                                _store23(ram_, 0xA3470 + j, 1, (0x31 * j + eff) & 0xFF)
                                _store23(ram_, 0xA3468 + j, 1, (0x17 * j + eff) & 0xFF)
                    return model(ram_, r)
                rets[2] = eff
                stops[c] = gi
            else:
                stops[c] = model
        for cb in (RD36_SYNC_CB, RD36_READY_CB):
            x = gv.xs32(x)
            rets[cb] = (0, 1, 0xFFFFFFFF, 7, gv.xs32(x))[x % 5]
            stops[cb & 0x1FFFFFFF] = None
            rets[cb & 0x1FFFFFFF] = rets[cb]
    else:
        x = gv.xs32(x)
        rets[RD36_CB] = (0, 1, 0xFFFFFFFF, 7, gv.xs32(x))[x % 5]
        stops[RD36_CB & 0x1FFFFFFF] = None
        rets[RD36_CB & 0x1FFFFFFF] = rets[RD36_CB]
        x = gv.xs32(x)                       # pump effect: in a third of the cases it rewrites the counter word absolutely
        eff = (x & 3) | ((x >> 2) & 3) << 1 if x % 3 == 0 else 0
        rets[2] = eff

        def pump(ram_, r, eff=eff):
            if eff & 1:
                _store23(ram_, 0x9B6B4, 4, (0xFFFFFFFF, 0, 5, 0x80000000)[(eff >> 1) & 3])
            return rets[0x80081268]
        stops[0x80081268] = pump
    return x

# Thirty-fourth batch (cdgen34): libpad communication start 0x80082CF0.
PAD34_SA, PAD34_SB = 0x80131240, 0x80131280      # slot sentinels (arity one)
CDGEN34 = {0x80082CF0: ('', (0x80072714, 0x80072724, 0x80073C84, 0x8007E1E4, 0x8007E1F4, PAD34_SA, PAD34_SB))}
PAD34_NAMES = (0x80072714, 0x80072724, 0x80073C84, 0x8007E1E4, 0x8007E1F4, PAD34_SA, PAD34_SB)
P34_PTR = (0x1F801070, ZONE + 0x1F00, ZONE + 0x1F40, ZONE + 0x1F80)
P34_ARG = (0, 1, 2, 0x1000, 0xFFFFFF10, 0xFFFFFF0F, 0x7FFFFF10, 0x80000000, 0xFFFFFFFF, 0x12345678, 0xF0, 0xEF)
P34_MASK = (0, 1, 2, 0x80, 0xFFFE, 0xFFFF, 0x10000, 0xFFFFFFFE, 0xFFFFFFFF, 0x80000000, 0x12345678, 0x7FFFFFFF)
P34_SLOT = (0, PAD34_SA, PAD34_SA & 0x1FFFFFFF, PAD34_SB, PAD34_SB & 0x1FFFFFFF)
P34_CNT = (0, 1, 0xFFFFFFFF, 0x12345678)


def _mut34(ram, code, n_call, is_slot):
    """A callee's write on its m-th call (code bits 0-2 kind, 3-6 value index,
    7 call number for a slot: 1 or 2; a BIOS callee is called once).
    Absolute writes, applied to RAM and logged as stores."""
    kind, vi = code & 7, (code >> 3) & 15
    if n_call != (1 + ((code >> 7) & 1) if is_slot else 1):
        return
    if kind == 1:
        _store23(ram, 0x9B784, 4, P34_PTR[vi & 3])
    elif kind == 2:
        _store23(ram, 0x9B758, 4, P34_ARG[vi % len(P34_ARG)])
    elif kind == 3:
        _store23(ram, 0x9B728, 4, P34_SLOT[vi % len(P34_SLOT)])
    elif kind == 4:
        _store23(ram, (P34_PTR[1 + (vi & 1) + ((vi >> 1) & 1)] + 4) & 0x1FFFFF, 4, P34_MASK[(vi * 5 + 1) % len(P34_MASK)])
    elif kind == 5:
        _store23(ram, 0x9B75C, 4, (0, 1, 0xFFFFFFFF, 0x42)[vi & 3])
    elif kind == 6:
        _store23(ram, (0xA5AC0, 0xA5AC4)[vi & 1], 4, P34_CNT[(vi >> 1) & 3])


def _pad34_callee(code, is_slot):
    cnt = [0]

    def f(ram, r):
        cnt[0] += 1
        _mut34(ram, code, cnt[0], is_slot)
        return code & 0x3F
    return f


def _pad34_seed(ram, x, args):
    x = gv.xs32(x)
    struct.pack_into('<I', ram, 0x9B784, P34_PTR[0] if x & 3 == 0 else P34_PTR[1 + (x >> 4) % 3])
    for ptr in P34_PTR[1:]:                   # the mask word behind a zone pointer, and the status word
        x = gv.xs32(x)
        struct.pack_into('<I', ram, (ptr + 4) & 0x1FFFFF, P34_MASK[x % len(P34_MASK)] if x & 0x100 else gv.xs32(x))
        x = gv.xs32(x)
        struct.pack_into('<I', ram, ptr & 0x1FFFFF, x)
    x = gv.xs32(x)                            # the mask register (hardware read)
    struct.pack_into('<I', ram, 0x1074, P34_MASK[x % len(P34_MASK)] if x & 0x100 else gv.xs32(x))
    x = gv.xs32(x)
    struct.pack_into('<I', ram, 0x9B758, P34_ARG[x % len(P34_ARG)] if x & 0x100 else gv.xs32(x))
    x = gv.xs32(x)
    struct.pack_into('<I', ram, 0x9B728, (0, 0, P34_SLOT[1], P34_SLOT[1], P34_SLOT[2], P34_SLOT[3], P34_SLOT[4])[x % 7])
    for a in (0x9B75C, 0xA5AC0, 0xA5AC4):
        x = gv.xs32(x)
        struct.pack_into('<I', ram, a, (0, 1, 0xFFFFFFFF, x)[x % 4])
    return x


def _pad34_models(x, rets, stops, ram, args):
    for sent in PAD34_NAMES:
        x = gv.xs32(x)
        code = x & 0xFF if x & 0x300 else 0      # about a quarter of the callees make a write
        is_slot = sent in (PAD34_SA, PAD34_SB)
        if is_slot and x & 0xC00 == 0:             # some slot calls zero the slot word (the second read is null-checked)
            code = 3 | (x & 0x80)
        rets[sent] = rets[sent & 0x1FFFFFFF] = code
        stops[sent] = stops[sent & 0x1FFFFFFF] = _pad34_callee(code, is_slot)
    return x


# Thirty-eighth and thirty-ninth batches (sw-cd): see the docstring.
CDGEN38 = {
    0x80082314: ('', (0x8007F0C8, 0x8007F72C, 0x8007FBF0, 0x80080B44, 0x80081DF8, 0x800822AC)),
}
SV38 = (0, 2, 4, 0xFFFFFFFF, 0x101, 0x10001, 0x80000000, 5, 3, 1,
        0x103, 0x10003, 0x80000003, 0x80000001, 0x1000001, 0x3000000)   # idx 8 = 3, idx 9 = 1; the rest test as neither
OTHER38 = (0, 1, 2, 3, 4, 5, 6, 7, 10, 11, 12, 13, 14, 15)
LANE38 = (2, 2, 2, 0, 1, 3, 0x102, 0x10002, 0xFFFFFFFF)       # first lane read
LANE38B = (0x10, 0x10, 0, 1, 2, 0x110, 0x10010, 0x1010, 0x80000010, 0x20, 0xFFFFFFFF, 0x100)   # second lane read
PKT38 = (0, 0, 0, 1, 2, 0x100, 0x10000, 0x80000000, 0xFFFFFFFF)
WORD38 = (1, 2, 3, 4, 0x100, 0x10000, 0x80000000, 0xFFFFFFFF)


def _ds38_models(x, rets, stops):
    x = gv.xs32(x)
    m = 1 + (x % 7 if x & 0x100 else (x >> 12) % 4)
    code = m
    for i in range(1, m + 1):
        x = gv.xs32(x)
        if i == m:
            idx = 9 if i % 2 else 8
        else:                                # neither the call's terminal value nor (odd) 1 / (even) 3
            pool = OTHER38 + ((8,) if i % 2 else (9,))   # an odd call may see 3, an even one 1
            idx = pool[x % len(pool)]
        code |= idx << (4 * i)
    rets[0x8007F72C] = code
    n = [0]

    def ss(ram, r):
        n[0] += 1
        return SV38[(code >> (4 * min(n[0], m))) & 15]
    stops[0x8007F72C] = ss
    x = gv.xs32(x)
    i0, i1 = x % 9, (x >> 8) % 12
    rets[0x8007FBF0] = i0 | i1 << 4          # first lane read LANE38[i0], every later one LANE38B[i1]
    stops[0x8007FBF0] = (lambda ln: (lambda ram, r: LANE38[i0] if ln.append(1) or len(ln) == 1 else LANE38B[i1]))([])
    x = gv.xs32(x)
    rets[0x8007F0C8] = PKT38[x % 9] if x & 0x100 else gv.xs32(x)
    if rets[0x8007F0C8]:
        x = gv.xs32(x)
        k = x % 5
        x = gv.xs32(x)
        v = WORD38[x % 8] if x & 0x100 else gv.xs32(x) | 1
        x = gv.xs32(x)
        w = (v, 0, 1, v, gv.xs32(x))[(x >> 8) % 5]
        MEMSEQ[0xB28F8] = [[0] * k + [v, w], 0]
    return x


# Thirty-fifth batch (sw-cd): see the docstring.
CD39_CB = (0x80131200, 0x80131240)
HEAD39F = (0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 8, 9, 15, 0xFFFFFFFF, 0x7FFFFFFF, 0x80000000)
COUNT39 = (0, 1, 1, 2, 2, 3, 4, 5, 8, 9, 16, 0xFFFFFFFF, 0x80000000)
CNTW39 = (0, 1, 2, 3, 5, 0xFFFFFFFF, 0x80000000, 9)


def _ds39_seed_flush(ram, x, args):
    for k in range(-2, 26):
        a = (0xA3540 + 0x18 * k) & 0x1FFFFF
        x = gv.xs32(x)
        struct.pack_into('<I', ram, a, (0, 0, 1, 2, 2, x & 0xFF, 0xFFFFFFFF, 0x80000000, x)[x % 9])
        x = gv.xs32(x)
        sent = CD39_CB[(x >> 8) & 1]
        struct.pack_into('<I', ram, a + 0x10, (0, 0, sent, sent, sent & 0x1FFFFFFF)[x % 5])
    x = gv.xs32(x)
    struct.pack_into('<I', ram, 0xA3600, HEAD39F[x % len(HEAD39F)])
    x = gv.xs32(x)
    struct.pack_into('<I', ram, 0xA3608, COUNT39[x % len(COUNT39)])
    x = gv.xs32(x)
    args[1] = ZONE + (x % 0x400) * 4 if x % 4 else gv.xs32(x)
    return x


def _ds39_models_flush(x, rets, stops):
    x = gv.xs32(x)
    m = x % 4                                 # 0: never rewrites the count word
    v = (x >> 8) % len(CNTW39)
    rets[0x8007EB88] = m | v << 4
    n = [0]

    def push(ram, r):
        n[0] += 1
        if m and n[0] == m:
            _store23(ram, 0xA3608, 4, CNTW39[v])
        return 0
    stops[0x8007EB88] = push
    for sent in CD39_CB:                      # callbacks stopped (sentinel and alias); results ignored
        stops[sent] = None
        stops[sent & 0x1FFFFFFF] = None
        rets[sent] = rets[sent & 0x1FFFFFFF] = gv.xs32(x)
    return x


# Thirty-sixth batch (sw-cd): see the docstring.
CDGEN39 = {
    0x8007E704: ('wp', (0x8007EB88,)),
    0x8007E964: ('bp', (0x8007E5C4, 0x8007EB88, 0x8007FB44, 0x8007FBF0, 0x80080998)),
}
CD39_ENTRY_CB = (0x80131300, 0x80131340)
CD39_GLOBAL_CB = (0x80131380, 0x801313C0)
LV39 = (1, 1, 1, 0, 2, 3, 0xFFFFFFFF, 0x101, 0x10001, 0x80000001)
CW39 = (0, 1, 2, 5, 0x100, 0xFFFFFFFF, 0x80000000, 3)
STATUS39 = (2, 2, 5, 5, 0, 1, 3, 4, 6, 0x102, 0x205, 0xFF, 0x12)
HEAD39S = (0, 1, 2, 3, 4, 5, 6, 7, 7, 7, 0, 1, 2, 8, 9, 0xFFFFFFFF, 0x80000000, 0x7FFFFFFF)
RETRY39 = (0, 1, 2, 3, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFE, 0x80000000, 0x7F000000, 0x100)


def _ds39_seed_sync(ram, x, args):
    prev = 0
    for k in range(-2, 26):
        a = (0xA3540 + 0x18 * k) & 0x1FFFFF
        x = gv.xs32(x)
        if x % 5 < 2 and prev:                       # a chained entry: the same sequence as the previous one
            seq = prev
        else:
            seq = (0, 0, 1, 2, 3, 4, 0xFFFFFFFF, x & 0xFF, x)[x % 9]
        prev = seq
        struct.pack_into('<I', ram, a, seq)
        x = gv.xs32(x)
        struct.pack_into('<I', ram, a + 4, x)
        struct.pack_into('<I', ram, a + 8, gv.xs32(x))
        x = gv.xs32(x)
        struct.pack_into('<I', ram, a + 0xC, x)
        x = gv.xs32(x)
        cb = CD39_ENTRY_CB[(x >> 8) & 1]
        struct.pack_into('<I', ram, a + 0x10, (0, 0, cb, cb, cb & 0x1FFFFFFF)[x % 5])
        x = gv.xs32(x)
        struct.pack_into('<I', ram, a + 0x14, RETRY39[x % 10] if x & 0x100 else (x >> 12) & 0xF)
    x = gv.xs32(x)
    struct.pack_into('<I', ram, 0xA3604, HEAD39S[x % len(HEAD39S)])
    x = gv.xs32(x)
    struct.pack_into('<I', ram, 0xA3600, (0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0xFFFFFFFF)[x % 11])
    x = gv.xs32(x)
    struct.pack_into('<I', ram, 0xA3608, (0, 1, 2, 5, 0x100, 0xFFFFFFFF, 0x80000000, 1, 3)[x % 9])
    x = gv.xs32(x)
    g = CD39_GLOBAL_CB[(x >> 8) & 1]
    struct.pack_into('<I', ram, 0xB8AB0, (0, g, g, g & 0x1FFFFFFF)[x % 4])
    x = gv.xs32(x)
    args[0] = STATUS39[x % len(STATUS39)] if x & 0x300 else args[0]
    x = gv.xs32(x)
    args[1] = ZONE + (x % 0x400) * 4 if x % 4 else gv.xs32(x)
    return x


def _ds39_models_sync(x, rets, stops):
    x = gv.xs32(x)
    i1, i2 = x % 10, (x >> 8) % 10
    rets[0x8007FBF0] = i1 | i2 << 4
    stops[0x8007FBF0] = (lambda ln: (lambda ram, r: LV39[i1] if ln.append(1) or len(ln) == 1 else LV39[i2]))([])
    x = gv.xs32(x)
    w, v = x & 1, (x >> 4) % 8
    rets[0x8007E5C4] = w | v << 4

    def pop(ram, r):
        if w:
            _store23(ram, 0xA3608, 4, CW39[v])
        return 0
    stops[0x8007E5C4] = pop
    rets[0x8007EB88] = 0                       # the queue push: no rewrite here
    for sent in CD39_ENTRY_CB + CD39_GLOBAL_CB:   # callbacks stopped (sentinel and alias); results ignored
        stops[sent] = None
        stops[sent & 0x1FFFFFFF] = None
        rets[sent] = rets[sent & 0x1FFFFFFF] = gv.xs32(x)
    return x


def cdgen_cases(base, entries=None, n=6000, table=None, indep=False, sweep=False):
    """indep: draw each callee's scripted return from its own xorshift step.
    The default (kept for the committed cdgen..cdgen5 sets) derives them
    from overlapping bit windows of one word, which leaves some
    combinations (e.g. -1 from the first callee with 0 from the second)
    unreachable.
    sweep: in every third case, each callee's return walks all byte values
    0x00..0xFF and -1..-16 (a different phase per callee), so a compare of
    a callee's result against any small constant is exercised.  Off by
    default so the committed vector sets reproduce."""
    out = []
    CDGEN_SPUN.clear()
    table = table or CDGEN
    for entry in entries or table:        # every static jal target must be a stopped callee
        _check_callees_complete(base, entry, table[entry][1])
    edges = (0, 1, 2, 3, 0x7F, 0x80, 0xFF, 0x100, 0x102, 0x202, 0x7FFFFFFF, 0x80000000, 0xFFFFFFFF)
    for entry in entries or table:
        kinds, callees = table[entry]
        for seed in range(1, n + 1):
            s = seed * 7919 ^ entry
            ram, x, ring, size = prep(base, s)
            for a in CDGEN_SEED.get(entry, ()):
                x = gv.xs32(x)
                struct.pack_into('<I', ram, a & 0x1FFFFF, (0, 1, 2, x & 0xFF, x)[x % 5])
            if entry == 0x800808BC:          # state / substate words: dense small values
                x = gv.xs32(x)
                struct.pack_into('<I', ram, 0x9B574, x % 5 if x & 7 else gv.xs32(x))
                x = gv.xs32(x)
                struct.pack_into('<I', ram, 0x9B578, x % 0x21 if x & 7 else gv.xs32(x))
            if entry == 0x800819D8:          # table at 0x800A3CB4: small count, then seeded words
                x = gv.xs32(x)
                struct.pack_into('<I', ram, 0xA3CB4, x % 13)
                for a in range(0xA3CB8, 0xA3F00, 4):
                    x = gv.xs32(x)
                    struct.pack_into('<I', ram, a, (0, 1, 2, 3, x & 0xFF, x)[x % 6])
                x = gv.xs32(x)
                if x % 8 == 0:               # discriminator: 48 nonzero keys that match no small a0
                    for k in range(48):
                        struct.pack_into('<I', ram, 0xA3CB4 + 0x2C * k, 4 + k % 4)
            args = []
            for k in kinds:
                x = gv.xs32(x)
                if k == 'b':
                    args.append(x & 0xFF if x & 1 else edges[(x >> 8) % len(edges)] & 0xFF)
                elif k == 'w':
                    args.append(edges[(x >> 8) % len(edges)] if x & 1 else gv.xs32(x))
                else:
                    args.append(ZONE + (x % 0x400) * 4 if x % 4 else gv.xs32(x))
            if entry == 0x800840DC:          # card record: zone pointer, its +0x3C object pointer, flag and +0x36 byte
                x = gv.xs32(x)
                rec = ZONE + (x % 0x300) * 4
                args[0] = rec
                x = gv.xs32(x)
                obj = ZONE + 0x1000 + (x % 0x300) * 4
                struct.pack_into('<I', ram, (rec + 0x3C) & 0x1FFFFF, obj)
                x = gv.xs32(x)
                ram[obj & 0x1FFFFF] = (0x80 | (x & 0xF)) if x & 0x30 else (x >> 8) & 0xFF
                x = gv.xs32(x)
                ram[(rec + 0x36) & 0x1FFFFF] = 0 if x & 1 else (x >> 8) & 0xFF
            if entry == 0x80080CDC:          # dst: NULL or a record-zone pointer (wild words alias registers/stack)
                x = gv.xs32(x)
                args[0] = 0 if x % 4 == 0 else ZONE + (x % 0x400) * 4
            for a in CDGEN_CB_FIXED.get(entry, ()):
                struct.pack_into('<I', ram, a, CARD_CB_SENT)
            if entry in CDGEN_CB_WORDS:      # callback word: 0, the sentinel, or its KUSEG alias (a positive nonzero word)
                x = gv.xs32(x)
                sent = CDGEN_CB_WORDS[entry][1]
                struct.pack_into('<I', ram, CDGEN_CB_WORDS[entry][0],
                                 (sent, sent, sent & 0x1FFFFFFF)[x % 3] if entry in CDGEN_CB_NONNULL
                                 else (0, sent, sent, sent & 0x1FFFFFFF)[x % 4])
            if entry in CDGEN_CB_WORD:
                x = gv.xs32(x)
                struct.pack_into('<I', ram, CDGEN_CB_WORD[entry], CD_CB_SENT if x & 1 else 0)
            if entry == 0x80082444:          # text_130: the a0 == 1 arm calls the stack-buffer callees
                x = gv.xs32(x)
                if x & 1:
                    args[0] = 1
            if entry == 0x80077548:          # GPU GP0/GPUSTAT read values seeded
                for a in (0x1810, 0x1814):
                    x = gv.xs32(x)
                    struct.pack_into('<I', ram, a, x if x & 1 else x & 0x1FFFFFFF)
            if entry == 0x800819D8:          # a0 matched against small table values
                x = gv.xs32(x)
                if x & 3:
                    args[0] = x % 4
            if entry == 0x800719C4:          # record word +4: flag bits around bit 3
                x = gv.xs32(x)
                args[0] = ZONE + (x % 0x300) * 4
                x = gv.xs32(x)
                struct.pack_into('<I', ram, (args[0] + 4) & 0x1FFFFF,
                                 (0, 8, 7, 0xF7, 0xFFFFFFFF, 0xFFFFFFF7, 0x10, x)[x % 8])
            if entry == 0x8007E4E0:          # source words 0x8007E344..0x8007E3B3 randomized (a copy, not constants)
                for a in range(0x7E344, 0x7E3B4, 4):
                    x = gv.xs32(x)
                    struct.pack_into('<I', ram, a, x if seed % 4 else (0, 0xFFFFFFFF, a)[seed % 3])
            if entry == 0x80082B08:          # card state record at *0x8009B784: words 0 and 1 (bit 0 tested)
                x = gv.xs32(x)
                rec = ZONE + (x % 0x300) * 4
                struct.pack_into('<I', ram, 0x9B784, rec)
                for k in (0, 4):
                    x = gv.xs32(x)
                    struct.pack_into('<I', ram, (rec + k) & 0x1FFFFF, (0, 1, 2, 3, 0xFFFFFFFE, 0xFFFFFFFF, x)[x % 7])
            if entry == 0x80084B78:          # card object: +0x3C pointer target byte, +0xE8, state +0x46, hook word +0x14
                x = gv.xs32(x)
                obj = ZONE + (x % 0x200) * 4
                args[0] = obj
                x = gv.xs32(x)
                p60 = ZONE + 0x1000 + (x % 0x200) * 4
                struct.pack_into('<I', ram, (obj + 0x3C) & 0x1FFFFF, p60)
                x = gv.xs32(x)
                ram[p60 & 0x1FFFFF] = 0xF3 if x % 3 == 0 else (0, 0xF2, 0xF4, 0xFF, x & 0xFF)[(x >> 8) % 5]
                x = gv.xs32(x)
                ram[(obj + 0xE8) & 0x1FFFFF] = 0 if x & 1 else (x >> 8) & 0xFF
                x = gv.xs32(x)
                ram[(obj + 0x46) & 0x1FFFFF] = (0, 1, 2, 3, 0x7F, 0x80, 0xFE, 0xFF, x & 0xFF)[x % 9]
                x = gv.xs32(x)
                struct.pack_into('<I', ram, (obj + 0x14) & 0x1FFFFF, (0, CARD_CB1, CARD_CB1, CARD_CB1 & 0x1FFFFFFF)[x % 4])
            if entry == 0x80084EB0:          # card object: +0x4C counter, state bytes +0x46/+0x49/+0x4A, pointers +0x30/+0x3C, +0xE8
                x = gv.xs32(x)
                obj = ZONE + (x % 0x200) * 4
                args[0] = obj
                x = gv.xs32(x)
                struct.pack_into('<I', ram, (obj + 0x4C) & 0x1FFFFF, (0, 1, 0x7FFFFFFF, 0xFFFFFFFF, x)[x % 5])
                for k, ch in ((0x46, (0, 0, 1, 1, 2, 0xFF, 0x80)), (0x4A, (0, 1, 2, 3, 4, 5, 0xFF)), (0x49, (0, 0, 1, 2, 0xFF))):
                    x = gv.xs32(x)
                    ram[(obj + k) & 0x1FFFFF] = ch[x % len(ch)] if x & 0x100 else (x >> 16) & 0xFF
                x = gv.xs32(x)
                p60 = ZONE + 0x1000 + (x % 0x200) * 4
                struct.pack_into('<I', ram, (obj + 0x3C) & 0x1FFFFF, p60)
                x = gv.xs32(x)
                ram[p60 & 0x1FFFFF] = 0xF3 if x % 3 == 0 else (0, 0xF2, 0xF4, 0xFF, x & 0xFF)[(x >> 8) % 5]
                x = gv.xs32(x)
                struct.pack_into('<I', ram, (obj + 0x30) & 0x1FFFFF, ZONE + 0x1800 + (x % 0x100) * 4)
                x = gv.xs32(x)
                ram[(obj + 0xE8) & 0x1FFFFF] = x & 0xFF
            if entry == 0x80083DF0:          # card object: bytes +0x46 and +0x53
                x = gv.xs32(x)
                args[0] = ZONE + (x % 0x300) * 4
                for k in (0x46, 0x53):
                    x = gv.xs32(x)
                    ram[(args[0] + k) & 0x1FFFFF] = (0, 0, 1, 2, 3, 0xFE, 0xFF, x & 0xFF)[x % 8]
            if entry == 0x80074BB8:          # libgpu state bytes 0x8009574C..0x8009574F
                for a in range(0x9574C, 0x95750):
                    x = gv.xs32(x)
                    ram[a] = (0, 1, 2, 0x7F, 0x80, 0xFF, x & 0xFF)[x % 7]
            if entry == 0x80080F98:          # read-progress state 0x8009B6AC..0x8009B6CC
                for a, choices in ((0x9B6AC, (0, 1, 2, 0x200, 0x7FFFFFFF)), (0x9B6B4, (0, 1, 2, 3, 0xFFFFFFFF, 0x80000000, 0x7FFFFFFF)),
                                   (0x9B6B8, (0, 1, 2, 3, 0xFFFFFFFE, 0xFFFFFFFF)), (0x9B6BC, (0, 1, 0x1234)), (0x9B6CC, (0, 1, 7))):
                    x = gv.xs32(x)
                    struct.pack_into('<I', ram, a, choices[x % len(choices)] if x & 0x100 else gv.xs32(x))
                x = gv.xs32(x)
                struct.pack_into('<I', ram, 0x9B6B0, ZONE + (x % 0x400) * 4 if x % 8 else gv.xs32(x))
            if entry == 0x8008214C:          # p = 0x8009B708: p[0], p[1] gate; p[-2], p[-1] callbacks
                for a in (0x9B708, 0x9B70C, 0x9B700, 0x9B704):
                    x = gv.xs32(x)
                    struct.pack_into('<I', ram, a, (0, 0, 1, 2, 0xFFFFFFFF, 0x80000000)[x % 6] if x & 0x100 else gv.xs32(x))
                x = gv.xs32(x)
                if x & 1:
                    args[0] = 2              # the a0 == 2 arm
            if entry == 0x800809E0:          # libds state block 0x8009B558..0x8009B59B and 0x800A36A0..A8
                for a in list(range(0x9B558, 0x9B59C, 4)) + [0xA36A0, 0xA36A4, 0xA36A8]:
                    x = gv.xs32(x)
                    struct.pack_into('<I', ram, a, (0, 1, 0xFF, 0x80000000, 0xFFFFFFFF, x & 0xFF, x)[x % 7])
            if entry == 0x80077144:          # mode: every low-3-bit value, high bits set in some; DMA/GPU words seeded
                x = gv.xs32(x)
                if x & 1:
                    args[0] = (x >> 8) % 8 | ((x >> 16) << 3 if x & 2 else 0)
                for a in (0x95874, 0x95878, 0x95884, 0x10A8, 0x10F0, 0x1814):
                    x = gv.xs32(x)
                    struct.pack_into('<I', ram, a, x)
            if table is CDGEN20:
                x = _card20_seed(ram, x, args)
            if table is CDGEN21:
                x = _sio21_seed(ram, x)
            if table is CDGEN22:
                x = _sio22_seed(ram, x, entry, args)
            if table is CDGEN23:
                x = _card23_seed(ram, x, args)
            if table is CDGEN24:
                x = _card24_seed(ram, x, args)
            if table is CDGEN25:
                x = _card25_seed(ram, x, args)
            if table is CDGEN26:
                x = _card26_seed(ram, x, args)
            if table is CDGEN28:
                x = _ds28_seed(ram, x, args)
            if table is CDGEN29:
                x = _gs29_seed(ram, x, args)
            if table is CDGEN30:
                x = _ds30_seed(ram, x, args)
            if table is CDGEN31:
                x = _ds31_seed(ram, x, args)
            if table is CDGEN32:
                x = _stream32_seed(ram, x, args)
            if table is CDGEN33:
                x = _cd33_seed(ram, x, entry, args)
            if table is CDGEN34:
                x = _pad34_seed(ram, x, args)
            if table is CDGEN35:
                x = _sio35_seed(ram, x, args)
            if table is CDGEN39:
                x = (_ds39_seed_flush(ram, x, args) if entry == 0x8007E704 else _ds39_seed_sync(ram, x, args))
            if table is CDGEN36:
                x = _rd36_seed(entry, ram, x, args, seed)
            ff_pool = None
            if table is CDGEN19:
                x, ff_pool = _ff_seed(ram, x, entry, args)
            x = gv.xs32(x)
            if indep:
                rets = {}
                for c in callees:
                    x = gv.xs32(x)
                    rets[c] = (0, 1, 0xFFFFFFFF, gv.xs32(x))[x % 4]
                if sweep and seed % 3 == 0:
                    for i, c in enumerate(callees):
                        k = (seed // 3 + 37 * i) % 0x110
                        rets[c] = k if k < 0x100 else (0xFF - k) & 0xFFFFFFFF
            else:
                rets = {c: (0, 1, 0xFFFFFFFF, x)[(x >> (3 + i)) % 4] for i, c in enumerate(callees)}
            for c in callees:                # a zero test vs a narrower mask: nonzero with the low 16 bits clear
                if c in CDGEN_HIGH_RETS:
                    x = gv.xs32(x)
                    if x % 6 == 0:
                        rets[c] = (0x10000 << ((x >> 8) % 16)) & 0xFFFFFFFF
            for c in callees:                # callees whose result is narrower than a word
                if c in CDGEN_RET_MASK:
                    rets[c] &= CDGEN_RET_MASK[c]
            for c in callees:                # callees returning a guest pointer: a record-zone pointer
                if c in CDGEN_PTR_RET:
                    x = gv.xs32(x)
                    rets[c] = ZONE + (x % 0x400) * 4
            if table is CDGEN33 and entry == 0x8007F72C:   # discriminators: == 1 vs low bits; signed > 0 vs nonzero / low-half tests
                x = gv.xs32(x)
                if x & 1:
                    rets[0x8007FBF0] = (1, 1, 1, 0x101, 0x10001, 0x80000001, 0x100, 0x1000001)[(x >> 8) % 8]
                x = gv.xs32(x)
                if x & 1:
                    rets[0x8007F778] = (0, 1, 2, 0x10000, 0x100, 0x7FFFFFFF, 0x80000000, 0xFFFFFFFF, 0x10001)[(x >> 8) % 9]
            if entry == 0x80080F98:          # timeout base 0x8009B6C4 near VSync(-1) - 1200
                x = gv.xs32(x)
                d = (0, 1, 0xFFFFFFFF, 2, 0x80000000, gv.xs32(x))[x % 6]
                struct.pack_into('<I', ram, 0x9B6C4, (rets[0x80073A44] - 1200 + d) & 0xFFFFFFFF)
            UNFAITHFUL[0] = False
            try:
                stops = {c: None for c in callees}
                if entry in CDGEN_CB_WORDS:  # the alias is stopped and recorded like the sentinel
                    stops[CDGEN_CB_WORDS[entry][1] & 0x1FFFFFFF] = None
                if table is CDGEN18:         # the object's +0x14 hook: sentinel or its alias
                    stops[CARD_CB1 & 0x1FFFFFFF] = None
                if table is CDGEN16:         # the read poll: positive k times, then a final result
                    x = gv.xs32(x)
                    k, sp_, sf = x % 6, (x >> 8) % len(SEQ_POS), (x >> 12) % len(SEQ_FINAL)
                    rets[0x800811E4] = k | sp_ << 8 | sf << 12
                    stops[0x800811E4] = _poll_model(k, SEQ_POS[sp_], SEQ_FINAL[sf])
                if table is CDGEN22:         # the timeout check: 0 for j calls, then a final value
                    x = gv.xs32(x)
                    j, f = x % 10, (x >> 8) % len(WAIT_FINAL)
                    rets[0x80084FE4] = j | f << 8
                    stops[0x80084FE4] = _poll_model(j, 0, WAIT_FINAL[f])
                if table is CDGEN23:         # sequences with callee memory writes; slot aliases
                    x = _card23_models(x, rets, stops)
                if table is CDGEN24:         # step table entries and the error hook may rewrite the index word
                    x = _card24_models(x, rets, stops)
                if table is CDGEN25:         # callees advance / rewrite the record words (absolute)
                    x = _card25_models(x, rets, stops)
                if table is CDGEN26:         # reset slot / hook / recv step may rewrite obj+0x46 / +0x36
                    x = _card26_models(x, rets, stops)
                if table is CDGEN36:
                    x = _rd36_models(entry, x, rets, stops, ram, seed)
                if table is CDGEN28:         # position model; callback / text_490 rewrite the block (absolute)
                    x = _ds28_models(x, rets, stops)
                if table is CDGEN39:         # flush: queue push may rewrite the count word; sync: lane sequence, pop rewrite
                    x = (_ds39_models_flush(x, rets, stops) if entry == 0x8007E704 else _ds39_models_sync(x, rets, stops))
                if table is CDGEN38:         # DsSystemStatus sequence, lane / packet draws, 0x800B28F8 wait model
                    x = _ds38_models(x, rets, stops)
                if table is CDGEN33 and entry == 0x8007C13C:   # acknowledge worker: POS33[p] k times, then FIN33[f]
                    x = gv.xs32(x)
                    k, p_, f_ = x % 6, (x >> 8) % len(POS33), (x >> 12) % len(FIN33)
                    x = gv.xs32(x)
                    w_ = x % 12 if x % 12 < 8 else 8
                    rets[0x8007AAB4] = k | p_ << 4 | f_ << 8 | w_ << 12
                    stops[0x8007AAB4] = _ack33_model(rets[0x8007AAB4])
                    for _word, sent in CD33_CB:      # callbacks are stopped; their results are ignored
                        stops[sent] = None
                        stops[sent & 0x1FFFFFFF] = None
                        rets[sent] = rets[sent & 0x1FFFFFFF] = gv.xs32(x)
                if table is CDGEN34:         # every callee may make one absolute write (its RETS code)
                    x = _pad34_models(x, rets, stops, ram, args)
                if table is CDGEN35:         # the two slots return scripted values and may rewrite later reads
                    x = _sio35_models(x, rets, stops, ram, args)
                if table is CDGEN21:         # the timeout check: 0 for j calls, then a final value
                    x = gv.xs32(x)
                    j, f = x % 10, (x >> 8) % len(WAIT_FINAL)
                    rets[0x80084FE4] = j | f << 8
                    stops[0x80084FE4] = _poll_model(j, 0, WAIT_FINAL[f])
                if table is CDGEN20:         # slot aliases are stopped and return the sentinel's value
                    for word, sent in CDGEN20_SLOTS:
                        if sent in rets:
                            stops[sent & 0x1FFFFFFF] = None
                            rets[sent & 0x1FFFFFFF] = rets[sent]
                if table is CDGEN19:         # strcmp: 0 for one name pointer (pool entry or none)
                    x = gv.xs32(x)
                    match = ff_pool[x % 4] if x % 5 else ZONE + 0x1FF0
                    rets[0x80072A54] = match
                    stops[0x80072A54] = _ff_strcmp_model(match)
                    if FF_HOOK in rets:          # the alias returns the same scripted value
                        rets[FF_HOOK & 0x1FFFFFFF] = rets[FF_HOOK]
                    LOWRAM_ON[0] = True
                try:
                    res = run_with_callees(ram, entry, args, rets, stops)
                finally:
                    LOWRAM_ON[0] = False
                    MEMSEQ.clear()
                    SIO_ACK[0] = None
                    REGBITS.clear()
                    REGCOUNT.clear()
                    SIO35_SEQ.clear()
            except AssertionError as e:
                if 'instruction budget' not in str(e):
                    raise
                CDGEN_SPUN[entry] = CDGEN_SPUN.get(entry, 0) + 1
                continue
            if UNFAITHFUL[0]:
                continue
            ret, loads, stores, calls = res
            call_text = ' '.join(','.join('%08X' % y for y in rec) + '@%d' % m
                                 for rec, blob, m in calls) or '-'
            out.append(gv.line(entry, s, 0, args, (ret, loads, stores)) +
                       ' RETS %s CALL %s' % (','.join('%X' % rets[c] for c in callees) or '0', call_text))
    return out


def errmath_cases(base):
    out = []
    edges = (0, 1, 2, 0x11, 0x21, 0x22, 0x7F, 0x80, 0xFF, 0x100, 0x7FFF, 0x8000,
             0xFFFF, 0x10000, 0x7FFFFFFF, 0x80000000, 0xFFFFFFFF)
    for seed in range(1, 6001):
        s = seed * 7919
        ram, x, ring, size = prep(base, s)
        args = []
        for _ in range(2):
            x = gv.xs32(x)
            k = x % 4
            args.append(x % 0x40 if k == 0 else edges[(x >> 8) % len(edges)] if k == 1
                        else gv.xs32(x) if k == 2 else (x >> 4) % 0x1000)
        if seed > 4000:                      # discriminators: a0 near the delivering codes
            x = gv.xs32(x)
            lo = 0x18 + x % 0x18
            hi = (0, 0, 0x100, 0x10000, 0x80000000, 0xFFFFFF00, x & 0xFFFFFF00)[(x >> 8) % 7]
            args[0] = (hi | lo) & 0xFFFFFFFF
        x = gv.xs32(x)
        rets = {0x80073A34: (0, 1, 0xFFFFFFFF, x)[(x >> 3) % 4]}
        UNFAITHFUL[0] = False
        res = run_with_callees(ram, 0x800739C4, args, rets, {0x80073A34: None})
        if UNFAITHFUL[0]:
            continue
        ret, loads, stores, calls = res
        call_text = ' '.join(','.join('%08X' % y for y in rec) + '@%d' % n
                             for rec, blob, n in calls) or '-'
        out.append(gv.line(0x800739C4, s, 0, args, (ret, loads, stores)) +
                   ' RETS %X CALL %s' % (rets[0x80073A34], call_text))
    return out


def spucallee_cases(base):
    out = []
    for entry in (0x8007DAE0, 0x80085BB4):
        for seed in range(1, 5001):
            s = seed * 7919
            ram, x, ring, size = prep(base, s)
            x = gv.xs32(x)
            shift = 3 if x % 3 else (x >> 4) % 5
            if seed > 4000:                  # discriminators: wide shift words
                shift = (5, 15, 16, 31, 32, 33, 0xFFFFFFFF)[x % 7]
            struct.pack_into('<I', ram, 0x9B424, shift)
            x = gv.xs32(x)
            lst = 0x80101000 + (x % 0x400) // 8 * 8
            struct.pack_into('<I', ram, 0x9B464, lst)
            n = 1 + (x >> 12) % 8
            spans = []
            for i in range(n):
                x = gv.xs32(x)
                start = (x >> 8) & 0xFFFFF
                x = gv.xs32(x)
                length = (x >> 16) & 0x3FFF
                flags = (0x40000000 if x & 1 else 0) | (0x80000000 if i == n - 1 else 0)
                struct.pack_into('<II', ram, (lst & 0x1FFFFF) + 8 * i, start | flags, length)
                spans.append((start, length))
            x = gv.xs32(x)
            if entry == 0x8007DAE0:
                args = [(x >> 4) % 0x200, gv.xs32(x), (0, 1, x, 0x10000, 0xFFFF0000)[x % 5]]
            else:
                st, ln = spans[(x >> 3) % n]
                sh = shift
                cand = [(st >> sh) + d for d in (-1, 0, 1)] + [((st + ln) >> sh) + d for d in (-1, 0, 1)]
                args = [cand[(x >> 8) % len(cand)] & 0xFFFFFFFF if x % 4 else gv.xs32(x)]
            UNFAITHFUL[0] = False
            res = gv.run(ram, entry, args)
            if not UNFAITHFUL[0]:
                out.append(gv.line(entry, s, 0, args, res))
    return out


def main():
    if sys.argv[2:] == ['spucallees']:
        exe = (v.ROOT / 'build/disc1.candidate.exe').read_bytes()
        out = spucallee_cases(bytes(v.fixture(exe, 0, 0, 0)))
        open(sys.argv[1], 'w').write('\n'.join(out) + '\n')
        print('cases', len(out))
        return
    if sys.argv[2:] == ['spureverb']:
        exe = (v.ROOT / 'build/disc1.candidate.exe').read_bytes()
        out = spureverb_cases(bytes(v.fixture(exe, 0, 0, 0)))
        open(sys.argv[1], 'w').write('\n'.join(out) + '\n')
        print('cases', len(out))
        return
    if sys.argv[2:] == ['fidma']:
        exe = (v.ROOT / 'build/disc1.candidate.exe').read_bytes()
        out = fidma_cases(bytes(v.fixture(exe, 0, 0, 0)))
        open(sys.argv[1], 'w').write('\n'.join(out) + '\n')
        print('cases', len(out))
        return
    if sys.argv[2:] == ['cardclear']:
        exe = (v.ROOT / 'build/disc1.candidate.exe').read_bytes()
        out = cardclear_cases(bytes(v.fixture(exe, 0, 0, 0)))
        open(sys.argv[1], 'w').write('\n'.join(out) + '\n')
        print('cases', len(out))
        return
    if sys.argv[2:] == ['qsort']:
        exe = (v.ROOT / 'build/disc1.candidate.exe').read_bytes()
        out = qsort_cases(bytes(v.fixture(exe, 0, 0, 0)))
        open(sys.argv[1], 'w').write('\n'.join(out) + '\n')
        print('cases', len(out))
        return
    if sys.argv[2:] == ['cdgen34']:
        exe = (v.ROOT / 'build/disc1.candidate.exe').read_bytes()
        out = cdgen_cases(bytes(v.fixture(exe, 0, 0, 0)), table=CDGEN34, indep=True, sweep=True)
        open(sys.argv[1], 'w').write('\n'.join(out) + '\n')
        print('cases', len(out), 'spun', CDGEN_SPUN)
        return
    if sys.argv[2:3] == ['cdgen38']:
        exe = (v.ROOT / 'build/disc1.candidate.exe').read_bytes()
        out = cdgen_cases(bytes(v.fixture(exe, 0, 0, 0)), table=CDGEN38, indep=True, sweep=True)
        open(sys.argv[1], 'w').write('\n'.join(out) + '\n')
        print('cases', len(out), 'spun', CDGEN_SPUN)
        return
    if sys.argv[2:3] == ['cdgen39']:
        exe = (v.ROOT / 'build/disc1.candidate.exe').read_bytes()
        out = cdgen_cases(bytes(v.fixture(exe, 0, 0, 0)), table=CDGEN39, indep=True, sweep=True)
        open(sys.argv[1], 'w').write('\n'.join(out) + '\n')
        print('cases', len(out), 'spun', CDGEN_SPUN)
        return
    if sys.argv[2:3] == ['cdgen33']:      # optional entries: per-entry output is independent of the others
        exe = (v.ROOT / 'build/disc1.candidate.exe').read_bytes()
        out = cdgen_cases(bytes(v.fixture(exe, 0, 0, 0)), entries=[int(a, 16) for a in sys.argv[3:]] or None,
                          table=CDGEN33, indep=True, sweep=True)
        open(sys.argv[1], 'w').write('\n'.join(out) + '\n')
        print('cases', len(out), 'spun', CDGEN_SPUN)
        return
    if sys.argv[2:] == ['cdgen35']:
        exe = (v.ROOT / 'build/disc1.candidate.exe').read_bytes()
        out = cdgen_cases(bytes(v.fixture(exe, 0, 0, 0)), table=CDGEN35, indep=True, sweep=True)
        open(sys.argv[1], 'w').write('\n'.join(out) + '\n')
        print('cases', len(out), 'spun', CDGEN_SPUN)
        return
    if sys.argv[2:3] == ['cdgen36']:
        # usage: OUT.txt cdgen36 [N [ENTRY]]; without ENTRY the two entries run as child processes
        # (CD_ready first) and their outputs are concatenated.
        n = int(sys.argv[3]) if len(sys.argv) > 3 else 6000
        if len(sys.argv) > 4:
            exe = (v.ROOT / 'build/disc1.candidate.exe').read_bytes()
            CALL_CAP[0] = 700
            ent = int(sys.argv[4], 16)
            out = cdgen_cases(bytes(v.fixture(exe, 0, 0, 0)), entries=[ent], table=CDGEN36, indep=True, n=n)
            open(sys.argv[1], 'w').write('\n'.join(out) + '\n')
            print('cases', len(out), 'spun', CDGEN_SPUN)
            return
        import subprocess
        procs = [(e, subprocess.Popen([sys.executable, sys.argv[0], sys.argv[1] + '.' + e, 'cdgen36', str(n), e]))
                 for e in ('8007B290', '80081110')]
        for e, pr in procs:
            if pr.wait():
                raise SystemExit('cdgen36 child %s failed' % e)
        with open(sys.argv[1], 'w') as o:
            for e, pr in procs:
                o.write(open(sys.argv[1] + '.' + e).read())
                os.remove(sys.argv[1] + '.' + e)
        return
    if sys.argv[2:] == ['cdgen32']:
        exe = (v.ROOT / 'build/disc1.candidate.exe').read_bytes()
        out = cdgen_cases(bytes(v.fixture(exe, 0, 0, 0)), table=CDGEN32, indep=True, sweep=True)
        open(sys.argv[1], 'w').write('\n'.join(out) + '\n')
        print('cases', len(out), 'spun', CDGEN_SPUN)
        return
    if sys.argv[2:] == ['cdgen31']:
        exe = (v.ROOT / 'build/disc1.candidate.exe').read_bytes()
        out = cdgen_cases(bytes(v.fixture(exe, 0, 0, 0)), table=CDGEN31, indep=True, sweep=True)
        open(sys.argv[1], 'w').write('\n'.join(out) + '\n')
        print('cases', len(out), 'spun', CDGEN_SPUN)
        return
    if sys.argv[2:] == ['cdgen30']:
        exe = (v.ROOT / 'build/disc1.candidate.exe').read_bytes()
        out = cdgen_cases(bytes(v.fixture(exe, 0, 0, 0)), table=CDGEN30, indep=True, sweep=True)
        open(sys.argv[1], 'w').write('\n'.join(out) + '\n')
        print('cases', len(out), 'spun', CDGEN_SPUN)
        return
    if sys.argv[2:] == ['cdgen29']:
        exe = (v.ROOT / 'build/disc1.candidate.exe').read_bytes()
        out = cdgen_cases(bytes(v.fixture(exe, 0, 0, 0)), table=CDGEN29, indep=True, sweep=True)
        open(sys.argv[1], 'w').write('\n'.join(out) + '\n')
        print('cases', len(out), 'spun', CDGEN_SPUN)
        return
    if sys.argv[2:] == ['cdgen28']:
        exe = (v.ROOT / 'build/disc1.candidate.exe').read_bytes()
        out = cdgen_cases(bytes(v.fixture(exe, 0, 0, 0)), table=CDGEN28, indep=True, sweep=True)
        open(sys.argv[1], 'w').write('\n'.join(out) + '\n')
        print('cases', len(out), 'spun', CDGEN_SPUN)
        return
    if sys.argv[2:] == ['cdgen26']:
        exe = (v.ROOT / 'build/disc1.candidate.exe').read_bytes()
        out = cdgen_cases(bytes(v.fixture(exe, 0, 0, 0)), table=CDGEN26, indep=True, sweep=True)
        open(sys.argv[1], 'w').write('\n'.join(out) + '\n')
        print('cases', len(out), 'spun', CDGEN_SPUN)
        return
    if sys.argv[2:] == ['cdgen25']:
        exe = (v.ROOT / 'build/disc1.candidate.exe').read_bytes()
        out = cdgen_cases(bytes(v.fixture(exe, 0, 0, 0)), table=CDGEN25, indep=True, sweep=True)
        open(sys.argv[1], 'w').write('\n'.join(out) + '\n')
        print('cases', len(out), 'spun', CDGEN_SPUN)
        return
    if sys.argv[2:] == ['cdgen24']:
        exe = (v.ROOT / 'build/disc1.candidate.exe').read_bytes()
        out = cdgen_cases(bytes(v.fixture(exe, 0, 0, 0)), table=CDGEN24, indep=True, sweep=True)
        open(sys.argv[1], 'w').write('\n'.join(out) + '\n')
        print('cases', len(out), 'spun', CDGEN_SPUN)
        return
    if sys.argv[2:] == ['cdgen23']:
        exe = (v.ROOT / 'build/disc1.candidate.exe').read_bytes()
        out = cdgen_cases(bytes(v.fixture(exe, 0, 0, 0)), table=CDGEN23, indep=True, sweep=True)
        open(sys.argv[1], 'w').write('\n'.join(out) + '\n')
        print('cases', len(out), 'spun', CDGEN_SPUN)
        return
    if sys.argv[2:] == ['cdgen22']:
        exe = (v.ROOT / 'build/disc1.candidate.exe').read_bytes()
        out = cdgen_cases(bytes(v.fixture(exe, 0, 0, 0)), table=CDGEN22, indep=True, sweep=True)
        open(sys.argv[1], 'w').write('\n'.join(out) + '\n')
        print('cases', len(out), 'spun', CDGEN_SPUN)
        return
    if sys.argv[2:] == ['cdgen21']:
        exe = (v.ROOT / 'build/disc1.candidate.exe').read_bytes()
        out = cdgen_cases(bytes(v.fixture(exe, 0, 0, 0)), table=CDGEN21, indep=True, sweep=True)
        open(sys.argv[1], 'w').write('\n'.join(out) + '\n')
        print('cases', len(out), 'spun', CDGEN_SPUN)
        return
    if sys.argv[2:] == ['cdgen20']:
        exe = (v.ROOT / 'build/disc1.candidate.exe').read_bytes()
        out = cdgen_cases(bytes(v.fixture(exe, 0, 0, 0)), table=CDGEN20, indep=True, sweep=True)
        open(sys.argv[1], 'w').write('\n'.join(out) + '\n')
        print('cases', len(out), 'spun', CDGEN_SPUN)
        return
    if sys.argv[2:] == ['cdgen19']:
        exe = (v.ROOT / 'build/disc1.candidate.exe').read_bytes()
        out = cdgen_cases(bytes(v.fixture(exe, 0, 0, 0)), table=CDGEN19, indep=True, sweep=True)
        open(sys.argv[1], 'w').write('\n'.join(out) + '\n')
        print('cases', len(out), 'spun', CDGEN_SPUN)
        return
    if sys.argv[2:] == ['cdgen18']:
        exe = (v.ROOT / 'build/disc1.candidate.exe').read_bytes()
        out = cdgen_cases(bytes(v.fixture(exe, 0, 0, 0)), table=CDGEN18, indep=True, sweep=True)
        open(sys.argv[1], 'w').write('\n'.join(out) + '\n')
        print('cases', len(out), 'spun', CDGEN_SPUN)
        return
    if sys.argv[2:] == ['cdgen17']:
        exe = (v.ROOT / 'build/disc1.candidate.exe').read_bytes()
        out = cdgen_cases(bytes(v.fixture(exe, 0, 0, 0)), table=CDGEN17, indep=True, sweep=True)
        open(sys.argv[1], 'w').write('\n'.join(out) + '\n')
        print('cases', len(out), 'spun', CDGEN_SPUN)
        return
    if sys.argv[2:] == ['cdgen16']:
        exe = (v.ROOT / 'build/disc1.candidate.exe').read_bytes()
        out = cdgen_cases(bytes(v.fixture(exe, 0, 0, 0)), table=CDGEN16, indep=True, sweep=True)
        open(sys.argv[1], 'w').write('\n'.join(out) + '\n')
        print('cases', len(out), 'spun', CDGEN_SPUN)
        return
    if sys.argv[2:] == ['cdgen15']:
        exe = (v.ROOT / 'build/disc1.candidate.exe').read_bytes()
        out = cdgen_cases(bytes(v.fixture(exe, 0, 0, 0)), table=CDGEN15, n=2000, indep=True, sweep=True)
        open(sys.argv[1], 'w').write('\n'.join(out) + '\n')
        print('cases', len(out), 'spun', CDGEN_SPUN)
        return
    if sys.argv[2:] == ['cdgen14']:
        exe = (v.ROOT / 'build/disc1.candidate.exe').read_bytes()
        out = cdgen_cases(bytes(v.fixture(exe, 0, 0, 0)), table=CDGEN14, indep=True, sweep=True)
        open(sys.argv[1], 'w').write('\n'.join(out) + '\n')
        print('cases', len(out), 'spun', CDGEN_SPUN)
        return
    if sys.argv[2:] == ['cdgen13']:
        exe = (v.ROOT / 'build/disc1.candidate.exe').read_bytes()
        out = cdgen_cases(bytes(v.fixture(exe, 0, 0, 0)), table=CDGEN13, indep=True, sweep=True)
        open(sys.argv[1], 'w').write('\n'.join(out) + '\n')
        print('cases', len(out), 'spun', CDGEN_SPUN)
        return
    if sys.argv[2:] == ['cdgen12']:
        exe = (v.ROOT / 'build/disc1.candidate.exe').read_bytes()
        out = cdgen_cases(bytes(v.fixture(exe, 0, 0, 0)), table=CDGEN12, indep=True, sweep=True)
        open(sys.argv[1], 'w').write('\n'.join(out) + '\n')
        print('cases', len(out), 'spun', CDGEN_SPUN)
        return
    if sys.argv[2:] == ['cdgen11']:
        exe = (v.ROOT / 'build/disc1.candidate.exe').read_bytes()
        out = cdgen_cases(bytes(v.fixture(exe, 0, 0, 0)), table=CDGEN11, indep=True, sweep=True)
        open(sys.argv[1], 'w').write('\n'.join(out) + '\n')
        print('cases', len(out), 'spun', CDGEN_SPUN)
        return
    if sys.argv[2:] == ['cdgen10']:
        exe = (v.ROOT / 'build/disc1.candidate.exe').read_bytes()
        out = cdgen_cases(bytes(v.fixture(exe, 0, 0, 0)), table=CDGEN10, indep=True, sweep=True)
        open(sys.argv[1], 'w').write('\n'.join(out) + '\n')
        print('cases', len(out), 'spun', CDGEN_SPUN)
        return
    if sys.argv[2:] == ['cdgen9']:
        exe = (v.ROOT / 'build/disc1.candidate.exe').read_bytes()
        out = cdgen_cases(bytes(v.fixture(exe, 0, 0, 0)), table=CDGEN9, indep=True, sweep=True)
        open(sys.argv[1], 'w').write('\n'.join(out) + '\n')
        print('cases', len(out), 'spun', CDGEN_SPUN)
        return
    if sys.argv[2:] in (['cdgen8'], ['cdgen8', 'sweep']):
        exe = (v.ROOT / 'build/disc1.candidate.exe').read_bytes()
        out = cdgen_cases(bytes(v.fixture(exe, 0, 0, 0)), table=CDGEN8, indep=True,
                          sweep=sys.argv[3:] == ['sweep'])
        open(sys.argv[1], 'w').write('\n'.join(out) + '\n')
        print('cases', len(out), 'spun', CDGEN_SPUN)
        return
    if sys.argv[2:] == ['cdgen7']:
        exe = (v.ROOT / 'build/disc1.candidate.exe').read_bytes()
        out = cdgen_cases(bytes(v.fixture(exe, 0, 0, 0)), table=CDGEN7, indep=True)
        open(sys.argv[1], 'w').write('\n'.join(out) + '\n')
        print('cases', len(out), 'spun', CDGEN_SPUN)
        return
    if sys.argv[2:] in (['cdgen6'], ['cdgen-indep'], ['cdgen-sweep']):
        exe = (v.ROOT / 'build/disc1.candidate.exe').read_bytes()
        if sys.argv[2:] != ['cdgen6']:   # cross-check: every cdgen table, independent (and swept) returns
            out = []
            for t in (CDGEN, CDGEN2, CDGEN3, CDGEN5, CDGEN6, CDGEN7, CDGEN8)[:6 if sys.argv[2:] == ['cdgen-indep'] else 7]:
                out += cdgen_cases(bytes(v.fixture(exe, 0, 0, 0)), table=t, n=3000, indep=True,
                                   sweep=sys.argv[2:] == ['cdgen-sweep'])
        else:
            out = cdgen_cases(bytes(v.fixture(exe, 0, 0, 0)), table=CDGEN6, indep=True)
        open(sys.argv[1], 'w').write('\n'.join(out) + '\n')
        print('cases', len(out), 'spun', CDGEN_SPUN)
        return
    if sys.argv[2:] == ['cdgen5']:
        exe = (v.ROOT / 'build/disc1.candidate.exe').read_bytes()
        out = cdgen_cases(bytes(v.fixture(exe, 0, 0, 0)), table=CDGEN5, indep=True)
        open(sys.argv[1], 'w').write('\n'.join(out) + '\n')
        print('cases', len(out), 'spun', CDGEN_SPUN)
        return
    if sys.argv[2:] == ['cdgen3']:
        exe = (v.ROOT / 'build/disc1.candidate.exe').read_bytes()
        out = cdgen_cases(bytes(v.fixture(exe, 0, 0, 0)), table=CDGEN3)
        open(sys.argv[1], 'w').write('\n'.join(out) + '\n')
        print('cases', len(out), 'spun', CDGEN_SPUN)
        return
    if sys.argv[2:] == ['cdgen2']:
        exe = (v.ROOT / 'build/disc1.candidate.exe').read_bytes()
        out = cdgen_cases(bytes(v.fixture(exe, 0, 0, 0)), table=CDGEN2)
        open(sys.argv[1], 'w').write('\n'.join(out) + '\n')
        print('cases', len(out), 'spun', CDGEN_SPUN)
        return
    if sys.argv[2:] == ['cdgen']:
        exe = (v.ROOT / 'build/disc1.candidate.exe').read_bytes()
        out = cdgen_cases(bytes(v.fixture(exe, 0, 0, 0)))
        open(sys.argv[1], 'w').write('\n'.join(out) + '\n')
        print('cases', len(out), 'spun', CDGEN_SPUN)
        return
    if sys.argv[2:] == ['errmath']:
        exe = (v.ROOT / 'build/disc1.candidate.exe').read_bytes()
        out = errmath_cases(bytes(v.fixture(exe, 0, 0, 0)))
        open(sys.argv[1], 'w').write('\n'.join(out) + '\n')
        print('cases', len(out))
        return
    if sys.argv[2:] == ['modetype']:
        exe = (v.ROOT / 'build/disc1.candidate.exe').read_bytes()
        out = modetype_cases(bytes(v.fixture(exe, 0, 0, 0)))
        open(sys.argv[1], 'w').write('\n'.join(out) + '\n')
        print('cases', len(out))
        return
    if sys.argv[2:] == ['reverb']:
        exe = (v.ROOT / 'build/disc1.candidate.exe').read_bytes()
        out = reverb_cases(bytes(v.fixture(exe, 0, 0, 0)))
        open(sys.argv[1], 'w').write('\n'.join(out) + '\n')
        print('cases', len(out))
        return
    if sys.argv[2:] == ['callback']:
        exe = (v.ROOT / 'build/disc1.candidate.exe').read_bytes()
        out, spun = callback_cases(bytes(v.fixture(exe, 0, 0, 0)))
        open(sys.argv[1], 'w').write('\n'.join(out) + '\n')
        print('cases', len(out), 'skipped (budget)', spun)
        return
    if sys.argv[2:] == ['flush']:
        exe = (v.ROOT / 'build/disc1.candidate.exe').read_bytes()
        out, spun = flush_cases(bytes(v.fixture(exe, 0, 0, 0)))
        open(sys.argv[1], 'w').write('\n'.join(out) + '\n')
        print('cases', len(out), 'skipped (budget)', spun)
        return
    entries = [int(a, 16) for a in sys.argv[2:]] or ENTRIES
    exe = (v.ROOT / 'build/disc1.candidate.exe').read_bytes()
    base = bytes(v.fixture(exe, 0, 0, 0))
    out, spun, dropped = [], 0, 0
    for entry in entries:
        for seed in range(1, 6001):
            s = seed * 7919
            ram, x, ring, size = prep(base, s)
            x = gv.xs32(x)
            SIO_RX[0] = None
            if entry == 0x80083578:                     # SIO0 RX-ready poll through D_8009B788
                args = []
                x = gv.xs32(x)
                struct.pack_into('<H', ram, 0x1044, (x >> 8) & 0xFFFF)
                x = gv.xs32(x)
                SIO_RX[0], SIO_RX[1] = x % 7, 0
            elif entry in (0x80084FC4, 0x80084FE4):       # libpad RCNT2 wait pair
                x = gv.xs32(x)
                cnt = (x >> 8) & 0xFFFF
                struct.pack_into('<H', ram, 0x1120, cnt)
                x = gv.xs32(x)
                struct.pack_into('<H', ram, 0x1124, ((x >> 8) & 0xFFFF) ^ (0x200 if x & 1 else 0))
                x = gv.xs32(x)
                struct.pack_into('<H', ram, 0x1128, (0, 0xFFFF, 0x8000, 0x1000, (x >> 8) & 0xFFFF)[x % 5])
                x = gv.xs32(x)
                k = x % 8                                 # start word 0x800A76D0
                start = cnt if k == 0 else (cnt + 1 + (x >> 8) % 0x400) & 0xFFFF if k == 1 \
                    else gv.xs32(x) if k == 2 else (cnt - (x >> 8) % 0x2000) & 0xFFFF
                struct.pack_into('<I', ram, 0xA76D0, start)
                x = gv.xs32(x)
                el = (cnt - start) & 0xFFFF
                lim = (0, 1, 0x3B, 0x3C, 0x3D, 0x100, 0xFFFF, 0x10000, 0x7FFFFFFF, 0xFFFFFFFF,
                       el, el + 1, max(el - 1, 0), el >> 3, (el >> 3) + 1, max((el >> 3) - 1, 0))[x % 16] \
                    if x & 0x100 else (x >> 12) % 0x200   # limit word 0x800BD02C
                struct.pack_into('<I', ram, 0xBD02C, lim & 0xFFFFFFFF)
                args = [lim & 0xFFFFFFFF] if entry == 0x80084FC4 else []
            elif entry in (0x80082ADC, 0x80082CDC):       # libcard leaves without arguments
                args = []
                x = gv.xs32(x)
                struct.pack_into('<I', ram, 0x9B78C, (0, 1, 2, 0x7FFFFFFF, 0x80000000, 0xFFFFFFFF, x)[x % 7])
            elif entry == 0x8007B964:
                args = [gv.ptr(x)]
            elif entry == 0x8007BAC0:
                args = []
            elif entry == 0x8007C394:
                if x % 4:
                    args = [ring + 32 * ((x >> 4) % (size + 1)) + 4 * ((x >> 12) % 8 if x & 0x10000 else 0)]
                else:
                    args = [gv.ptr(x >> 4)]
            else:
                args = [gv.ptr(x), gv.ptr(gv.xs32(x))]
            UNFAITHFUL[0] = False
            try:
                res = gv.run(ram, entry, args)
            except AssertionError as e:
                if 'instruction budget' not in str(e):
                    raise
                spun += 1
                continue
            if UNFAITHFUL[0]:
                dropped += 1
                continue
            out.append(gv.line(entry, s, 0, args, res))
    open(sys.argv[1], 'w').write('\n'.join(out) + '\n')
    print('cases', len(out), 'skipped (budget)', spun, 'dropped (unfaithful)', dropped)


if __name__ == '__main__':
    main()
