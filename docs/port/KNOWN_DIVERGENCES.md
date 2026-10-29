# Native port: known divergences from retail

**Purpose: preservation** — the port preserves the game exactly as it shipped.

**Standard (owner, 2026-09-28): the port must behave 1:1 like retail —
gameplay, logic, visuals and audio. No approximations, invented behaviour or
silent cuts.** The only allowed differences are:

1. the platform layer (host rendering, audio output, input, disc access),
   which must reproduce PS1 hardware behaviour faithfully, and
2. opt-in mods/cheats, off by default and inert when off.

Everything else that differs from retail is a divergence and is listed here
with its reason until it is fixed. "Matches retail" is only claimed with
objective evidence: a byte-matched C path, a numeric comparison against the
matched code and disc data, or an emulator/video diff. Anything not checked is
marked **untested**.

Baseline table last comprehensively updated: 2026-09-29. Its historical open
count of 21 has not been recomputed. Current additions follow.

## GPU jump-table dispatch and stale-row reconciliation (2026-10-26)

- A mismatched libgpu jump-table slot now takes retail's `jalr` through the
  guest-code registry instead of a named boundary. LoadImage and StoreImage
  keep a boundary (host RECT) and now stop instead of answering 0. Production
  is unchanged. See `docs/evidence/native-gpu-jtb-20261026/REPORT.md`.
- Rows #1-#4, #16 and #32 below were re-measured on int3 with the tools named
  in each row:
  - #1-#3 carry current counts;
  - #4 is measured statically by the callback censuses;
  - #16's listed bodies all resolve to native registry rows;
  - #32's room half is closed and its EXE half stays open.
- **New, open: ClearOTagR terminator seed.**
  - `func_800752AC_port.c` seeds `D_8009580C = 0x004957F8`
    (`GA_OTC_FLAG_BIT 0x00400000u`). Retail seeds `0x040957F8`: see
    `src/func_800752AC.c:14`, the EXE's `lui 0x0400` at `0x80075330`, and the
    ClearOTag adapter's correct value.
  - So the port's DrawOTag walk ends at a size-0 node with an out-of-RAM link.
    Retail ends at a size-4 node: a `GP0(80h)` 2x1 VRAM self-copy at (0,0),
    then four GP0 NOPs at `D_800957F8`.
  - Three tests and an oracle mock pin the wrong value.
  - Found by the review in `docs/evidence/native-gpu-jtb-20261026/REPORT.md`.
    Fixing it changes every frame's GPU stream, so it gets its own slice.

## Native TTY closure and current census (2026-10-26)

The interrupted native printf callback binding is completed, with B(3Fh)
corrected to literal puts. GPU debug branches now continue after a native
print; unknown hooks retain the stop policy. Six generated callers no longer
bind BIOS TTY calls to boundaries; generated boundary sites fall 31 -> 25.
The generator's mixed pointer/integer printf-argument adaptation is corrected.
See `docs/evidence/native-tty-20261026/REPORT.md` for tests and limits.

The structural table below is historical. Current lexical EXE categories are
1039 decomp-derived, 1124 hand-translated, 35 stub and 55 absent across 2253
configured C spans. “Stub” includes complete native bodies with defensive
boundary branches. These counts do not prove execution coverage or matching
status. The remaining generated sites and the existing five non-CD proof
obligations stay open; no natural campaign or full native claim is added.

## Additional native and fidelity gaps (2026-10-23, int3)

- **Liberate coverage:** static level-32 Liberate exposed missing ovl_03C9
  callbacks `0x801EDC44` / `0x801EEBC8`. Native C and fingerprint bindings are
  now integrated at `345d9d2db`, with caller oracle and actual host-dispatch
  initialization/update proof. Full ability lifecycle, rendering/audio/timing
  and scene validity of existing sound/child guards remain unverified. See
  `docs/evidence/pe-liberate-native-20261023/REPORT.md` and INTEGRATION.md.
