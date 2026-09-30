# Parasite Eve Native PC Port — Phase 6E-B54K-Q

**Current route verification (2026-09-13):** The restored battle reward
screen now renders and processes loot. A shared host/RAM state bug is fixed:
normal cold boot commits 2 XP and collects 6 ammunition in m0013i. The loot
functions and item-base initialization pass 198 original/native comparisons,
plus full-RAM comparisons from the corrected connected reward capture.

The old fixed controller sequence reaches the first sewer battle but loses it;
its previous 62,000-frame supply result bypassed rewards and remains historical.
An adaptive input replay defeats that encounter using the normal baton switch,
completes level-up and loot, and restores field control at frame 52,111 with
Aya at 27 HP. It then completes the second sewer battle and reaches the sewer
junction at frame 54,520 with 33 HP and 16 XP. Level drawing passes 183
original/native cases and an isolated 80-frame animation/confirmation
comparison. The complete victory handler now passes 83 original/native cases
plus a captured-phase comparison. The corrected input replay wins M32's
encounter at frame 57,791 with 35 HP, opens its gate and enters M33 at 58,538.
Two restored task pause/resume handlers pass 518 original/native cases and
a captured sequence; the connected replay completes M33's scene and reaches
the M34 boss encounter. Restored message opcode D1 passes 160 original/native
cases plus its captured call; the replay starts the fight and reaches frame
60,271. Effect 8's lifecycle now passes 177 original/native cases and its
captured constructor call; the replay advances within that frame to the
projectile initializer (`8018F434`). That initializer is now translated and
passes 252 original/native cases plus four captured-context variants, with
its six retained stack values supplied explicitly. Native dispatch now tracks
the original sound, VM, command and drawing writes that supply those values.
Five original/native histories match the retained inputs and complete records
of the first two projectiles, including partial stack writes between them.
An ordinary-input cold boot executes the five-projectile burst; the current
controller then loses the fight at frame60,533 and resets to the opening.
The complete fight still requires verification;
whole-frame differences remain outside the initializer proof.
The field counter and main-loop destination snapshot now share their original
guest-RAM words with all readers.
Projectile
movement, trail allocation, drawing and collision now pass 303 original/native
comparisons, including complete non-stack RAM, GPU packets and scratchpad.
All 1,398 native tests and all 10 non-route CTest checks pass.
The full Day 2 route is incomplete.

The rebuilt PSX executable remains byte-identical to retail.
**Whole-route retail fidelity remains unproved.**
[M34 retained stack writers](../docs/ai_context/M34_RETAINED_STACK_WRITERS.md).
[M34 native stack binding](../docs/ai_context/M34_STACK_BINDING.md).
[M34 continuous frame stack evidence](../docs/ai_context/M34_FRAME_STACK_PRESERVATION.md).
[Destination snapshot RAM repair](../docs/ai_context/DESTINATION_SNAPSHOT_RAM.md).
[Guest frame-counter repair](../docs/ai_context/FIELD_FRAME_COUNTER_RAM.md).
[M34 projectile movement, drawing and collision](../docs/ai_context/M34_PROJECTILE_CHILDREN.md).
[M34 projectile initializer and stack evidence](../docs/ai_context/M34_PROJECTILE_INITIALIZER.md).
[M34 effect lifecycle and projectile dependency](../docs/ai_context/M34_BOSS_EFFECT.md).
[M34 message and effect boundary](../docs/ai_context/MESSAGE_WINDOW_D1.md).
[Task suspension and M33 progress](../docs/ai_context/SCRIPT_TASK_PAUSE.md).
[Victory handler restoration](../docs/ai_context/BATTLE_VICTORY_HANDLER.md).
[Level animation evidence](../docs/ai_context/BATTLE_LEVEL_ANIMATION.md).
[Current loot evidence](../docs/ai_context/BATTLE_LOOT_SCREEN.md).
[Reward progression](../docs/ai_context/BATTLE_REWARD_PROGRESSION.md).
[Historical sewer evidence](../docs/ai_context/DAY1_SEWER_MOVEMENT.md).

Historical update:

DAY1-13 restores the original hit-origin and particle/spark initialization
callbacks:90 original/native cases pass, including weapon-VM creation and
sound requests. Normal and sanitizer CTest each5/5; Windows crossbuild passes.
Hit drawing, the fresh rehearsal replay and fullDay1 remain unfinished.


**Goal:** Retail-accurate native PC port of Parasite Eve (PSX, NTSC-U SLUS-006.62).

**DAY1-12 update (2026-09-06):** Restored script36 actor tracking and
approach, reached during the rehearsal fight. 840 original/native steps and
captured-state handler/VM comparisons pass. Normal and sanitizer suites
pass5/5; Windows cross-build passes. A fresh live replay of this command
remains necessary. Full Day1 and final publication are unfinished.

**DAY1-11 update (2026-09-06):** Rehearsal flashes now retain the original
shared stack values written by concurrent pistol casing and muzzle draws.
644 original/native pump frames pass in normal and sanitizer builds; a
64-frame comparison using captured rehearsal RAM matches all defined
non-stack bytes. Live run28 passed the flash boundary and reached the next script command.

**DAY1-10 update (2026-09-06):** Arrange Items now opens, draws, sorts and
returns normally. Its carried/storage sorting helpers preserve the original
quicksort behavior and equipped-item identities. All 542 original/native
comparisons pass in normal and sanitizer builds; full CTest passes 5/5 on
both. Windows runtime crossbuild passes; execution remains untested. Live
run27 verifies all three sort options and cancellation, then uses Medicine1
and continues the Day1 route. [Menu evidence](../docs/evidence/pe-day1-arrange-items/README.md).
Full Day1 accuracy is still unfinished; the launcher release remains DAY1-9.

**Working tree progress (2026-09-05):** the visible native opening now
renders the stage backdrop, all three performers and staircase. The pending
scripted camera pan that blacked out the scene and cropped the actors is
restored. Auditorium collision, stair access and the battle transition work.
Name entry supports normal letter selection, backspace, Default and End.
The first Eve fight completes through normal targeting and firing, including
reload, Eve's attacks, her scripted retreat and restored field movement.

The latest INV11 visible replay also opens the field inventory, transfers and
saves ammo, rearranges an item, then uses Medicine 1 after the fight. Aya's
live and saved HP rise from14 to45, the medicine is consumed, and closing the
menu restores movement. Seven shot events and the full stage view were
captured in that same run. Normal and ASan/UBSan CTest pass2/2 each,
1,187 native test groups through PACE1 battle-menu commands, medicine
animation/healing and its particles/rings, equipment command completion and
escape judgement, PE reservations, costs and availability, and the game-over
fade transition. The PE menu, ability application, and battle command lifecycle
are also checked against 503 original-executable cases.

The INV17 visible replay consumed Medicine 1, healed Aya from 42 to 45,
rendered its particles and rings, then completed Eve's fight with seven shots.
Eve's scripted retreat, final dialogue and restored movement were verified.
The preceding run also exposed a missing game-over fade start, now restored
and checked against the original executable; the full defeat flow still needs
a fresh visible verification.
PE menu/application/command execution are native; full Liberation presentation
and several ability effects remain unfinished. PE is excluded by the story menu
mask during the opening Eve encounter. The INV20 visible replay completed
that fight, then opened Heal 1 after PE unlocked: live and saved HP17→45,
PE80→20, with normal confirmation and return to field control.
Equipment page construction, filtering, cursor restoration, and stat comparisons
match232 original-executable cases. Ordinary weapon/armor selection,
cancellation and confirmation now match215 additional original cases and are
wired into the main menu. The INV22 visible replay completed the fight,
then equipped Club 1 and restored M84F, exercised the armor page, and
returned to normal movement. Status/config/upgrade pages remain unfinished. Audio is audible (audio lane, 2026-09-28): `platform/pe_spu.c` is a
native SPU (24 ADPCM voices, ADSR, sweeps, noise, pitch modulation, reverb,
CD/XA input), `platform/pe_audio_driver.c` runs the retail AKAO tick
`func_8008DB7C` from guest RAM in a small R3000A interpreter at the retail
240 Hz RCNT2 rate, and `platform/host_audio.c` outputs 44.1 kHz stereo via a
runtime-`dlopen`ed `libpulse-simple.so.0` (PipeWire/PulseAudio; silent if
missing). `PE_AUDIO=0` disables it; `PE_AUDIO_WAV=out.wav` dumps the mix
(headless too); `PE_AUDIO_DEVICE=1` forces the sound device in headless runs;
`PE_AUDIO_KON_LOG=path` writes a key-on timeline; `PE_AUDIO_DEBUG=1` prints
SPU state every 10 s; `PE_AUDIO_LATENCY_MS` (default 100: 60 % device buffer,
40 % queue) sets the output latency. The device feed is steered by an adaptive
resampler (at most ±0.5 %) so the 60 Hz emulated audio clock, the 59.94 Hz
pacer and the device crystal never starve or overfill it; starves fade out and
back in. Voices use the psx-spx 4-point gaussian table, XA uses the CD
decoder's 7-phase zigzag FIR behind a 133 ms jitter buffer, and reverb I/O uses
the 39-tap half-band FIR (audio2 lane). Evidence and gaps:
`../docs/evidence/pe-native-audio/REPORT.md`,
`../docs/ai_context/DAY1_FIDELITY_GAPS.md` §3.
See
`../docs/ai_context/ACTIVE_HANDOFF.md` for current status
and `../docs/evidence/pe-aya-visible-dialogue/REPORT.md` for screenshots.
The milestone and counts below describe the earlier merged baseline.

