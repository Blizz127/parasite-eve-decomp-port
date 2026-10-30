# Changelog

## 2026-10-30 — source sync, no new build

Source only. No binary, release, tag or asset. Synced from the development
tree at `4863cb9c2` (previous source sync: `41ecb77e3`, 2026-10-29), same rule as
before.

**Native PC port (source): replacing Psy-Q SDK code**

- Psy-Q functions linked into the game binary: 280 -> 264 (counted by
  `tools/analysis/psyq_port_guard.py` against the allowlist).
- New host implementations written from the psx-spx hardware documentation,
  not from SDK source: BIOS rand/srand, GetODE (GPUSTAT bit 31).
- ResetCallback / InterruptCallback rewritten clean-room
  (`pc_port/platform/pe_plat_irq_callback.c`). A separate author who never saw
  SDK source or the earlier translation wrote it from 336 input/output vectors
  captured from the original code
  (`pc_port/tools/pe_irq_callback_vectors.py`, checked by
  `pe_irq_callback_vectors_check.c`) and from psx-spx.
- SetTexWindow / SetDrawArea rewritten the same clean-room way
  (`pc_port/platform/pe_plat_gpu_area.c`) from 15902 vectors
  (`pc_port/tools/pe_gpu_area_vectors.py`, `pe_gpu_area_vectors_check.c`).
- Existing host models (BIOS exit/FlushCache/ChangeClear*, critical
  sections, SetIntrMask, VSync callbacks) are now bound under `pe_plat_*`
  names.
- Test fixes: test binaries whose link-time hooks named renamed symbols,
  standalone GPU tests missing a link stub, and one stale blend expectation.

**Not published in this sync (held)**

- `pc_port/CMakeLists.txt`: the change does not apply cleanly to the curated
  public copy. As before, the PC port does not build from this repository
  alone. The generator also names providers that are not published.

## 2026-10-29 — source sync, no new build

Source only. No binary, release, tag or asset. Synced from the development
tree at `41ecb77e3` (previous source sync: `2918a7f0`, 2026-10-07) with the
same rule as before: unchanged public files are replaced, curated files get
the change as a patch, and files that define Psy-Q SDK functions, include
backend headers, or carry retail-derived test data stay unpublished.

**Decompilation**

- Matching EXE functions published as C: 1772 -> 1773 (one new game
  function since the previous sync). Psy-Q functions stay unpublished; their
  bytes still come from your disc at build time.
- Overlay C spans published: 3321 -> 3498 (room and subsystem overlays).
- New `src/nonmatching/`: native-port C for functions that are not
  byte-matched yet. Each is checked by differential tests against the
  original code; none counts as matching progress.

**Native PC port (source)**

- The game binary still runs only native C: no MIPS interpreter or CPU
  emulation in production. The original-code interpreters exist only in test
  programs as references. An unported call stops with
  `CPU_BOUNDARY/REFUSED`; it is never emulated.
- Large hardware-model work: GPU (status, DMA, image uploads, CLUT cache),
  SPU (ADPCM, envelopes, reverb, noise), MDEC, CD, timer 1, SIO0 and DMA
  controllers, each with focused checks.
- Missing-code stops now propagate through the loaders, battle and render
  paths instead of continuing on stale state.

**Not published in this sync (held)**

- 17 new `pe_plat_*` SDK-replacement sources and headers. Some were written
  from matched Psy-Q source and are quarantined in the development tree;
  the rest wait for a provenance audit.
- Enhancement and assist features (volume, presentation scale and
  similar), as before.
- Nine curated files whose change didn't apply cleanly as a patch (for
  example `pc_port/CMakeLists.txt` and `pc_port/tests/test_native.c`), and
  two analysis harnesses that define Psy-Q functions.
- The published tree alone doesn't build the PC port; it never did.

**Validation (development tree, 2026-10-29)**

- Native test suite: 1871 run, 1865 passed, 0 failed, 6 skipped (the tests of
  the quarantined adapters).
- Day 1 with pad input only (no memory writes) wins the first Eve fight and
  reaches the sewer alligator's second phase, then loses: the automated
  player runs out of healing. This is an assisted route, not someone
  playing.
- Days 2 to 5: warp-started assisted segments reach story flag 0x278 (past
  the Disc 2 change) with no missing-code stop. These are coverage
  diagnostics, not play.