- **ovl_03C9 effect child `0x801F0068`** (effect descriptors 0x6E-0x71) is now
  behaviour-equivalent native C (lifted and fingerprint-bound, not matched
  decompilation).
  - Proved:
    - e21 differential: 0 mismatches over 388 of 394 words; the remaining 6 are
      dead in retail;
    - a real-chain spawn/claim/tick dispatch test.
  - Descriptors 0x6E-0x71 are Liberate's per-hit effects (PE 19, command 406,
    learned at level 32). No other direct issuer exists in the EXE or PE.IMG.
    Still open: a field-script operand path (room scripts not scanned), and
    indirect calls through an address built other than as a literal word or a
    `lui` pair.
  - The draw pass ran with real callees in a fixture test, under the port's
    stop policy: 40 frames, all three wait paths, 823 GPU polygons executed
    through DrawOTag's validator. There was no stop, no new boundary and no
    bootstrap return, and a byte-flip negative control fails it.
  - Not proved:
    - the mesh renderer's gouraud and quad groups;
    - rendering and audio output against retail;
    - a natural-play Liberate cast.
  - See `docs/evidence/ovl03c9-effect-801F0068-20261024/`.
- **Credits CD cadence:** fixed at `76a8dc555`.
  - Device time now advances to each CD deadline and VBlank edge, with IRQ
    service between them: 2.5 sectors per 564480-cycle frame at double speed.
  - `func_8007B9EC` runs the retail acknowledge loop.
  - One bounded assisted credits diagnostic read 8794 XA sectors with zero
    underruns; the earlier RUN A had `cd-underruns=4395`. Audible output has not
    been observed.

  Remaining divergences:
  - *Single pending sector.* An acknowledged but unread sector is superseded on
    the first device-time advance after the acknowledge. psx-spx says a data
    request issued shortly after the acknowledge still belongs to the old
    interrupt, but does not say how long that window is.
    - So if a CD IRQ waited at least one sector period, the port would stop at
      `cd_stream_port.c:368` where retail delivers the sector.
    - The post-cadence re-review found no default-production window for that
      (`docs/evidence/cd-site-rereview-20261023/FOLLOWUP-CD368.md`).
    - The read head also waits behind a pending response or command phase,
      where hardware keeps reading and skips sectors.
  - *Sector period.* The port uses 225792 cycles per sector. psx-spx gives an
    average of 224466 (0x36CD2), measured on a PAL PSone.
  - *Flush during a command issue.* A libds command issued while an INT1 is
    pending acknowledges and drops that sector without a callback. Retail's
    `func_8007B9EC` does the same; that retail also loses the sector is inferred.
    In the credits this needs the INT1 to be pending inside a VBlank dispatch.
    No ovl_0457 TU reads the `D_80172D00` ring by symbol.
  - *`pe_cdreg.c:218` (`CD_device_getlocp_spun_down`).* A GetlocP sent with the
    motor stopped stops the port, where retail gets an INT5 error and continues.
    - No default credits path issues one (re-review cdreg218, medium
      confidence).
    - A Disc 2 image trimmed to the last file extent would reach it at the end
      of the credits; retail would also hang on that image.
    - psx-spx gives no error byte for this case, so it is not modelled.
- **EX credits lead-out:** production host pad refill was fixed at `cd2710c18`.
  Waiting beyond the physical image extent hit a device boundary at the
  reviewed tip. Fixed by `51095e7f3`: INT4 DataEnd stops the drive; an EX host-pad
  regression consumes it and exits natively. Integrated suite 1730/1730 and
  normal-boot image/audio parity pass. Real-time credits/audio parity remains open.