Window controls: arrows move; Enter/Space/Z/X = Cross; C = Circle;
V = Triangle; S = Square; Q/E = L1/R1; 1/3 = L2/R2;
Tab = Select; P = Start; Escape closes the window.
Linux controllers (`platform/host_pad.c`, kernel joystick API, no extra
libraries) work alongside the keyboard: A/B/X/Y = Cross/Circle/Square/Triangle,
LB/RB = L1/R1, LT/RT = L2/R2, Back = Select, Start = Start, D-pad or left stick
= directions. `/dev/input/js0..7` are rescanned about once a second (hot-plug);
`PE_JOYSTICK=/dev/input/jsN` pins one device, `PE_JOYSTICK=off` disables them,
`PE_PAD_SWAP_XY=1` swaps Square/Triangle, `PE_PAD_DEBUG=1` logs raw events.
Tested by `pe-host-pad-tests` (CTest `host-pad-input`). The owner preview
package is built by `pc_port/package_preview.sh` (no game data inside).
Resizing or maximizing scales the full game image to fit while keeping its
original 4:3 aspect ratio; unused space has black borders.
Pacing is per emulated VBlank (`PE_AudioDriver_SetVBlankPacer`): one VBlank
= 1/59.94 s of wall time, so the game's 30 fps loop (2 VBlanks per present),
its `VSync(n)` waits and the 240 Hz music clock all run at retail speed; a
present only polls input. (Pacing per present had run gameplay and music at
2x.) F6 toggles normal speed (60000/1001 Hz) and uncapped fast-forward; on a
controller L3 (left stick click) toggles it and R3 (right stick click)
fast-forwards while held. The window title shows the active mode, and the sound
device is muted with a fade while fast-forwarding (the WAV dump is not). Normal is the default. Frame waits use a monotonic
clock and poll input; long pauses discard the timing backlog.

**Metrics authority:** the merged tree generates and cross-checks current
native status from the implementation, CMake source graph, test inventory,
and evidence in `docs/generated/NATIVE_PORT_STATUS.md`. Disc-1 matching-build
status is derived from `configs/USA/disc1.yaml`.

**Current milestone:** B54K-Q translates the first 149 words of the naturally
loaded `func_801909B4` overlay. It copies four graphics environments,
publishes six arena pointers, executes three translated state owners and
`SetDispMask(0)`, then captures the exact first `MoveImage` call including
its `RECT{320,0,160,256}` payload and destination `(704,0)`. Real Disc 1 now
enters the overlay and reaches `func_8007512C` from `func_801909B4`.
Normal and fresh ASan/UBSan suites pass 962/962 with zero diagnostics.
Scheduler provenance remains independently artifact-bound: this rung adds no
destination, `m0360i`, or persistence special case. Full proof is in
`docs/evidence/pe-b54kq-1909b4-moveimage-prefix/REPORT.md`.

**Historical retained milestone:** B54K-B6 completed the L9 packet group in
`func_80030894` and independently proved its 97-byte bank-0 write map. VIS1
exposes the same authoritative 1024x512 RGB555/STP VRAM as a read-only raw or
PPM diagnostic; it is not a 320x240 rendered-display claim. Evidence remains
at `docs/evidence/pe-b54kb6-30894-l9/REPORT.md` and
`docs/evidence/pe-vis1-vram-visible-state/REPORT.md`.

**Earlier retained milestone:** B53I-D admits one separate later checkpoint
for the second LoadImage DMA token created by B53I-C. The exact 32-word
transfer from
`0x8012B8B8` exposes pixels `0x6000..0x603F` at `(256..319,456)` before
CHCR clears from `0x01000201` to `0x00000201`. Because the idle pump already
removed channel-2 callback/enable, stored and physical DICR remain
`0x00800000`: no flag 26, bit-31 rise, I_STAT source 3, CPU/DMA dispatcher,
pump, worker, or queue activity occurs. Token 2 is accepted exactly once;
replay and pre-reset tokens are rejected. Each checkpoint still captures only
one token and never recaptures; the existing host-safe continuation now makes
exactly two explicit calls, with the second admitted only after the first
returns normally, and has no loop or third opportunity. The DICR bridge and
CPU scanner are entered only for a pending sticky edge and live
`I_STAT & I_MASK`, respectively, while older pending/unmasked sources remain
independent of a new DICR edge. Eight focused groups bring
the suite to 566 tests; the independent D oracle runs 15 explicit scenarios,
and the full matrix is 46/46. Full proof is in
`docs/b53i_d_second_dma_completion.md`.

**Prior milestone:** B53I-C completes all 152 words of the libgpu queue
pump `func_80076EE4` by translating the exact 136-word idle suffix
`0x80076F10..0x8007712F`. After B53I-B2 completes the first DMA and invokes
the pump, retail saves/disables I_MASK, removes channel-2 callback identity
`0x80076EE4` only for the last/no-DrawSync item, polls the inert GPUSTAT
ready bit, and dispatches queued guest worker `0x80076664` with argument
`0x800BD03C` and source `0x8012B8B8`. The worker issues the second DMA at
`MADR/BCR/CHCR = 0x8012B8B8/0x00020010/0x01000201`; only after it returns
does the consumer advance at retail `0x80077054`. I_MASK restores to its
actual saved value (`0x0009` canonically), marker remains 1 because DMA is
busy, and the enclosing CPU dispatcher normally clears `D_800945E6` to 0.
The checkpoint still services only its pre-captured first token, so the
second DMA stays active, incomplete, and invisible in VRAM with DICR
`0x00800000` and no new IRQ. Ten focused groups bring the suite to 558
tests; the independent C oracle verifies all 152 words/delay slots and
20 state scenarios. Its complete pump and one-token contracts remain frozen.
Full proof is in `docs/b53i_c_idle_gpu_pump.md`.

**Prior milestone:** B53I-B2 connects one explicitly captured DMA2 token
to physical DICR completion state, a separate source-3 edge bridge, the
bounded retail CPU source scanner, complete 96-word `func_80074520`, and the
typed DMA channel-2 identity `0x80076EE4`. Stored DICR excludes bit 31;
physical reads derive it as `force15 || (master23 && any flags24..30)`, and
every raw transition sticky-latches only a false-to-true edge. Normal DMA
completion always publishes VRAM and clears CHCR, but creates flag 26 only
when channel-2 enable and master are set. CPU I_STAT acknowledgement precedes
`func_80074520`; DICR W1C precedes the DMA callback. Completion, edge bridge,
and CPU service remain independently callable, generation-checked phases.
At that rung, completed DMA made the B53H pump stop at the then-untranslated
`func_80076EE4_idle_pump`; B53I-C now resolves that boundary while preserving
all phase separation and acknowledgement ordering. Actual typed-pump entry
telemetry still proves completion and edge bridging cannot hide an early
call in the silent busy path. Fifteen B2 focused groups remain retained.
The independent B2 oracle verifies all 96 DMA words, 53 CPU dispatcher window
words plus its full-body hash, and 17 explicit state scenarios without
production imports. Full proof is in `docs/b53i_b2_dma_irq_delivery.md`.

**Prior milestone:** B53I-B1 adds the single native 16-bit I_STAT authority
beside I_MASK in `platform/pe_irq.[ch]`. I_STAT writes are exact W0C
(`status &= written`), assertions retain masked sources, and reset advances
a 64-bit generation. Retail ResetCallback installs source 0 guest identity
`0x8007440C` with its ordered B(5Bh)(0)/C(0Ah)(3,0) control calls, then source
3 identity `0x80074520`; final I_MASK and `D_80094614` are `0x0009`. CPU
callback slots remain solely guest-backed. Full proof is in
`docs/b53i_b1_irq_state_registration.md`.

**Prior milestone:** B53H translated the execution-proven busy-DMA fast
path of the 152-word / `0x260` libgpu queue pump `func_80076EE4`
(`0x80076EE4..0x80077143`, body SHA-256
`a124857ab6fd91a3b68ea3e5c2efa5337bf6ed5528ef94ca23bc4a781337a78c`).
ABI is `int func_80076EE4(void)`. The pump's first state read is DMA2 CHCR
through the retail pointer `D_80095860` = `0x1F8010A8`; when
`CHCR & 0x01000000` is set it returns exactly `1` from the branch delay
slot at `0x80076F0C` and exits through the shared epilogue, having written
nothing but its own stack frame. That is the canonical path: the first
LoadImage transfer is still in flight, so the pump performs no I_MASK
exchange, no queue read, no GPUSTAT read, no callback or DICR change, no
worker call, and no consumer advance. At B53H, the remaining 136 words
(`0x80076F10..0x8007712C`) were deliberately left untranslated because
reaching them required genuine hardware completion; B53I-C now completes
that suffix. With
the pump returning a real value, `func_80076C34` yields its retail pending
count `(producer-consumer)&0x3F` and both LoadImage calls complete, so the
canonical Disc 1 strict frontier advances one rung to the **pre-existing
B50 prefix cut** in `func_8006AD40`, which B53H names
`func_8006AD40_prefix_cut` so it stays a visible frontier (exit 1) rather
than degrading to a silent exit 0; bootstrap strict remains
`func_8007F72C` from `func_800698D4`. The queued second LoadImage stays
unconsumed (producer=1, consumer=0, entry byte-identical) and the transfer
stays pending for a later genuine completion. Boundary signalling is now
explicit rather than inferred from the sticky-first stop reason: the pump
reports through an out-parameter and the sibling direct-issue path compares
a monotonic `PE_Port_StopEpoch()`, so no latched stop can suppress retail's
unconditional I_MASK restore. Eight new tests bring normal and fresh
ASan/UBSan suites to 525/525, all 15 frozen B53B tests remain green, and
the independent B53H oracle verifies all 152 words, the arithmetic branch
target, the four-site caller census, the dormant jump-table slot, and every
idle-path scenario without ever completing DMA. Full proof is in
`docs/b53h_func_80076EE4.md`.

