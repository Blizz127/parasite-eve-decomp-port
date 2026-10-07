# Changelog

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