- **Coverage:** the boundary review has 53 completed independent verifications
  across 76 classified sites. Ten verifiers failed and thirteen sites were not
  selected. GPU packet validation remains unknown. A completed orchestration
  is not a fully verified absence of reachable stops.
  - Post-cadence re-review at `76a8dc555`: all 26 pe_cdreg.c and pe_libcd.c
    boundary sites were re-verified, with no failed verifier.
    - 24 were UPHELD. 2 were REFUTED, as class corrections only.
    - Final production reach is "no" for all 26; cdreg117 and cdreg218 are at
      medium confidence.
    - A three-lens follow-up on `cd_stream_port.c:368/:372` found no
      default-production window.
  - The six other extra sites the classifiers named were verified at
    `6ee1588fa` (workflow `wf_c7d62efd-15d`). All six have production reach
    "no" at high confidence:
    - `cd_stream_port.c:591`, `:862` and `:1014`;
    - `func_8001220C_port.c:111`, now classed dormant;
    - `pe_irq_delivery.c:83-97`;
    - `pe_callback.c:~150-161`.
    - See `docs/evidence/cd-extra-sites-20261023/`.
  - *Port-only nested IRQ dispatch.* `HostFB_VSync` ->
    `PE_Port_ServiceDmaIrqCheckpoint` -> `PE_IRQ_ServicePendingForGeneration`
    ignores `D_800945E6`. It runs when an MDEC decode is valid and GPU DMA2 is
    pending, and both can happen during credits images and movies.
    - A temporary probe found 0 nested entries in the 1200-frame boot and in the
      full suite.
    - Real-disc ASSISTED-DIAGNOSTIC runs with counters:
      - P3 movies-on route (5 movies): the checkpoint branch ran 70405 times,
        all in main context; 0 inside a dispatch; 0 nested entries.
      - Credits warp: the branch never ran.
    - Not proven absent; the EX credits image path was not run. It remains
      disclosed (`docs/evidence/nested-dispatch-probe-20261023/`).
  - See `docs/evidence/cd-site-rereview-20261023/`.

See `docs/evidence/boundary-retriage-int3-20261023/REPORT.md` for original
classifications, completed overrides, scope limits and saved evidence paths.

## Structural (whole classes of code)