**Prior milestone:** B53G translated the complete 43-word / `0xAC` Psy-Q
DMA callback-slot setter `func_800746A0`
(`0x800746A0..0x8007474B`, body SHA-256
`ac160079410a40d719e5a8668f627d05d36d287d08959adc22fd3e069e4e99dd`). It
stores the 32-bit guest handler into the guest-backed eight-slot table
`D_800956C0`, performs the exact DICR read/modify/write through the single
B53B `pe_gpu` authority, and returns the full previous identity. Retail
validates no channel, so neither does the port; the enable bit is
`1 << ((channel + 16) & 31)` and master bit 23 is set on both the install
and removal paths. DICR bit 31 is read and immediately masked off, never
tested and never written; I_MASK is not touched. At the B53G setter rung,
bit 31 required no additional modeling because it was masked before use;
B53I-B2 later extends the same DICR authority for
actual physical reads and edge delivery. Registration is not delivery: at
B53G the pending first LoadImage DMA stays active
and incomplete, and no callback, pump, DrawSync, VRAM, VBlank, or GPU-ready
change occurs. This completes the B53F wrapper `func_80073CF4`, so the
canonical enqueue proceeds through retail ring construction and publishes
the second LoadImage request (worker `0x80076664`, argument = guest address
of the copied inline RECT, auxiliary `0x8012B8B8`, producer `0`→`1`,
consumer unmoved) and stops at the untranslated queue pump. Canonical
strict execution now stops at `func_80076EE4` from `func_80076C34`;
bootstrap strict remains `func_8007F72C` from `func_800698D4`. Seven new
tests bring normal and fresh ASan/UBSan suites to 517/517, all 15 frozen
B53B tests remain green, and the independent B53G oracle verifies all 43
words, every delay slot, the whole-executable caller census, both DICR RMW
expressions, and the explicit bit-31 verdict. Full proof is in
`docs/b53g_func_800746A0.md`.

**Prior milestone:** B53F translated the execution-proven installed-target
path through the 12-word Psy-Q indirect wrapper `func_80073CF4` after
ResetCallback, forwarding the DMA channel and 32-bit guest handler identity
to the installed `func_800746A0` backend without introducing a native
function pointer or a duplicate callback table. Full proof is in
`docs/b53f_func_80073CF4.md`.

**Historical B32 detail:** `func_800614AC` is translated retail logic: 36
instructions / `0x90` bytes at executable `0x800614AC..0x8006153B`
(exclusive end `0x8006153C`, file offset `0x51CAC`, live split
`asm/disc1/51CAC.s`). It stores `a0 & 0x00FFFFFF` at `0x8009D14C`, computes
the three pairwise byte means with the retail arithmetic and saturation
branches, stores the packed result at `0x8009D150`, and returns that result.
It has no guest reads, direct callees, hardware activity, blocking, pointer
storage, clamping, or fallback. The B32 oracle independently verifies the
executable SHA-1 and all 36 words, executes delay slots, and checks ordered
writes and return values. The `func_8005D6F4` call is at `0x8005D8C8`, with
`a0=0x00404040` formed by the delay-slot `ori`; the return is discarded. Its
remaining sibling calls are `func_8005E884`, `func_8005E850`,
`func_800649D0`, and `func_80052790` in raw ROM order. The final B32 suites
are 344/344 native and 344/344 ASan/UBSan; real-disc strict now stops at
`func_8005E884` from `func_8005D6F4`, while bootstrap strict remains at
`func_8007F72C` from `func_800698D4`. Nothing has been pushed and the next
rung has not started.

**Historical B30 detail:** `func_80042C78` is a translated retail prefix: 16
instructions / `0x40` bytes at executable `0x80042C78..0x80042CB4`
(exclusive end `0x80042CB8`, file offset `0x33478`). It clears the proven
guest state words at `$gp+0x168/0x170/0x174`, sets `$gp+0x16C` to `0x20`,
calls translated `func_80042CC4` with `a0=0x90` and `a1=0xFF` after its
delay slot, and commits `$gp+0x17C = 0x48` afterward. The direct guest
write footprint is `0x8009CED8`, `0x8009CEDC`, `0x8009CEE0`, `0x8009CEE4`,
and `0x8009CEEC`; it does not block or use host pointers, mirrors, clamps,
or fallbacks. `tools/b30_oracle.py` independently verifies and executes all
16 words. B31 implements that dependency; real-disc strict now stops at
`func_800614AC` from `func_8005D6F4`.

**Historical B31 detail:** `func_80042CC4` is translated retail logic: 31
instructions / `0x7C` bytes at executable `0x80042CC4..0x80042D3C`
(exclusive end `0x80042D40`, file offset `0x334C4`, live split
`asm/disc1/334C4.s`). It is a void leaf with `(a0, a1)` arguments. It clears
`0x800A1878`, uses the branch delay-slot color shift, fills the
termination-dependent byte ramp while signed `lbu < a1` holds, and stores
the final count at `0x8009CEE0` (`$gp+0x170`). The B30 call `(0x90, 0xFF)`
produces `00 90 CF EA F6 FB FD FE FF` and count 9. Its direct writes are the
actual subset of `0x800A1878..0x800A1887` plus the word at `0x8009CEE0`; it
has no callees, SDK/GPU/disc/audio/input activity, blocking, host pointers,
mirrors, or clamping. Executable call sites are `func_80042C78 @ 0x80042C98`
and `func_8005D2B4 @ 0x8005D5E8`; both discard the void return, and incoming
a2/a3 are overwritten before use. `tools/b31_oracle.py` independently
checks the executable SHA-1, all 31 words, delay slots, ordered writes,
threshold paths, and counts. The final native and ASan/UBSan suites are
341/341. Real-disc strict stops at `func_800614AC` from `func_8005D6F4`;
bootstrap strict remains at `func_8007F72C` from `func_800698D4`. Nothing has
been pushed and the next rung has not started.

**Historical B29 detail:** `func_80053D2C` is translated retail logic: 80
instructions / `0x140` bytes at executable `0x80053D2C..0x80053E6B`
(exclusive end `0x80053E6C`, file offset `0x4452C`). It scans the shared
`$gp` resource table `D_8009D048/D_8009D050` for the first zero halfword,
looks up records through translated `func_8005DB44`, dispatches record types
1–18, and stores exact halfwords without clamping. Types 1–9 call translated
`func_80053968` with the original argument and return the retail
zero-inversion; types 16–18 call translated `func_80053B48` with the selected
record pointer and forward its return. These were centralized boundaries at
B29 and were completed by B41 and B42 respectively.
The function returns 0 for a missing record/type 11/unknown type, and 1 for
the full-table failure path. Its only direct guest write is the selected table
halfword. It does not block and adds no SDK/GPU/disc/audio/input activity,
host pointers, KUSEG mirror, or address-zero fallback.

`tools/b29_oracle.py` independently transcribes and verifies all 80 words
against executable SHA-1 `452fb033f2eaa4b18aa20a5bca60b8125af3a37b`, then
executes that transcription with delay slots, exact writes, returns, and
unresolved call boundaries. Dedicated tests cover exact widths, full-table
and direct-ID behavior, dependency returns, and reset behavior. B29 is
337/337 normal and 337/337 ASan/UBSan. Real-disc strict now stops at
`func_80042C78` from `func_8005CCA4` (exit 1); B29 does not implement it.

**Historical B26 context:** B26 `func_8005DC4C` rung — PE.IMG message/string-table
lookup (20 words at `0x8005DC4C..0x8005DC9B`, file `0x4E44C`, live split
`4CC98.s:1779-1802`).  A pure read-only walk of a nested relative-offset
archive whose base is the cycle-A streaming destination `0x800A8028`:

```
ptr = mem32[0x800A802C] + 0x800A8028      # both sign-extended from lui 0x800B
tbl = ptr + mem32[ptr + 4]
return (idx <u mem16[tbl]) ? tbl + sext16(mem16[tbl + 2 + 2*idx]) : 0
```

Records are addressed by a **signed 16-bit** offset relative to the table
(stride 2, not 16), so the whole record pool must live within `tbl ± 32 KiB`.
Zero is retail's own out-of-range return (`addu $v0,$zero,$zero`), not an
invented sentinel.  The routine performs three guest reads on the failure
path and four on the success path, **zero guest writes**, has no callees,
cannot block, and is deterministic from guest state alone.  Its signature
takes ONE argument: no instruction in the body reads `$a1`.

Measured against the real Disc 1 USA image: `R = 0x30`, `S = 0x14`,
`count = 120`, entry values `242..1971`, so records span
`0x800A815E..0x800A881F`.  `min(entry) == 2 + 2*count` exactly — the record
pool begins where the offset table ends, independently confirming the
layout.  Index 30 (the index all three `func_8005D6F4` call sites use)
resolves to `0x800A82A9`, whose bytes are `10 48 30 FF`.  Independent
oracle: `tools/b26_oracle.py`.

Exe-wide there are **39 call sites in 24 distinct callers**; observed
constant indices run 3..117, all below the measured count of 120.  Every
return consumer is a register move or one `sw $v0,484($gp)`; no call site
anywhere dereferences `$v0` in place, and none compares it to zero.

Previous milestone: B25 func_8005D6F4 rung — resource-buffer +
display-state initializer (147 words at `0x8005D6F4..0x8005D93F`, file
`0x4DEF4`, live split `4CC98.s`).  bzero(`0x800C0DE0`, `0x12E4`) via the
real BIOS A(28h) trampoline, two `0xFF` buffer fills at `0x800C0DF0`
with retail reload-per-iteration loops, two 0xFF-terminated string
copies whose sources now come from the REAL func_8005DC4C, state stores
`D_8009D0C8/C0/C4` + `D_8009D218`, direct stores `sh 0x0203 →
0x800C1F80`, `sw 0x00404040 → 0x800C0E44`, timer-tick clears
`0x800A76A4/B0/BC/C8`, terminator bytes at `0x800C20A4/B4`; returns
0xFF.  Seven unresolved callees route through the centralized bootstrap
boundary in retail ROM order.  Independent oracle:
`tools/b25_oracle.py`.