- Not run on real hardware or by hand.
- Published tree checks: `psyq_port_guard.py --strict` 0 findings;
  `retail_data_guard.py --strict` on all 715 added or changed files, no
  retail bytes; private-content scan clean.
- Turnkey matching build from this exact tree, run from a clean copy with
  the documented Docker steps: `VERIFY_US=PASS`, retail SHA-1
  `452fb033f2eaa4b18aa20a5bca60b8125af3a37b` reproduced, all 1773 packed C
  spans equal retail.

Public binaries stay held: the PC-port runtime still links 280
Psy-Q-classified functions (the Psy-Q guard's runtime allowlist rows),
which the project is replacing with its own code.

## 2026-10-08 — native-only room code (release `native-only-ab6de50b`)

- Room functions that read pointers from room data, call through code
  pointers or use inline GTE asm are now generated as native C from the
  matched decompilation (guest-memory lowering plus GTE asm lowering onto the
  native GTE model). Byte-identical room twins resolve through a closure
  fingerprint.
- Warp sweep at story 0x90: 377 of 402 room slots run clean, 4 rooms still
  stop (m0159i, m0256i, m0349i, m0432i), 1 scripted return to title (m0050i),
  20 unused. Day 3 probes (m0092i–m0098i, m0191i, m0360i) ran clean (story
  flag only, not played).
- Rooms that reach unported code now stop with `CPU_BOUNDARY/REFUSED`
  instead of silently skipping it.
- Tested headless on a real disc image: Day 1 route PASS, Day 2 movie check
  PASS, test suite 1636/1636 with the disc. Untested by hand past Day 1 and
  not yet on real hardware.

## 2026-10-07 — native-only fix (release `native-only-328511c8`)

- Fixes the stop about 12 minutes into Day 1 in `native-only-2918a7f0`.
  The native sound tick reached sound command 0x9B (`func_8008C720`), which
  the hand-written queue code didn't handle. Commands without a hand case now
  run their matched C (`pc_port/platform/pe_stream_commands.c`).
- A stop for missing code now prints `[STOP] … requested by: +0x…`. Most of
  the disc, graphics and boot code that can stop the game printed nothing
  before.
- Tested headless on a real disc image: the Day 1 route passes to the end of
  Day 1, and the Day 2 movie check reaches the opening of Day 2. The test
  suite passes 1634/1634 with the disc present. Not yet played on real
  hardware; Day 2 and later are unverified.

## 2026-10-07 — native-only port (release `native-only-2918a7f0`, UNTESTED)

- The game binary runs only native C. The sound driver's tick no longer runs
  retail code in a MIPS interpreter. The interpreter is now a test-only
  reference (`pc_port/tests/pe_akao_interp_oracle.c`).
- An unported call stops the game with `CPU_BOUNDARY/REFUSED pc=… ra=…`
  instead of being emulated. `PE_BOUNDARY_POLICY=record` logs the call and
  continues, for diagnosis only.
- New disc-free checks against the matched C: `pe-leaf-oracle-tests` and
  `pe-akao-tick-oracle-tests` (`pc_port/tools/build_leaf_oracle.py`). The
  whole sound tick shows 0 mismatches over 8,785 random cases.
- Source synced from the development tree at `2918a7f0`, using the same rule
  as the first snapshot: port files that define Psy-Q-classified functions or
  include backend headers stay unpublished. Some changes of this build
  therefore live in files that aren't published, among them
  `akao_tick_port.c`, `absent_sdk_port.c`, `absent_hi2_port.c`,
  `pe_stream.c` and `pe_bios_string.c`.
- Status and the list of calls that still stop the game:
  [docs/port/NATIVE_ONLY_STATUS.md](docs/port/NATIVE_ONLY_STATUS.md).

## 2026-09-28 — repository history rewritten

- Removed from all history with git filter-repo: 635 blobs containing retail
  game bytes (embedded tables, instruction words, hex dumps), decoded game
  script text, and 82 game-frame screenshots. The current tree was unchanged
  by the rewrite (tree 22901f1f…).
- All 40 branches were force-pushed. Existing clones and forks keep the old
  objects: re-clone, and do not push old branches back.
- Verification: `build/lanes/hist/verify_rewrite.py` over every blob of every
  rewritten branch reported 0 blobs with retail bytes, script text or game media.
- Going forward, `tools/analysis/retail_data_guard.py` runs in the build, ctest,
  the pre-commit hook (`git config core.hooksPath .githooks`) and CI.