| # | Divergence | Evidence / reason | Status |
|---|---|---|---|
| 1 | 1126 EXE functions run as **hand-translated** ports, not the matched decomp C (108,254 words); 1039 functions / 32,937 of the 149,632 matched words (22.0%) come from decomp-derived C. 2026-10-26 figures on int3; the 2026-09-29 baseline was 1167 hand / 946 decomp-derived | `tools/analysis/pc_port_coverage.py` | Being switched to matched C in route-tested batches; each hand port is unverified until switched or diffed |
| 2 | 33 lexical "stub" and 55 absent EXE functions (2026-10-26; 40 / 88 on 2026-09-29). "Stub" means the function contains any boundary call, defensive branches of otherwise complete ports included. 39 of the 55 absent are inline-asm leaves the generators skip; some run under other names (e.g. `func_80079754` as RotMatrixYXZ in `platform/plat_gte.c`) | same tool | Replace with matched C or faithful ports |
| 3 | 445 Psy-Q-classified functions are linked into the runtime (976 guard findings in 303 places, 2026-10-26; 438 on 2026-09-29) (libcd/libds, libgpu, libgte, libetc, libpress, libspu, libpad, libcard) — via generated decomp and hand translations | `tools/analysis/psyq_port_guard.py --binary … --require-exe` | To be replaced by PsyCross/own platform code (`docs/ai_context/PSYQ_PORT_REPLACEMENT.md`); also a licensing blocker for public release |
| 4 | Overlay (room/battle) code: coverage not yet measured the same way | `tools/analysis/overlay_callback_census.py`, `tools/analysis/room_callback_census.py` | **Measured statically** (2026-10-24/25): every own-blob code reference in the six subsystem overlays and the 185 code rooms has a native path (0 uncovered). This is reference coverage, not executed coverage |
| 32 | Matched C that uses Psy-Q inline GTE macros (`inline_c.h` shapes: `gte_ldv0`, `gte_rtps`/`rtpt`, `gte_stsxy`/`stsz`, SetRot/SetTransMatrix, CompMatrix, nclip, avsz3/4, …) cannot reach the port through the generators: `gen_decomp_ports` / `gen_overlay_ports` reject every inline-asm body as `asm`. That blocks the RoomLib room-effect handlers (RoomLib_HandlerA–F, hundreds of room references, e.g. m0022i's 8018F630), EXE `func_800E051C` (once matched), the 94 EXE leaves flagged `inline GTE/asm`, and the M34 phase-2 effects (hand-translated line-for-line for #16 instead). Those functions run as hand ports or are skipped. | `gen_overlay_ports.py --check --verbose` (`skip …: asm`) | **Room half closed (2026-10-26):** E21 lowering emits pe_gte primitives; `gen_overlay_ports.py --check` finds all 2951 deduplicated room bodies eligible, and m0022i's 8018F630 is a generated TU. **EXE half open:** `gen_decomp_ports.py --check --e21` skips 113 EXE leaves as inline asm and lowers 3; the other 110 run as hand translations (69), are absent (39) or are stub-bucket (2). `docs/evidence/native-gpu-jtb-20261026/kd-reconcile.txt` |

## Platform layer (hardware behaviour not yet faithful)

| # | Divergence | Reason | Status |
|---|---|---|---|
| 5 | Movies are fed by a host-side stream producer instead of the modelled CD drive's interrupt-driven delivery; ~2 s read-ahead cap; no seek/spin-up timing | `pc_port/platform/pe_str_feed.c` | Open |
| 8 | SPU main volume assumed 0x3FFF; SPU IRQ and CD-DA not modelled | `pe_spu.c` | Open |
| 9 | The field-VM host argument frame lives in guest RAM at 0x801FF400 (port-only scratch) | `func_80017018_port.c` | Also covers GA_POLY/GA_DESC/GA_EA_SLOT, moved out of the overlay window into the same audited block. Audited: nothing else writes it (`PE_VMFRAME_AUDIT=1`, bad=0 over ~1.6M opcodes); retail uses the real stack instead, so it stays listed until the port runs the matched VM |
| 36 | Host display (X11 window) is not a model of video scan-out: the displayed 320x240 frame is scaled to the window by the X server (XRender, nearest-neighbour, aspect-correct letterbox with black bars; exact pixel replication at integer scales, nearest sampling at non-integer ones such as 1920x1200) instead of a CRT's analog output, and a blit whose pixels equal the frame already on screen is skipped (a movie's repeat presents, the idle-VBlank rescans of a static screen). `PE_WINDOW_SCALER=cpu` (or a server without XRender/24-bit depth) uses the CPU nearest-neighbour scaler (Win32 always does); the fallback reason is printed once on the `[WINDOW]` line. Both scalers sample the source at the destination pixel centre with the same 16.16 arithmetic (`HostWindow_NearestStep`/`NearestIndex`), so they put identical pixels on screen | `host_window.c`, `host_window_scale.h`, `host_window_win32.c` | Display only: no effect on guest state, presents, the VBlank clock or audio. XRender vs CPU: XGetImage of the window equals `HostWindow_ScaleBGRA` pixel for pixel at 1920x1200, 2560x1600 (pillarbox), 300x800 (letterbox), 1280x800, 1920x1080, 1366x768 (build/lanes/fmv5/xrprobe.txt). Pause dim still shown (former #30): on the oracle pause.pad schedule the paused headless frame matches the oracle paused frame on 99.45% of the pixels whose pre-pause frames agree, and 25 consecutive 2 fps window grabs match the paused VBlank shot within 6 px (build/lanes/fmv5/m/pause_headless/diff.txt, m/pause_win/dimcheck.txt). Fixes slow movies on large panels: FMV002 (68.1 s movie, 1021 frames, dropped=0) at 2560x1600 fullscreen on a Release build takes 73.6 s with XRender vs 261.4 s before, under the same machine load (the CPU fallback: 69.4 s at 1920x1200). Needs an optimised binary: `scripts/build_pc_port.sh` now defaults to `CMAKE_BUILD_TYPE=Release` and `package_release.sh` refuses an unoptimised build (an empty build type left FMV002 at 222.6 s for the 68.1 s movie even with this fix) |
| 31 | CD load timing: the port loads the New Game → FMV002 data ~400 VBlanks faster than the emulator oracle (seek cost is a flat 0x100000 cycles in `pe_cdreg.c`; host-synchronous reads cost ~0 emulated time). Every later event is shifted relative to retail by it (e.g. RNG state at the first encounter) | audio3 2026-09-29: oracle `newgame.pad` replayed on the port with the true VBlank clock (`PE_PAD_SCRIPT_CLOCK=vblank`): aligned through the FMV001 skip (VBlank 3100/3101) and the title menu; FMV002 frames identical (RMSE 0.7) at oracle 6250 ↔ port 5850 and 6350 ↔ 5950; the gap is the black load after the New Game confirm (port black 3650→~3790, oracle still black at 4163). Related to #5. The oracle's own CD timing is a model: to be cross-checked against the retail video before a fix | Open |

## Game behaviour (open bugs, observed by the owner or tests)

| # | Divergence | Evidence | Status |
|---|---|---|---|
| 13 | Aya's gun is not drawn when she readies it before the rat fight | owner report | Open |
| 15 | Plaza: the date's walk to the door with Aya after the dialogue (retail part 01 @ 5:16) not yet observed in the port | owner report; emulator oracle | **Untested past the dialogue**: every date state observed matches retail (arrival anims 0B→00→01→00 confirmed after the turn fix; plaza anim 04 standing identical to the oracle through VSync 16500, `persist[0]` bit 2 clear in both). The walk needs the player input the video used; parked (cosmetic) |
| 16 | Unported room-effect callbacks are skipped with a `[DAY1_SHIM] … not ported` line, so those effects do not draw. Remaining on Day 1: room m0022i (theater door 8, optional, no key award) effect module 0x46. Its init/update/destroy (8018F1D4/8018F500/8018FE98) have matched C but no port, and its behaviour `o->fn` = 8018F630 is `RoomLib_HandlerD` (matched, vendor-derived, inline GTE; see #32). The former "A8001148" (m0012i) line was that module's finalizer 8018FF34 (`return 0`), which now resolves; the former "M0359I 8018F238" item was a worktree-build artifact (overlay fingerprints missing; see docs/evidence/port-16-m34p2-effects/REPORT.md). Both are behaviour-identical to retail. **M34 phase 2 (#16):** 80190E98 (beam), 801914A0 (beam segment update, 2e6f7a66), 80191A98 (beam floor glow) and 80191FE0 (ember burst) have matched C (src/overlays/room_m0348i/) and run as line-for-line ports in `m34_boss_effect_port.c` (`PE_M0348iEffectChild`), with the glow fade 80191DB0 and ember motion 801922E0 (matched C) that they expose; their inline `gte_CompMatrix` macro is reproduced op-for-op on pe_gte (tests `M348G_mvmva_words`, `M348G_comp_matrix`). The floor glow's renderer **func_800C4FC4 is a hand port of unmatched asm** (B57C4.s, 0x574, handwritten GTE ring renderer) in `func_800C9C20_port.c`; evidence: native test `M348G_c4fc4_ring` (analytic GTE inputs) and the retail-video comparison (docs/evidence/port-16-m34p2-effects/REPORT.md); it is unverified beyond that. Its RotTransPers4 `p`/`flag` outputs are dropped (the retail caller never reads sp+0x40/0x44; the port has no guest stack frame there). Not ported: the masked-body twins in room_m0174i (80191FB0 / 801925B8 / 80192BB0 / 801930F8; not on the Day-1 route). | port logs, w16 | **Native bodies exist (2026-10-26):** m0022i 8018F1D4 / 8018F500 / 8018FE98 / 8018F630 / 8018FF34 and the four m0174i twins each resolve to exactly one fingerprint-matched registry row by the room callback census rule (`docs/evidence/native-gpu-jtb-20261026/kd-reconcile.txt`). The callback dispatchers take the `[DAY1_SHIM]` skip only when `PE_GuestCode_Resolve` fails, so it does not apply to them. Whether those effects draw like retail is untested |
| 17 | Eve's flash: the first state-0 draw after the effect-stack model loses the borrowed caller-stack bytes uses the owner joint position, not the original value. The `[DAY1_SHIM]` log names the cause. Every writer the Day-1 route reaches is bound from retail bytes since 2026-10-20 (CDD0C plus the frameless update leaves of the three weapon tables). The 049784820 route logged `DAY1_SHIM 0` up to its known game over at 44602 (NATIVE_COMPLETION.md); later play is unverified. The framed update callback 800CDFE0 is also bound on branch claude/pe-cdfe0-20261020 (frame above the borrowed window; docs/evidence/flash-cdfe0-20261021/REPORT.md): the aid-assisted movies route went from 2 hits, both 800CDFE0, to 0. There is no oracle frame for the flash itself | `m0023i_effect_port.c` | Open |
| 37 | Retail reads stack it never wrote: RotMatrix (`func_80079754`) fills only a matrix's 3x3, and the code then loads that matrix's translation. The 0x4A0 effect body in m0065i/m0087i/m0391i/m0393i/m0399i/m0401i loads it as GTE TR. m0034i/m0174i `func_8018F434`/`func_8018F67C` `gte_CompMatrix(&o->m, &m20, &o->m)` uses `m20.t`, so `o->m.t` follows the residue and effect placement can differ. The native guest-frame residue is different. | e21 sweep 2026-10-17 (MISMATCH GTE data 25 / RAM; `docs/port/NATIVE_COMPLETION.md`) | Open: a fix needs recorded retail stack contents (runner `--recorded` snapshot), not zero-fill |
| 18 | Police-siren ambience after Carnegie Hall | owner report (unsure) | **Untested** — to be checked against the retail video first |
| 19 | Frame pacing paces one host frame per emulated VBlank at 59.94 Hz | `host_window.c` | Believed faithful (240 Hz sequencer tick measured retail-exact); window-side timing **untested** on the owner's PC beyond his playtest |
| 20 | Second Eve battle: the music's vocal/choir layer is missing (instrumental plays) | owner report on r2; reproduced on the r2 binary (vocal-window band r 0.30 vs retail) | **Not reproduced on the current tree** (full-fight 600–1500 Hz band correlation vs retail part 02 @ 1:23–2:20: median r 0.58, vs 0.21 at a wrong offset). r2 lacked them (reproduced). Not caused by EA100/101 (a controlled run with them suppressed keeps the vocals) or func_8006F044 (not on the Day-1 path). The r2→current change that restored them is unidentified. Regression check: `pc_port/tools/audio/eve2_vocal_check.sh` (current 0.59 PASS, r2 0.30 FAIL) |
| 26 | The field-VM dispatcher `func_80017018` is a hand port; its inline opcode re-implementations are being switched to the generated matched handlers (batch 1: 42 opcodes done, suite + route green; unlisted opcodes now use the guest-code registry instead of stopping) | shadow audit | In progress (port3); note the matched `src/func_80017018.c` itself has mislabelled case labels (see ACTIVE_HANDOFF) |
| 28 | Disc swap: `func_80069B08` (insert-disc prompts, lid open/close poll, re-validate) is a stub, so past the disc-1/disc-2 story boundary (story ≥ 0x258) the port skips the prompt and keeps reading disc 1. **Not on the Day 1 path** (0 hits on the route) | gfx2 audit item 26 | Closed in this lineage: `func_80069B08` is a hand port (`game/boot/func_80069B08_port.c`) with the drive-tray model `platform/pe_drive_tray.c`. pe-laterdays (2026-10-21) drove the Disc 1→2 swap natively (`[TRAY]` lines), but only as aided coverage (dev warps), not play evidence; no oracle comparison of the swap screen yet |
| 29 | Dithering of flat textured semi-transparent polygons (e.g. floor-shadow edges): the port dithers them, the PCSX-Redux oracle does not; psx-spx does not specify it (DuckStation dithers) | gfx2 first-room oracle diff (98.90%) | **Untested** against real hardware; kept as is |
| 35 | Render_DrawEntity (func_8003AF14) hand port retains extra guards and different flag dispatch vs matched C: null entity guard, packet-RAM guard before func_8003B144, combined flag-9 lighting arm and hoisted func_8003C0B4. Lighting consumer corrected 2026-10-26: all six production func_8003B97C_lighting_cut sites now call the existing generated matched-source func_8003B97C; hand lighting/empty cuts retired. Retail scratchpad writes and empty-model BK/RGBC ordering restored. | docs/evidence/native-lighting-20261026/REPORT.md; final e21 384 runs, GTE checked, 217/217 words | Partially open: AF14 flag/guard migration and retail pixel-oracle parity remain; parent/candidate first-room/battle A/B evidence is not retail pixel proof |

## Fixed (kept for the record)

- Two game globals were split between a host copy and the guest word, so hand-written writers and generated (matched-C) readers disagreed: `D_8009D018` (the generated `func_80051E58`, live on the field-message path, always read 0) and the story word `D_800A7918` (the main loop's volume gate in `func_8001220C` always saw story < 0x258). Both now live only in guest memory; proof tests `pc_port/tests/test_split_brain.h` fail without the fix. A sweep found no other split globals (found and fixed 2026-09-29, never separately listed).
- Pause screen not drawn: while paused, the port kept the last presented frame instead of retail's dimmed pause screen. The port now rescans the GP1(05h) display area from VRAM on every VBlank without a PutDispEnv, as the GPU does (display only; no present, no route-clock effect). Paused frame vs the emulator oracle: 99.2% identical on the unchanged background; luminance port 38.46→20.37→38.46 vs oracle 35.85→17.79→36.31; real-time windowed check dims and resumes. Harness: `PE_PAD_SCRIPT_CLOCK=vblank` lets scripted input reach a paused game (keyed on the emulated VBlank count, `PE_GPU_VSyncQuery`, so oracle `.pad` scripts replay frame for frame) (former #30).
- Damage "far too high in both directions" (owner report on r2): **no divergence on the current tree**. The combat path equals the matched C; Aya's persistent stats, inventory and equipment at the first field room are byte-identical between a normal movies-on New Game (real title, opening movie, name entry) and route mode (only name-entry UI residue differs; build/lanes/day1/ayarec/mem_diff.txt). Hits-to-kill match or are harder than retail (first rat: port 2 commands, retail part 01 @ 15:53–16:03 one); Eve hits Aya for 5–11. r2 still ran the hand-cut battle tick, since replaced by the matched C (66d5f407), so the owner's r2 observation is not re-checked on r2 itself (former #11).
- With movies on, the port stopped at the end of Day 1 (story 0x80, after the car ride). Cause: `func_80076C34` (libgpu command-ring enqueue) cut retail's full-ring wait (`while (full) { if (func_80077404()) return -1; func_80076EE4(); }`) and stopped on the first full 64-entry ring (first attributed to StoreImage `func_800768A0`; a fatal-path diagnostic there never fired). Fixed 2026-09-29: the full-ring path now waits as retail does (`func_80077404` normal half, DMA/IRQ checkpoint, pump); the movies-on run continues into Day 2 (story 0x90) with no boundary, and the Day 1 closing narration and Day 2 opening audio match retail part 02 @ 18:17–18:42 and part 03 @ 0:06 (former #27).
- Retail's per-VBlank game callback `func_8003E91C` (RNG advance + the four timer ramps incl. play time) now runs every VBlank in normal play, as verbatim retail TUs; rates identical to the emulator oracle (frame counter +1/VSync, play time +600 per 600 VSyncs). Saves now record real play time (former #29).
- `func_8006FC18` (Pm_Stop) now follows the matched C: calls the destroy callback, returns its result, frees the slot; 0 destroy-callback skips over the whole Day-1 route (former #27).
- Game Over timing: no divergence. Mode-3 phases 1–5 match retail per phase to the VSync (emulator oracle vs port: 48/142/62/162/120/2, 536 VSyncs mode 3 → restart in both). The earlier +304-frame figure included the port-side forced-death detection lag (former #28; re-check after #29).
- Memory-card saves persist: host 128 KiB .mcd behind our own BIOS card-file functions (clean-room from psx-spx); save → relaunch → load → resume verified in-game (former #10). Play time saves as 00:00:00 until #29 is fixed.
- Particle/effect sprites as streaks (Eve's spells, rats): `func_8006914C` skipped retail's effect-module inits, so the effect texpage table was never filled; now runs them (matches retail part 01 @ 9:45–9:58). (former #12)
- Pause kept audio running: `func_8006A0E8` was a stub; now sends the retail AKAO pause/resume (verified in a real-time window). (former #14)
- Game Over: field side (ops 0x31/0xED, `func_8006A25C`, BIOS event calls) and battle side (`func_8002B29C`, `func_8006CC68` tail) now the matched C; fade → green caption → restart → FMV001 → title, same order as the emulator oracle. Timing matches retail per phase (see below). (former #21)
- Battle tick `func_800299CC` now a complete translation of the matched C (was hand cuts). (former #22)
- Battle mode-3 death path now the matched C (item 0x12 consume, countdowns, enemy walk). (former #23) **Correction 2026-10-26:** the revive arm the tick calls, `func_8002AA98`, was a hand port of phase 0 of 8 until 2026-10-26. A death with item 18 held (or record +0x4C bit 0x800) therefore stayed in mode 3: the item was never consumed and HP never restored, where retail revives Aya at half HP. It is now the generated TU of `src/nonmatching/exe/func_8002AA98.c`, which the e21 differential checks against the retail EXE (`docs/evidence/native-revive-20261026/`). Whether item 18 can be obtained in play, and what sets bit 0x800, were not established.
- `func_80024A3C` now all 17 phases from the matched C. (former #24)
- Field op 0xED now the full `func_80016910`. (former #25)
- MDEC: output of a previous decode command is now drained in FIFO order per psx-spx (was dropped); unit test added; not reached on Day 1 (former #6).
- CD/XA audio level: the drive volume (CdMix/ATV) follows the game's own writes, and the SPU main/CD volume comes from the retail SpuSetCommonAttr call; level gap logo−FMV002 −7.61 dB vs retail video −7.94 dB, FMV002 fade-out within ~1 dB (former #7).
- Wrong movie on New Game/opera (movie id slot overwritten by the overlay load) — fixed, owner-confirmed "videos playing perfectly".
- Missing room sound banks (room-loader sound tail cut) — fixed, owner-confirmed "sound is great"; footstep timing/loudness matched to the retail video.
- Scripted turns finishing in one tick (turn opcode returned 1 instead of retail's 0) — fixed against the matched C.
- Game ran at 2× speed (per-present pacing) — fixed.

Test-only route aids (HP lock, auto-fight) run only under `--route-pad` and are
not divergences of the game; the normal-start check (`day1_route_regress.sh`,
exit 3) enforces that they are off.

## 2026-10-26 CdSync callback continuation

B290's non-null ready/sync hooks now dispatch through the existing native CD
callback dispatcher. Four synthetic regression groups fail on the parent and
pass with this change; installed CdInit callbacks, registered-target arguments,
unknown targets and nested stop propagation are covered. The lexical hand
boundary census is 221 -> 219. This does not close B290's timeout diagnostics or
prove natural campaign execution of these hooks. Full evidence and limits:
`docs/evidence/native-cdsync-20261026/REPORT.md`. F1 remains awaiting decision.

## 2026-10-26 SPU DMA4 completion callback without abort()

`PE_SpuDma_Service` follows `func_8007D614`'s arms on the live `D_8009B434`:
the identity stays native, any other value goes through the guest-code
registry with retail's `$a0 = 0xF0000000`, and an unresolved one is a
stopping boundary instead of `abort()`. A null slot with no enabled event is
`DeliverEvent`'s retail no-op (only SpuInit opens that event on disc 1). The
wait loop and its callers return on a stop; a busy flag with no transfer
pending stops (`SPU_DMA/STALL`) instead of aborting. Issue no longer refuses
by callback value. The hand boundary census is 219 -> 220 (the new boundary;
aborts were never counted). Still divergent: `func_800850F4`'s issue-path
aborts (programmed-I/O arm, bad shift, refused issue), the generated
`func_80085174` literal spin, and the `absent_cd_port.c` twin of `7D614`.
Evidence: `docs/evidence/native-spudma-20261026/REPORT.md`. F1 still awaits
its decision.

## 2026-10-26: m28/m0089 effect callback identity (C8)

The common m28 movement predicate also holds in m0089i. Three unrelated
m28 effect branches now additionally match the complete callback body before
selecting a hand port. m0089i's 801924F8 therefore reaches its existing
fingerprint-matched native registry row, rather than the corridor particle
implementation. The two other addresses are latent identity holes in that
room; no reachable call to them is claimed. Evidence and synthetic coverage
limits: `docs/evidence/effect-room-identity-20261026/REPORT.md`.


## 2026-10-26 SPU wait and in-flight issue

The generated85174 literal spin is replaced by the serviced native wait.
850F4 and reverb clear deliver an earlier pending DMA4 before changing its
callback slot.850F4 issue aborts are named stopping boundaries; programmed-I/O
remains unimplemented. Suite1770/1770 and route/boot regression receipts pass;
interrupt timing and all transitive caller stop handling are not proven.
Evidence: `docs/evidence/native-spuwait-20261026/REPORT.md`. Prior SPU entry's
issue-abort/literal-spin list is historical. Other native/distribution holds stay.