Previous milestone: B24 func_8005BCBC rung — resource-state
pointer/count selector (21 words at `0x8005BCBC..0x8005BD0F`, file
`0x4C4BC`, live split `4C4BC.s`).  Stores the incoming record pointer
into `D_8009D0C8`, selects a guest buffer base into `D_8009D0C0`
(0x800C20A4/0x800C20B4 for records, 0x800C0DE0/0x800C0DF0 for a0 == 0
per `D_8009D218`), writes count 8 to `D_8009D0C4`, returns 8.  No
callees.  Independent oracle: `tools/b24_oracle.py`.

Previous milestone: B22 func_8005DE88 rung — resource-list & state
initializer (23 words at `0x8005DE88..0x8005DEE3`, file `0x4E688`, live
split `4CC98.s`).  It links the 20 twelve-byte records beginning at
`D_800A2090`, null-terminates `D_800A2174`, and initializes the six
`$gp`-relative state words `D_8009D0DC..D_8009D0F0` exactly.  No callees;
returns void.  Independent oracle: `tools/b22_5de88_oracle.py`.

Previous milestone: func_8006A9E4 rung — PE.IMG streaming resource
load, translated as retail logic with two unresolved callees routed
through the centralized bootstrap boundary (classification 1).  Complete
body is 215 retail words / `0x35C` verified against the SHA-1-exact
executable (VRAM `0x8006A9E4`–`0x8006AD3F`, file `0x5B1E4`; live split
`asm/disc1/5B1E4.s`).  ROM order: `ClearImage({0,0,0x3FF,0x1FF},0,0,1)`
through the REAL host SDK (func_80074F44), four streaming
sector-read/poll cycles from PE.IMG (tables at `D_800930DC..E8`,
destinations `D_800A8028` and the `lw(D_800B0E6C)` stream buffer;
A/B polls restart the whole cycle on a -1, C/D polls are sltu-clamped so
the restart branch is dead retail code), a `0x10A50`-byte copy to
`D_800E2858`, two func_8006E498 archive lookups (keys `0x57D40D84` /
`0x57D41D84`, exact delay-slot store order `D_800B0E20`→`E18`→`E1C`),
the unresolved func_80087090 SPU upload, and a `0x1400`-byte copy to
`lw(D_800B0E08)`.  Sole exe call site `func_8001220C` @`0x80012284`
(nop delay slot, immediately after the func_8003E680 jal @`0x8001227C`);
return ignored → `void(void)`.  Three dependencies translated with it:
func_8006E6A8 (11-word issue wrapper; retail sector count forwarded to
the generic provider, proven by the 34-sector/67792-byte and
3-sector/5120-byte cycle/copy pairs), func_8006E7E8 (19-word completion
poll + `D_800B0CD8 &= 0xFEFFBFFF` RMW on st∈{-1,0}), and func_8006E498
(31-word pure guest table walk, guest-address result).  The rung also
fixed a 6D-S-class split-brain defect: `D_800B0E24..D_800B0E6C` are now
guest-RAM lvalue macros (retail readers load guest RAM) instead of
  duplicate host globals.  **Historical B17 context:** Phase 6E-B17 advanced
the strict frontier to `func_800528F0`: func_800527C8 (the multi-subsystem bootstrap dispatcher,
49 words, 17 calls) is now fully translated with 7 leaf implementations
and 10 unresolved callees routed through the centralized bootstrap
boundary in retail ROM order.  Strict mode with `--disc-image` stops at
`func_800528F0` (first unresolved callee INSIDE the translated dispatcher);
`--bootstrap-disc` still stops at `func_8007F72C` by design.**

**Historical status:** B31 func_80042CC4 VERIFIED — its final suite was
341/341 native and 341/341 ASan/UBSan; its strict real-disc frontier was
`func_800614AC` from `func_8005D6F4` (exit 1, normal and sanitizer agreed), bootstrap-disc stopped
at `func_8007F72C` from `func_800698D4` (exit 1, normal and sanitizer
agree). `func_8005CCA4` is translated retail logic — a 223-word resource-table
initializer with a 50-halfword descending zero loop
(0x800C0E48..0x800C0EAA = D_8009D048 + 0x62); its earlier GA_E24/E28 stores
sit below 0x800C0E48 and survive, and its four $gp-relative state words are
the shared host globals D_8009D048/50/58/64 (also used by func_80052C6C).
The completed B28 oracle independently transcribes all 223 words (W_5CCA4) and
cross-checks every word against the SHA-verified exe before executing from
the transcription.  The bootstrap-disc fixture now seeds a valid minimal
archive at D_800A8028 so func_8005DC4C returns a valid pointer instead of 0.
The completed B31 boundary was `func_80042CC4`; B32 completed
`func_800614AC`, leaving four sibling callees (`func_8005E884`,
`func_8005E850`, `func_800649D0`, `func_80052790`) behind the centralized
boundary. The bootstrap fixture correction seeds the valid archive lookup
result `0x800A8400`; malformed `PE_StoreU32(0x800A803C, 0xA49D968F)` leaves
`func_8005DBAC(0)` returning `0x24A816B7` without dereference. Address zero
remains invalid and no KUSEG or low-address mirror exists.

