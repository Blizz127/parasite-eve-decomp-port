# Phase 6D-R changelog (non-regression items only)

1. **Phase 6D-R pass title:** "Verify and harden translated Parasite Eve boot rung"
   — base aecddf9, branch phase6d-r-verify-boot-rung, not pushed.

2. **Fixed func_8003E680 callback stub** (`game/boot/func_8003E680_port.c`):
   `func_80073D24(0);` → `func_80073D24((int)(uintptr_t)func_8003E91C);`.
   The `Stub_Record` trace that pretended to be a callback-registration was
   also removed — Stub_Record is not a callback mechanism.

3. **Removed orphan duplicate sources** (`pc_port/game/`):
   `func_8001220C_port.c`, `func_800698D4_port.c`, `func_8006E834_port.c`,
   `game_port.h`.  The `pc_port/game/func_800698D4_port.c` copy was stale
   (lacked the strict-mode gate); `pc_port/game/func_8006E834_port.c` had
   divergent global declarations.  CMakeLists.txt already referenced only the
   `bootstrap/` originals — these were dead code.

4. **Added strict-mode gate to func_8007F72C** (`bootstrap/func_800698D4_port.c`):
   without it, `--strict-stubs` entered an infinite VSync loop inside
   func_8006A5BC's second wait loop.  Now exits 1 and names
   `func_8007F72C (CdReady)`.

5. **Corrected func_8006A64C size:** 10 words (0x28 bytes), not 9.
   Both `README.md` and `phase6d_boot_rung_inventory.md` updated.

6. **Corrected func_8006A8D4 size:** 68 words (0x110 bytes), not 89.

7. **Added 27 direct native tests** (12 → 39 total, all pass):
   func_8006A8D4 (5), func_8006A674 (5), func_8006A64C (1),
   func_8006A5BC (5), func_8003E610 (2), func_8003E680 (5), guards (4).

8. **Added ordered call log** (`stub_registry.c/.h`): `g_stub_order_log`,
   `Stub_ResetOrderLog()`.  Records every Stub_Record call in order, enabling
   exact-sequence verification in tests (func_8003E610 10-call order,
   func_8003E680 subsystem order, etc.).

9. **Three deterministic headless runs:**
   `fb28dc21dd1e41eb72b8fe22dd3295bb8ed0c040aa88f7885a68dedc2629dfdb` —
   identical SHA-256, equivalent traces, main_iters=1.

10. **Strict mode:** exits 1, "first unresolved provider:
    func_8007F72C (CdReady)".  No false black-frame success.

11. **Windowed run** (`DISPLAY=:10.0`): framebuffer SHA matches headless;
    visible native window with title progression; Escape/close functional;
    no emulator.

12. **Matching repository preserved:** SHA `452fb033`, 229 C leaves,
    `verify_us.sh` exit 0, no native files in matching repo.
    `git status --short` unchanged from before the verification.

13. **Code footprint:** +1126 / −180 lines across 10 files (8 modified + 2 new
    docs); 4 orphan files removed from `pc_port/game/`.