Final evidence: framebuffer SHA-256
`fb28dc21dd1e41eb72b8fe22dd3295bb8ed0c040aa88f7885a68dedc2629dfdb`,
bootstrap trace `42c1956e…`, real-disc trace `7b8724ac…`, real-disc
FNV-1a-64 `7D860391E1ED6C97`, and matching executable SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`. Nothing has been pushed; the
next rung has not started.

The B25 degenerate in-RAM default for the consumed `func_8005DC4C`
return is retired: the value now comes from the real archive.  Because
the archive is guest-resident and arrives from PE.IMG via cycle A of
`func_8006A9E4` before the dispatcher runs, tests that invoke
`func_8005D6F4` must establish that precondition (`B26_SeedArchive`) —
with the region zeroed the lookup correctly returns 0 and the caller
dereferences address 0, which the checked-access layer surfaces as a
fatal error rather than masking.  No KUSEG mirror, no clamping, and no
function-specific bypass was added.

Independent oracle: `tools/b26_oracle.py` (delay-slot-aware MIPS-I
interpreter on the SHA-1-verified retail words; seven interpreter
self-tests; conditions sampled at issue; every read/write logged with
width and order; 36 checks over empty, populated, terminator, failure,
first/last, repeated, `$a1`-independence, signed/wraparound, dirty,
reset and return-address boundary scenarios).  Dispatcher oracle still
reports 2 unresolved callees in retail order: `func_80051CC4`,
`func_80042C78`.  Matching build exact at SHA `452fb033` (227 C leaves).

Previous status: FUNC_800527C8 RUNG VERIFIED — 251 native tests pass;
ASan/UBSan clean (tests + headless + strict runs);
RNG, LZCR, callback, and dispatcher oracle dumps byte-identical to their
independent interpreters on the retail exe; 3 deterministic test runs
identical (251/251); windowed framebuffer matches headless; matching
build remains exact at SHA `452fb033` (227 C leaves).

**Previous milestones:** 6E-B15 (func_80038D1C byte test-and-clear —
11 retail words on `D_80091A20`, both return paths; with it
func_8003E680 became fully translated);
6E-B14 (func_8006536C record-table clear +
index byte clear — 19 retail words: 28×3-word table at `D_800A3180`
span `..0x800A32CF`, index byte `0x8009CDB4`);
6E-B13 (func_80034F10 subsystem table clear +
flag-bit clear — 45 retail words: 512-word array at `D_800A77F0`,
14×160-word matrix at `D_800BEA90` span `..0x800C0D8F`, six scalars,
`D_800B0CD8 &= ~0x3000`);
6E-B12 (func_8001A890 subsystem scalar/array
clear — 34 retail words: `0x8009CE08`–`0x8009CE17`, six stride-4
halfwords, four words, two halfwords, 20-word array at `D_8009DFB0`);
6E-B11 (func_800124F8 subsystem table
clear — 31 retail words: five scalars, 72×11-word matrix at
`D_8009D310`, 16-word array at `D_8009DF70`);
6E-B10 (func_80068D28 display-record data
init — 63 retail words, scalar block + 320×224 records + GP0-shaped
`0xE1000440` data words at `D_800BCF88`);
6E-B9 (func_8005BCA8 empty `jr $ra; nop`
stub rung — 2 retail words, zero footprint);
6E-B8 (func_80029388 slot-table clear +
default-record init — leaves func_8002F658 / func_80020EFC; 7×220
SlotRecord table at D_800A5D58);
6E-B7 (func_800371A4 `$gp`-relative byte
setter rung — 3-word `sb $a0, 0x124($gp)` → `D_8009CE94`);
6E-B6 (func_80073D24 VBlank callback rung —
libetc slot-4 wrapper with func_80074478 previous-handler semantics and
func_8007440C dispatch, guest-backed at `0x8009568C`/`0x800956AC`,
oracle-verified);
6E-B5 (func_80036DC8 timer-record init rung —
three 12-byte records at `0x800A76A0/AC/B8`, all 11 word stores in ROM
order);
6E-B4 (func_8003EAC8 LZCR registration rung —
GTE LZCS/LZCR leaf, exact edge semantics incl. the preserved below-table
write at `0x800A76EC`, oracle-verified; call-site correction: 20 distinct
sites, not 63);
6E-B3 (func_8003E974 initialization rung —
subsystem state clear + the 20-call ROM-ordered registration series);
6E-B2 (func_80070D6C RNG advance rung — the game's
lagged-Fibonacci RNG, init/advance/ranged wrapper, byte-exact against an
independent oracle on the retail executable, which the port loads into
guest RAM via SYSTEM.CNF `BOOT=` because the RNG read cursor provably
cycles through 14 retail code words below its table);
6E-B1 (func_80070D10 RNG-table init rung);
6E-A batch 3 (real Disc 1 byte path into guest RAM at `D_80011614`).

## Architecture

- **Build system:** CMake 3.16+
- **Reusable runtime target:** `pe_field_runtime` produces
  `libpe_field_runtime.a` from the translated retail graph, guest globals,
  bootstrap policy, and host-safe providers. The CLI-only `port_main.c` and
  X11 adapter remain separate consumers. This target boundary is verified;
  it is not a claim that Day 1 field execution is complete. See
  `docs/field_runtime_library.md` and
  `../docs/evidence/pe-field-runtime-library/REPORT.md`.
- **Backend:** Headless software framebuffer + X11 window via dlopen
- **Output:** PPM (Portable Pixmap), deterministic byte-identical results
- **Game entry:** `port_main.c` → `func_8001220C_port.c` → full PE boot chain
- **Memory strategy:** Contiguous 2 MiB guest RAM (`pe_guest_ram.[ch]`).
  All guest addresses are `pe_addr_t` (`uint32_t`); translation to host
  pointers happens only at real access sites via bounds-checked
  `PE_Translate` / `PE_LoadU8/16/32` / `PE_StoreU8/16/32`.  Named globals
  that live in guest RAM (the `0x800B0CD8` block, `D_80094488/8C`,
  `D_800BCE80`) are typed lvalue macros over `PE_Translate` — one store,
  no split-brain.  `PE_RamInit` allocates + zero-fills; `PE_RamReset`
  re-zeroes; `PE_RamDestroy` frees.
- **Callback slot model:** `pe_callback.[ch]` — guest-backed retail
  callback queue (Phase 6E-B6): 8 handler slots at guest `0x8009568C` +
  dispatch counter at `0x800956AC`, with func_80074478-faithful
  set-slot (previous-handler return, no store on identical handler) and
  func_8007440C-faithful dispatch.  Host side: typed guest-address →
  host-function bindings, full pointer width, never guest-visible.
- **Bootstrap policy:** `pe_bootstrap.[ch]` — every `BOOTSTRAP_RET`
  provider goes through `Bootstrap_ReturnInt/Void`.  Strict mode
  (`--strict-stubs`) aborts centrally at the first invoked provider.
  Deterministic provider sequences (`Bootstrap_SetIntSequence`) script
  per-symbol return values for wait-loop testing.
- **Disc layer:** `pe_disc.[ch]` — read-only host access to the
  user-supplied Disc 1 image (never embedded, copied, staged, or
  committed).  Single-track BIN/CUE MODE2/2352: 2048 user bytes at raw
  sector offset +24, ISO9660 PVD at user sector 16.  All reads
  bounds-checked; missing/truncated/malformed/wrong-disc inputs rejected
  safely.  `pe_libcd.c` carries the real disc providers:
  `func_80082314` (PVD verify, result word `D_800B28F8`),
  `func_80081414` (DsSearchFile, 24-byte CdlFILE to a guest address),
  `func_80080C48` (CdPosToInt), `func_8006E6D4` (read issue — synchronous
  bounded copy into guest RAM), `func_800811E4` (poll: 0 done / -1
  timeout).  `func_800698D4` runs the verbatim retail mount sequence;
  `--bootstrap-disc` remains as an explicit test fixture only.
- **Guest exe image:** `pe_guest_image.[ch]` — with `--disc-image`, the
  retail boot executable named by SYSTEM.CNF `BOOT=` is validated
  (PS-X EXE header, geometry, guest-RAM bounds) and loaded into guest
  RAM at its `taddr`.  Retail code reads its own text as data — the
  func_80070D6C RNG read cursor cycles through 14 code words below its
  table (`tools/rng_oracle.py`, proven against the retail exe) — so
  faithful RNG output needs those bytes.  Without an image the words
  read as zero: a documented fixture-mode divergence that nothing on
  the boot-to-black path consumes.

## Translated RNG (Phase 6E-B1/B2)

| Function | Size | Purpose |
|----------|------|---------|
| `func_80070D10` | 0x5C | Lagged-Fibonacci table init (17 words, 2 index words) |
| `func_80070D6C` | 0x64 | RNG advance; verbatim `i2 \|= 0x40` wrap (cycles, never clamps) |
| `func_80070DD0` | 0x34 | Handwritten ranged random: `a + (rand16·(b−a) >> 16)` |

Correctness gate: `--rng-oracle-dump` output is byte-identical to
`pc_port/tools/rng_oracle.py` running independently on the retail exe
(SHA-1 `452fb033…`).

## Translated subsystem-init rungs (Phase 6E-B3/B4)

| Function | Size | Words | Purpose |
|----------|------|-------|---------|
| `func_8003E974` | 0x154 | 85 | Subsystem state clear + 20 `func_8003EAC8` registration calls |
| `func_8003EAC8` | 0x3C | 15 | GTE LZCS/LZCR-indexed registration leaf |

`func_8003E974` (retail `$gp` = `0x8009CD70`) clears 5 `$gp`-relative
globals, clears the 32-word array at `D_800A76F0`, then issues the complete
ROM-ordered `func_8003EAC8(mask,value)` series
(`(1,0x4000) (0x80,0x1000) (0x100,0x2000) (0x8,0x10) (0x20,0x40)
(0x40,0x80) (0x10,0x20) (0x2,1) (0x4,8) (0x200,0x2000) (0x400,0x4000)
(0x2000,0x8000) (0x1000000,0x100) (0x2000000,0x200) (0x4000000,0x400)
(0x8000000,0x800) (0x10000000,0x1000) (0x20000000,0x2000)
(0x40000000,0x4000) (0x80000000,0x8000)`), ending with
`D_8009D1A0 |= 0x4000`.  Idempotent.  Result: 20 slots populated
(0-10, 13, 24-31), no duplicates; slots 11/12/14-23 stay 0 from the clear.

`func_8003EAC8` is a handwritten GTE LZCS/LZCR leaf:
`idx = (a0 == 0x80000000) ? 31 : 31 - LZCR(a0)`, then a word store
`D_800A76F0[idx] = a1` via the `sll`/`addu` chain.  LZCR counts leading
sign bits, so `idx ∈ [-1, 31]`: zero/all-ones inputs write one word below
the table at `0x800A76EC` (valid guest RAM — preserved, never clamped),
positives map to the highest SET bit, other negatives to the highest ZERO
bit.  20 distinct call sites, all in `func_8003E974`, all constant
arguments (the earlier "63" counted overlapping split files).  Bounded
test-only argument recording (`PE_3EAC8_RecordReset/SetEnabled/Count/At`)
is non-semantic: production stores identically with the recorder off.
Correctness gate: `--lzcr-oracle-dump` is byte-identical to
`tools/lzcr_oracle.py`, a tiny MIPS interpreter executing the verified
retail words.

## Translated timer-record init rung (Phase 6E-B5)

| Function | Size | Words | Purpose |
|----------|------|-------|---------|
| `func_80036DC8` | 0x28 | 10 | Timer-record init dispatcher (3 calls, retail order) |
| `func_80036DF8` | 0x3C | 15 | Record 0 init (5 stores incl. 2 dead zero-stores) |
| `func_80036E34` | 0x24 | 9 | Record 2 init |
| `func_80036E58` | 0x24 | 9 | Record 1 init |

Net state: three 12-byte records at `0x800A76A0/AC/B8`, each `{ 1, 0, x }`
with record 0 field2 preloaded to `0x1499700` (21,600,000).  Consumers
(`func_80019DB8`, `func_80052894/528C4`, and the packed-field writer
`func_80036E7C`) index field1 via base+i*12 and divide by 60 — consistent
with 60 Hz tick counters.  Sole caller of the dispatcher is
`func_8003E680`; the leaves are called only from it.  Idempotent;
verbatim ROM-order stores, all guest-RAM-backed.

## VBlank callback slot rung (Phase 6E-B6)

| Function | Size | Words | Role |
|----------|------|-------|------|
| `func_80073D24` | 0x34 | 13 | libetc jump-table wrapper: field 0x14, slot forced to 4, return forwarded |
| `func_80074478` | 0x2C | 11 | installed setter: `prev = tab[slot]; if (h != prev) tab[slot] = h; return prev` |
| `func_8007440C` | 0x6C | 26 | dispatcher: counter++, invoke non-null slots 0..7 in order |

Retail state is guest-RAM resident: 8 handler slots at `0x8009568C`
(D_8009568C, zero-initialized image bytes) and the dispatch counter at
`0x800956AC` (D_800956AC).  Exact retail address arithmetic is preserved
— slot 8 aliases the counter.  The host binding map (guest address →
host function, full width) is plumbing only; dispatch of an unbound
guest address is a visible error.  Both func_8003E680 call sites
(`0x8003E6E0` clear, `0x8003E6F0` install of `0x8003E91C`) discard the
return, matching retail.  ResetCallback's first guard-passing call
zeroes all 8 slots + counter (func_800743B4 semantics).  Correctness
gate: `--callback-oracle-dump` is byte-identical to
`tools/callback_oracle.py`, a MIPS interpreter executing the verified
retail words.

## $gp-relative byte setter rung (Phase 6E-B7)

| Function | Size | Words | Role |
|----------|------|-------|------|
| `func_800371A4` | 0xC | 3 | `sb $a0, 0x124($gp)` — `(uint8_t)a0` → `D_8009CE94` (guest `0x8009CE94`) |

Complete body verified word-for-word against the retail executable
(`0xA3840124 / 0x03E00008 / 0x00000000`).  Two call sites
(`func_8003E680` arg 0, `func_800527C8` arg 1), both return-discarding;
sole reader `func_80037870` (`lbu` + compare) is off the boot path.
Matches the decomp's matching C leaf statement-for-statement.  Single
byte of write footprint, proven by a full 2 MiB canary scan.

## Slot-table clear + default-record init rung (Phase 6E-B8)

| Function | Size | Words | Role |
|----------|------|-------|------|
| `func_80029388` | 0x6C | 27 | Clear 7 `D_800A5D58` slot in-use words (stride 220) + `D_8009D2A0`/`D_8009D2EC` bytes; calls the two leaves below |
| `func_8002F658` | 0x114 | 69 | Copy exe-rodata default records `D_80010928`→`D_800B8A20` (0x70) and `D_80010998`→`D_800B0CB0` (0x18); zero `D_8009D1B0`/`D_8009D1B4` |
| `func_80020EFC` | matched leaf | — | Five `$gp`-relative byte clears (`D_8009CE3C`/`D_8009D1D4`/`D_8009D1DC`/`D_8009D2D8`/`D_8009D1F0`) |

All 27 `func_80029388` words and all 69 `func_8002F658` words verified
against the retail executable (taddr `0x80010000`); `func_80020EFC` is
the decomp's Phase-5DE matching C leaf.  Exactly one exe-wide call site
for `func_80029388` (`func_8003E680` @0x8003E700, nop delay slot — the
`addu $a0,$zero,$zero` at 0x8003E6FC belongs to the preceding
`func_800371A4` call), one for `func_8002F658` (from `func_80029388`),
two for `func_80020EFC` (`func_80029388`, and `func_80029810` off the
boot path).  The slot table is the same 7×220 `SlotRecord` table the
decomp's matched `func_8002F9CC` leaf clears — retail has two clearing
functions; both are reproduced.  Write footprint proven by full 2 MiB
canary scans; copy content verified word-for-word against deterministic
source patterns.

## Empty stub rung (Phase 6E-B9)

| Function | Size | Words | Role |
|----------|------|-------|------|
| `func_8005BCA8` | 0x8 | 2 | Empty `jr $ra`/`nop` — no guest-memory effect |

Both words verified against the retail executable.  Sole call site
`func_8003E680`; return value unconsumed.  Zero write footprint proven
by a full 2 MiB canary scan; dirty state and `PE_RamReset` state pass
through unchanged.  Translating it advances strict mode past the call.

## Display-record data init rung (Phase 6E-B10)

| Function | Size | Words | Role |
|----------|------|-------|------|
| `func_80068D28` | 0xFC | 63 | Double-buffered display-record data init at `D_800BCF88` |

All 63 words verified against the retail executable (exe
`0x80068D28`–`0x80068E20`, live split `55430.s`).  Sole call site
`func_8003E680` @0x8003E710 (nop delay slot), `void(void)`, `$v0`=0
unconsumed.  Writes a scalar block at base+0x60..0x70 (halfwords
0xFF/0, bytes 1/2, post-loop halfwords 0), then two 16-byte records at
base+i*0x10 (bytes 3/0xFF×3/0x60→0x62, halfwords 0/0/0x140/0xE0) and
two 8-byte records at base+i*0x8 (byte 1, word `0xE1000440` — GP0-shaped
DATA in a guest struct, not a hardware write).  Loop byte values are
retail load-after-store from the just-written scalars (reproduced via
`PE_Load`).  Write extent `0x800BCFBB`–`0x800BCFF9`; idempotent,
including after `PE_RamReset`.  No SDK/GTE/hardware/callback use; no
oracle needed — the straight-line fixed-value init is fully proven by
the 63-word executable verification.  Strict mode advances to
`func_800124F8`.

## Subsystem table clear rung (Phase 6E-B11)

| Function | Size | Words | Role |
|----------|------|-------|------|
| `func_800124F8` | 0x7C | 31 | Zero-fill 72×11-word matrix at `D_8009D310` (stride `0x2C`) + 16-word array at `D_8009DF70` + five scalars |

All 31 words verified against the retail executable (exe
`0x800124F8`–`0x80012570`, file `0x2CF8`; live split `2A0C.s`).
Sole call site `func_8003E680` @`0x8003E718` (nop delay slot),
`void(void)`, `$v0`=0 unconsumed.  Pure zero-stores in ROM order:
`0x8009D300` (word), `0x8009D308` (halfword — `0x8009D304`
untouched), `0x8009CDFC`, then the row-major matrix
(`0x8009D310`–`0x8009DF6F`), `0x8009CE00`, the array
(`0x8009DF70`–`0x8009DFAF`, contiguous with the table end), and
`0x8009CE04`.  No reads, no SDK/GTE/hardware/GPU work; idempotent,
including after `PE_RamReset`.  Write footprint proven by a full 2 MiB
canary scan; strict mode advances to `func_8001A890`.

## Subsystem scalar/array clear rung (Phase 6E-B12)

| Function | Size | Words | Role |
|----------|------|-------|------|
| `func_8001A890` | 0x88 | 34 | Zero-fill `0x8009CE08`–`0x8009CE17` + six stride-4 halfwords + four scattered words + two halfwords + 20-word array at `D_8009DFB0` |

All 34 words verified against the retail executable (exe
`0x8001A890`–`0x8001A914`, file `0xB090`; live split `A404.s`).
Sole call site `func_8003E680` @`0x8003E720` (nop delay slot),
`void(void)`, `$v0`=0 unconsumed.  Pure zero-stores in ROM order:
word `0x8009CE08`; stride-2 halfword loop `0x8009CE0C`–`0x8009CE13`;
word `0x8009CE14`; 20-word array `0x8009DFB0`–`0x8009DFFC`
(contiguous above 124F8's array); six stride-4 halfwords
`0x8009CE18`–`0x8009CE2C` (interleaved upper halfwords untouched);
words `0x8009D1D8`/`D1FC`/`D2F8`/`D248`; halfwords `0x8009D264`/
`D1CC`.  No reads, no SDK/GTE/hardware/GPU work; idempotent,
including after `PE_RamReset`.  Write footprint proven by a full 2 MiB
canary scan; strict mode advances to `func_80034F10`.

## Subsystem table clear + flag-bit clear rung (Phase 6E-B13)

| Function | Size | Words | Role |
|----------|------|-------|------|
| `func_80034F10` | 0xB4 | 45 | Zero-fill 512-word array at `D_800A77F0` + 14×160-word matrix at `D_800BEA90` (stride `0x280`) + six scalars + `D_800B0CD8 &= ~0x3000` |

All 45 words verified against the retail executable (exe
`0x80034F10`–`0x80034FC0`, file `0x25710`; live split `2422C.s`).
Sole call site `func_8003E680` @`0x8003E728` (nop delay slot),
`void(void)`, `$v0`=&`D_800B0CD8` unconsumed.  ROM order: word
`0x8009D2E8`; 512-word array `0x800A77F0`–`0x800A7FEC`; `D_800B6A80`
(retail stores the same word 64× via a delay-slot loop with no pointer
advance — documented, reproduced as one store with identical end
state); 14×160-word matrix `0x800BEA90`–`0x800C0D8F` (row stride
`0x280`); scalars `0x8009D2AC`/`0x8009D20C`/`0x8009D2F0`/halfword
`0x8009D2A6`/`0x8009D254`/`0x8009D224` (ROM order 53C, 49C, 580, 536,
4E4, 4B4); final RMW `D_800B0CD8 &= ~0x3000` (sole guest read; store
in the `jr $ra` delay slot).  No SDK/GTE/hardware/GPU work;
idempotent, including after `PE_RamReset`.  Write footprint proven by
a full 2 MiB canary scan; strict mode advances to `func_8006536C`.

## Record-table clear + index byte clear rung (Phase 6E-B14)

| Function | Size | Words | Role |
|----------|------|-------|------|
| `func_8006536C` | 0x4C | 19 | Zero-fill 28×3-word record table at `D_800A3180` (stride `0xC`, contiguous 84 words) + index byte `0x8009CDB4` |

All 19 words verified against the retail executable (exe
`0x8006536C`–`0x800653B4`, file `0x55B6C`; live split `55430.s`).
Sole call site `func_8003E680` @`0x8003E730` (nop delay slot),
`void(void)`, `$v0`=0 unconsumed.  ROM order: the nested table clear
(`0x800A3180`–`0x800A32CF`, row-major) then `sb` 0 → `0x44($gp)` =
`0x8009CDB4` — the current-record index (func_800653B8 below reads
`lbu 0x44($gp)` and indexes `D_800A3180` + byte×12, confirming the
28-record×12-byte structure).  No reads, no SDK/GTE/hardware/GPU
work; idempotent, including after `PE_RamReset`.  Write footprint
proven by a full 2 MiB canary scan; strict mode advances to
`func_80038D1C`.

## Byte test-and-clear status leaf rung (Phase 6E-B15)

| Function | Size | Words | Role |
|----------|------|-------|------|
| `func_80038D1C` | 0x2C | 11 | `D_80091A20` test-and-clear: return 0 if flag was set (and clear it), else return `0xFF` |

All 11 words verified against the retail executable (exe
`0x80038D1C`–`0x80038D44`, file `0x2951C`; live split `2951C.s`);
also a matched C leaf in the matching decomp (`src/func_80038D1C.c`).
Exactly two exe call sites, both ignoring the return:
`func_8003E680` @`0x8003E738` (final call; void epilogue follows) and
`func_8006E9A0` @`0x8006EB7C`.  One byte read, one conditional byte
write (only when the flag was nonzero); no SDK/GTE/hardware/GPU work.
Repeated calls are state-stable but not return-idempotent (the first
call consumes the flag); after `PE_RamReset` the flag is 0 → return
`0xFF`, no write.  Write footprint proven by a full 2 MiB canary scan
on BOTH paths (write and no-write).  **Milestone: func_8003E680 is
now fully translated** — strict mode advances past it to
`func_8006A9E4` (from `func_8001220C`).

## PE.IMG streaming resource load rung (Phase 6E-B16)

| Function | Size | Words | Role |
|----------|------|-------|------|
| `func_8006A9E4` | 0x35C | 215 | ClearImage + four PE.IMG sector-read/poll cycles + two archive copies/lookups + translated func_800527C8 dispatcher + unresolved func_80087090 boundary call |
| `func_8006E6A8` | 0x2C | 11 | Issue wrapper: `func_8006E6D4(lba, 0, dest, sectors)`; the generic provider transfers `sectors * 0x800` bytes |
| `func_8006E7E8` | 0x4C | 19 | Completion poll: `func_800811E4` + `D_800B0CD8 &= 0xFEFFBFFF` when st∈{-1,0} |
| `func_8006E498` | 0x7C | 31 | Archive directory lookup by 32-bit key; pure guest table walk; guest-address result |

All 276 words verified against the retail executable (func_8006A9E4:
exe `0x8006A9E4`–`0x8006AD3F`, file `0x5B1E4`, live split `5B1E4.s`).
Sole func_8006A9E4 call site `func_8001220C` @`0x80012284` (nop delay
slot, return ignored).  The four cycles read `end-off` SECTORS at
`D_800B0DD8 + off` (proven: cycle B reads 34 sectors = 69632 bytes
followed by a 67792-byte copy; cycle D reads 3 sectors = 6144 bytes
followed by a 5120-byte copy).  Polls A/B restart the whole cycle on a
-1; polls C/D clamp the status with sltu so a -1 re-polls without
re-issuing.  func_800527C8 is now fully translated (Phase 6E-B17):
invoked exactly once inside the cycle-B poll loop via the retail $s1
one-shot guard; its 10 unresolved callees appear in retail ROM order
before the final func_80087090 call.  func_80087090 is still unresolved
and goes through the centralized bootstrap boundary — strict mode now
stops at `func_80052C6C` (the first unresolved callee inside the
translated dispatcher).  The retry/restart loop bodies beyond first-pass
completion are not externally triggerable in the synchronous host
model (issue always refreshes the poll timestamp before the poll), so
their -1 semantics are covered at the provider level instead.  This
rung also converted `D_800B0E24..D_800B0E6C` from duplicate host
globals to guest-RAM lvalue macros after the strict run proved retail
readers load them from guest RAM (6D-S split-brain defect class).
Write footprint proven by a full 2 MiB canary scan; patterned-fixture
test verifies every copied byte, both lookup results, the delay-slot
store order, and the provider invocation order.

## Translated Boot Rung functions (Phase 6D/6D-R/6D-S)

| Function | Matching size | Words | Purpose |
|----------|--------------|-------|---------|
| `func_8003E610` | 0x70 | 28 | Display/graphics bring-up (10 straight-line calls) |
| `func_8003E680` | 0xD4 | 53 | Subsystem-init dispatcher (5 globals, 2000 polls, callback) |
| `func_8006A5BC` | 0x90 | 36 | Boot init with two VSync-polled wait loops |
| `func_8006A64C` | 0x28 | **10** | Boot memory-layout wrapper |
| `func_8006A674` | 0x260 | 152 | Boot state initializer with five counting loops |
| `func_8006A8D4` | 0x110 | **68** | Memory-region layout (19 pointer assignments) |

## Building

```bash
cd pc_port
mkdir build && cd build
cmake ..
make

# Sanitizer build (ASan + UBSan, static runtimes)
cmake .. -DPE_PORT_SANITIZERS=ON
make
```

Produces:
- `libpe_field_runtime.a` — reusable translated/runtime static library
- `parasite-eve-port` — native executable
- `pe-native-tests` — test suite (current grind lane: 994 tests)
- `pe-field-runtime-link-test` — standalone archive-consumer smoke test

Run both CTest consumers from either the source root or build directory:

```bash
ctest --test-dir build --output-on-failure
```

## Running

```bash
# Headless deterministic (CI/validation; bootstrap-disc fixture)
./parasite-eve-port --headless --bootstrap-disc \
  --max-frames 1 \
  --screenshot /tmp/pe-black.ppm --trace /tmp/pe-boot.trace

# Real Disc 1 boot (image opened read-only, never copied)
./parasite-eve-port --headless \
  --disc-image "/path/Parasite Eve (USA) (Disc 1).bin" \
  --max-frames 1 \
  --screenshot /tmp/pe-black.ppm --trace /tmp/pe-boot.trace

# Real-disc byte-path verification driver: PVD verify → DsSearchFile
# "\PE.IMG;1" → CdPosToInt → bounded guest-RAM load at D_80011614 → poll,
# tracing every value (deterministic for a given image)
./parasite-eve-port --headless \
  --disc-image "/path/Parasite Eve (USA) (Disc 1).bin" \
  --disc-load-test --trace /tmp/pe-disc.trace
# Expected trace: pvd_verify=4, PE.IMG lba=1013 size=206213120,
# issue=1 dest=0x8010BD00 len=32768, poll=0, fnv1a64=7D860391E1ED6C97

# Windowed (requires X11 display)
DISPLAY=:10.0 ./parasite-eve-port --bootstrap-disc --hold-ms 5000 --debug-overlay

# Interactive Day-1 field route (autopilot) — the same route the
# route-boot-day2 harness proves, driven from the normal windowed binary.
# --route-pad installs the harness's deterministic 4-stage pad (shared
# pc_port/include/pe_route_pad.h table) and implies --skip-movie +
# --skip-opening-menu (the untranslated title/menu is in front of the field,
# and no pad can get past it).  Without --route-pad the interactive port has
# no SIO source, so the guest sees no button and cold boot parks at the field
# prefix; that is an input limit, not a stub.
# --auto-quit stops (and leaves the room on screen) at the documented
# executed-route frontier m0004i module 4 (pc=0x801B6CC8).
DISPLAY=:0 ./parasite-eve-port \
  --disc-image "/path/disc1.bin" --route-pad --scale 3
# Expected: m0010i -> m0002i -> m0003i -> m0372i -> m0004i -> m0378i ->
# m0377i -> m0378i -> m0004i, then "[ROUTE] reached m0004i frontier
# (pc=0x801B6CC8)" ~7900 route frames.  PE_ROUTE_DEBUG=1 prints a periodic
# token/story trace.  PE_ROUTE_PAD4=0xFF9F restores the old 3-stage stop at
# m0377i (stage 4 then continues to hold the stage-3 mask).

# Bounded caller-checkpoint report (B54K-M: expect 2/2/2, token 2)
./parasite-eve-port --headless --max-frames 2 --dma-checkpoint-report \
  --disc-image "/path/disc1.bin"

# Strict mode — centralized abort at first unresolved provider
./parasite-eve-port --headless --strict-stubs \
  --disc-image "/path/disc1.bin"
# Current global frontier: exit 1 at func_8007512C from func_801909B4.
# func_8006AD40, both caller DMA checkpoints, and 149 overlay words complete first.
# Bootstrap-disc still stops at func_8007F72C from func_800698D4.

# RNG oracle gate — must equal tools/rng_oracle.py on the retail exe
./parasite-eve-port --headless \
  --disc-image "/path/Parasite Eve (USA) (Disc 1).bin" --rng-oracle-dump

# LZCR oracle gate — must equal tools/lzcr_oracle.py on the retail exe
./parasite-eve-port --headless --lzcr-oracle-dump

# Callback oracle gate — must equal tools/callback_oracle.py on the
# retail exe
./parasite-eve-port --headless --callback-oracle-dump
```

`--bootstrap-disc` and `--disc-image` are mutually exclusive.  Without
either, the retail disc-wait is modeled honestly (bounded by a documented
host adaptation); the boot is not faked.

Normal interactive execution has no artificial frame limit and continues
until host quit or retail/fatal termination. Headless tests should use the
explicit `--max-frames N` budget: exactly N host presentations are admitted,
then execution stops at the next host-safe continuation check. The existing
`--max-main-iterations N` option now bounds exact starts of the outer retail
loop, but cannot bound time spent inside one nested state iteration.

Run the B49 real-disc acceptance harness with:

```bash
python3 tools/b49_host_loop_test.py ./build/parasite-eve-port \
  "/path/Parasite Eve (USA) (Disc 1).bin"
```

## Phase 6E-B21 correction

The raw `func_80071A24` trampoline is A(28h) `bzero(dst,len)`: executable
words `240A00A0 01400008 24090028` at file offset `0x62224`.  A(28h) uses
the A vector `0xA0`; `SysEnqIntRP` is C(02h) via vector `0xC0`.  The checked
provider clears the exact guest byte range and returns the incoming
destination.  `func_80064964` clears `0x800A3060..0x800A317F`, then writes
eight ordered `0xFF` bytes.  Independent contracts are
`tools/b21_bzero_oracle.py` and `tools/b21_order_oracle.py`.

The corrective history preserves provisional `8e90ac7` and incorrect
`14ac77b`; the B22 strict real-disc frontier was `func_80052C6C` from
`func_800527C8` (three identical captures), advanced to `func_8005BCBC`
(B23), to `func_8005D6F4` (B24), and now to `func_8005DC4C` from
`func_8005D6F4` (B25), while bootstrap-disc remains at `func_8007F72C`
by design.  Native tests: 285/285.

## Testing

```bash
cd pc_port/build
./pe-native-tests   # 566 tests (baseline + guest-RAM/Boot Rung/policy
                    # + real-disc: pe_disc fixtures, disc providers,
                    # guest-copy bounds, func_800698D4 sequences
                    # + 6E-B1: func_80070D10 RNG-init rung
                    # + 6E-B2: func_80070D6C/70DD0 RNG model equality,
                    #   wrap quirk, footprints, warm-up integration
                    # + 6E-B3: func_8003E974 write map, func_8003EAC8
                    #   call order/args, idempotence, footprints
                    # + 6E-B4: LZCR helper exactness, func_8003EAC8
                    #   edge/one-hot contract, below-table write, recorder
                    #   semantics, func_8003E974 final registered table
                    # + 6E-B5: func_80036DC8 final record state, per-leaf
                    #   write sets, footprint, idempotence, integration
                    # + 6E-B6: func_80073D24 slot contract, previous-
                    #   handler returns, oracle equality, dispatch order,
                    #   unknown identities, footprint, strict integration
                    # + 6E-B7: func_800371A4 byte-store contract, width
                    #   truncation, single-byte footprint, integration
                    # + 6E-B8: func_80029388 rung — 2F658 record copies,
                    #   20EFC byte clears, slot-table addresses/guards,
                    #   full-RAM footprints, idempotence, integration
                    # + 6E-B9: func_8005BCA8 empty stub — zero footprint,
                    #   dirty-state preservation, 3E680 integration,
                    #   frontier advance to func_80068D28
                    # + 6E-B10: func_80068D28 exact record state, canary
                    #   footprint, idempotence, 3E680 integration,
                    #   frontier advance to func_800124F8
                    # + 6E-B11: func_800124F8 exact cleared state, canary
                    #   footprint, halfword width, idempotence, 3E680
                    #   integration, frontier advance to func_8001A890
                    # + 6E-B12: func_8001A890 exact cleared state,
                    #   stride-4 halfword footprint, idempotence, 3E680
                    #   integration, frontier advance to func_80034F10
                    # + 6E-B13: func_80034F10 exact cleared state, RMW
                    #   mask, 64x redundant store, canary footprint,
                    #   idempotence, 3E680 integration, frontier
                    #   advance to func_8006536C
                    # + 6E-B14: func_8006536C exact table/index state,
                    #   byte-width proof, canary footprint, idempotence,
                    #   3E680 integration, frontier advance to
                    #   func_80038D1C
                    # + 6E-B15: func_80038D1C both return paths, no-write
                    #   path canary, byte-width proof, 3E680 integration
                    #   (dispatcher fully translated), frontier advance
                    #   past 3E680 to func_8006A9E4
                    # + 6E-B16/B54K-AC: func_8006A9E4 rung — 6E6A8 sector
                    #   forwarding/provider guards, 6E7E8 RMW on all
                    #   three poll outcomes, 6E498 hit/miss/empty/packed
                    #   fields/read-only footprint, full patterned run
                    #   (exact bytes, lookup slots, provider order),
                    #   2 MiB canary footprint, RamReset rerun, 3E680
                    #   integration; frontier advance to func_800527C8
                    # + 6E-B24: func_8005BCBC rung — all four return
                    #   paths, dispatcher/record selection paths, exact
                    #   D_8009D0C0/C4/C8 guest stores, widths/guards,
                    #   dirty/repeat/RamReset, strict-mode advance,
                    #   2 MiB canary footprint, 527C8 integration;
                    #   frontier advance to func_8005D6F4
                    # + 6E-B25: func_8005D6F4 rung — return 0xFF, bzero
                    #   range + just-past guards, buffer fills with
                    #   reload-per-iteration loops, boundary ROM order
                    #   (10 providers, three func_8005DC4C first),
                    #   controlled dependency returns, widths/guards,
                    #   dirty/repeat/RamReset, full 2 MiB canary,
                    #   527C8 integration incl. 52C6C→5D6F4 ROM-order
                    #   proof at 0x800C1F80; frontier advance to
                    #   func_8005DC4C from func_8005D6F4)
```

## Deterministic framebuffer

| Property | Value |
|----------|-------|
| SHA-256 (headless, 3 runs) | `fb28dc21dd1e41eb72b8fe22dd3295bb8ed0c040aa88f7885a68dedc2629dfdb` |
| Windowed SHA matches headless | ✅ |
| Trace SHA-256 (3 runs) | `42c1956e077a40fed5176653b6a18938a8a91e99e35fe9d7044f31581de785af` |
| Real-disc load trace (3 runs) | `7b8724acf4d4787f58ca0068e68839f171e2d3f36f72a42f0a4ef03f0041672b` |
| Unsupported traps | 0 |

## Phase 6E-B22 correction

`func_8005DE88` is a translated 23-word resource-list/state initializer at
`0x8005DE88..0x8005DEE3` (file offset `0x4E688`, live split `4CC98.s`). It
links the 12-byte records at `0x800A2090..0x800A217F`, null-terminates the
last link at `0x800A2174`, and initializes `$gp+0x36C..0x380` exactly. It has
no callees and no return value. At the end of B22 the strict real-disc
frontier was `func_80052C6C` from `func_800527C8`, exit status 1; native
tests were 271/271.  The independent write-order oracle is
`tools/b22_5de88_oracle.py`.

## Phase 6E-B24 rung

`func_8005BCBC` is translated retail logic — a resource-state
pointer/count selector: 21 words at executable `0x8005BCBC..0x8005BD0F`
(file offset `0x4C4BC`, live split `4C4BC.s`), every word verified
against the SHA-1-exact executable.  It stores the incoming argument
into `D_8009D0C8` ($gp+0x358), selects a buffer base into `D_8009D0C0`
($gp+0x350) and count `8` into `D_8009D0C4` ($gp+0x354), then returns
`8`.  Selection: a0 != 0 → `0x800C20A4`, +0x10 iff `byte6(a0) == 9`;
a0 == 0 → `0x800C0DE0`, +0x10 iff `D_8009D218 != 0` (the word
func_8005BC98 stores).  Exactly two exe call sites, both discarding the
return: `func_800527C8` @`0x80052834` (a0=0 in the delay slot) and
`func_8004DD64` @`0x8004DF28` (a0 = `D_8009D004`).  No callees, no
SDK/hardware activity, deterministic from guest state; idempotent;
PE_RamReset restores initial conditions (post-reset a0=0 selects
`0x800C0DE0`).  The three state words are guest-RAM resident — retail
readers/writers span func_8005BD10, func_8005BE1C, and the
func_8005D6F4 region.  Independent oracle: `tools/b24_oracle.py`
(delay-slot-aware MIPS-I interpreter over the verified retail words;
condition sampled at branch issue; asserts the full ROM-order
read/write footprint and return for all four selection paths).  At the
end of B24 the strict real-disc frontier was `func_8005D6F4` from
`func_800527C8` (three identical captures), exit status 1; native tests
were 279/279.

## Phase 6E-B25 rung

`func_8005D6F4` is translated retail logic — a resource-buffer +
display-state initializer: 147 words at executable
`0x8005D6F4..0x8005D93F` (file offset `0x4DEF4`, live split
`4CC98.s`), every word verified against the SHA-1-exact executable.
ROM order: `func_80071A24(0x800C0DE0, 0x12E4)` (REAL BIOS A(28h)
bzero); flag + `D_8009D0C8/C0/C4` block-1 stores (order C8, C0, C4);
fill loop #1 writing 0xFF×8 at `0x800C0DF0` with per-iteration guest
reloads of C0/C4; selection #1 reads C8 (always 0 — stored just above,
so the func_8005DC9C arm is statically dead) and calls
func_8005DC4C(0x1E, a1=0xFF residual fill register — retail sets the
copy dest only AFTER the call returns); string copy #1 from the
func_8005DC4C return into D_8009D0C0 until the copied byte == 0xFF;
block 2 re-selects 0x800C0DF0 with store order flag, C4, C8, C0 and
re-fills the buffer (retail overwrites copy #1's bytes — reproduced);
selection #2 + string copy #2; third func_8005DC4C call with the
post-copy-#2 cursor in a1; func_80052594 on its return;
func_8005CCA4; `sh 0x0203 → 0x800C1F80`; `sw 0x00404040 → 0x800C0E44`;
func_800614AC(0x00404040); timer-tick clears 0x800A76A4/B0/BC/C8;
func_8005E884 → r; func_8005E850(0, 8−r); func_800649D0(0);
func_80052790(1); terminator bytes 0xFF at 0x800C20A4/0x800C20B4;
returns 0xFF (sole call site func_800527C8 @0x8005283C, nop delay
slot, return discarded).  func_80071A24 is REAL; the nine remaining
callees are UNRESOLVED and route through the centralized boundary
(func_8005DC4C three call sites, func_8005DC9C dead arm,
func_80052594, func_8005CCA4, func_800614AC, func_8005E884,
func_8005E850, func_800649D0, func_80052790).  The consumed
func_8005DC4C return uses the documented degenerate in-RAM default
0x800C0DF0 (the destination buffer itself — first byte is the
just-written 0xFF terminator, so the copy loop executes and transfers
no fabricated content); tests script real returns via
`Bootstrap_SetIntSequence`.  Independent oracle: `tools/b25_oracle.py`
(delay-slot-aware MIPS-I interpreter; branch conditions sampled at
issue; shared register file; every read/write logged with width and
order; bzero modeled by the proven A(28h) contract; controlled
dependency returns asserted for both the degenerate boot path and a
seeded 4-byte string).  The strict real-disc frontier is now
`func_8005DC4C` from `func_8005D6F4` (three identical captures), exit
status 1; native tests are 285/285.

## Next steps

1. **Next artifact-free production rung:** implement generic PsyQ `MoveImage`
   (`func_8007512C`) and continue the recovered `func_801909B4` overlay
2. **Scheduler rung:** obtain the human-driven BTL151 PCSX packer capture;
   never synthesize `m0360i` or `persist[0] |= 4`
3. Continue boot/frontier coverage from each newly proven provider
4. Audio, input, save/load (later phases)

## Constraints

- PCSX-Redux is the retail oracle only — never used as runtime
- Matching decomp remains separate and SHA-exact (SHA `452fb033`; 335 exact
  C leaves plus 19 explicitly non-exact accepted residuals)
- No PS1 emulator in the native executable
- The Disc 1 image is user-supplied and read-only; never commit it or
  any derivative captures
- This is NOT a playable game yet
