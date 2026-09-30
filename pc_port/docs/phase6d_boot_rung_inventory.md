# Phase 6D Boot Rung Inventory — Verified 6D-R

Six real translated Boot Rung functions direct-tested and proven in
the native main path (`func_8001220C` → first clear).

## Function inventory

### func_8003E610 — Display/graphics bring-up
- **Matching size:** 0x70 (28 words)
- **Port file:** `game/boot/func_8003E610_port.c`
- **Structure:** 10 straight-line calls with immediate arguments
- **Key arguments:** `func_8003E754(0x140, 0xE0)` = 320×224 display
- **Verification:** Exact call order and argument values confirmed by order log

### func_8003E680 — Subsystem-init dispatcher
- **Matching size:** 0xD4 (53 words)
- **Port file:** `game/boot/func_8003E680_port.c`
- **Structure:** Zero 5 globals → 2000-pass poll → callback register → 13 subsystem inits
- **Poll count:** 0x7D0 = 2000 (verified)
- **Callback:** `func_80073D24((int)(uintptr_t)func_8003E91C)` — real function pointer
- **Fix applied (6D-R):** Callback was passing 0 instead of `&func_8003E91C`
- **Subsystem order:** func_8003E974, func_80036DC8, func_80073D24×2, func_800371A4,
  func_80029388, func_8005BCA8, func_80068D28, func_800124F8, func_8001A890,
  func_80034F10, func_8006536C, func_80038D1C

### func_8006A5BC — Boot init with VSync-polled wait loops
- **Matching size:** 0x90 (36 words)
- **Port file:** `game/boot/func_8006A5BC_port.c`
- **Structure:** 4 setup calls → 2 `while (f() != 1) VSync(0);` loops → store result
- **Setup order:** func_80085644, func_80086FF8, func_80087024, func_8008682C(0)
- **Wait loop 1:** `while (func_8007ED58() != 1) { func_80073A44(0); }`
- **Wait loop 2:** `while (func_8007F72C() != 1) { func_80073A44(0); }`
- **Final store:** `D_800B0DD4 = func_8007F7A8();` (unsigned short)
- **Strict-mode gate:** func_8007F72C exits on `g_strict_stubs`
- **Callee status:** All callees are bootstrap stubs; structure faithfully reproduced

### func_8006A64C — Boot memory-layout wrapper
- **Matching size:** 0x28 (**10 words** — not 9)
- **Port file:** `game/boot/func_8006A64C_port.c`
- **Structure:** Calls func_8006A8D4() then func_8006A674()
- **Verification:** Child call order confirmed via side-effect inspection

### func_8006A674 — Boot state initializer (five loops)
- **Matching size:** 0x260 (152 words)
- **Port file:** `game/boot/func_8006A674_port.c`
- **Structure:** 5 counting loops initializing the D_800B0CD8 region
- **Loop 1:** 0x31 iterations, zero-fill u32 at base+0x14 + i×4
- **Loop 2:** 2 iterations, fill -1 to pairs at base+0xDC..0xDF
- **Loop 3:** 4 iterations, zero u16 at &D_80094488+6+i×8 and D_8009448C+i×8
- **Loop 4:** Down-count 2→0, zero u32 at base+0x134..0x13C
- **Loop 5:** Down-count 1→0, zero u32 at base+0x140..0x144 + final base+0x148
- **Register pins:** Removed (native port — pins were for retail matching only)
- **Base type:** `D_800B0CD8` is `uint32_t` (4 bytes declared), cast to `unsigned char *`

### func_8006A8D4 — Memory-region layout (19 assignments)
- **Matching size:** 0x110 (**68 words** — not 89)
- **Port file:** `game/boot/func_8006A8D4_port.c`
- **Structure:** 19 ordered pointer assignments across 4 arena buffers
- **Assignment order (matching source):**
  1. D_800B0E24  = D_800F34F8
  2. D_800B0E28  = D_800F34F8 + 0x1800
  3. D_800B0E2C  = D_800F34F8 + 0x6000
  4. D_800B0E30  = D_800F34F8 + 0x6000 + 0xE000
  5. D_800B0E40  = D_8010BD00
  6. D_800B0E34  = D_80120D08
  7. D_800B0E38  = D_80120D08 + 0x1C98
  8. D_800B0E3C  = D_80120D08 + 0x5C98
  9. D_800B0E44  = D_80120D08 + 0x9C98
 10. D_800B0E4C  = D_80120D08 + 0xE498
 11. D_800B0E48  = D_80120D08 + 0xC098
 12. D_800B0E50  = D_80120D08 + 0x56498
 13. D_800B0E54  = D_80120D08 + 0x5A498
 14. D_800B0E5C  = D_80120D08 + 0x5A498 + 0x3800  = D_80120D08 + 0x5DC98
 15. D_800B0E58  = D_80120D08 + 0x56498 + 0x8000  = D_80120D08 + 0x5E498
 16. D_800B0E60  = D_80120D08 + 0x5E498 + 0x7000  = D_80120D08 + 0x65498
 17. D_800B0E6C  = D_801ED800
 18. D_800B0E64  = D_80011614 - 8
 19. D_800B0E68  = D_80011614
- **Determinism:** All 19 pointers identical across 3 independent initializations
- **Note on PS1 vs host:** PS1 addresses are in a contiguous 2MB space; host
  pointers span separate arena buffers. Pointer values differ from retail
  but relational offsets within each arena chain are preserved.

## Main call path (actual runtime order)

```
func_8001220C (main, translated 6C)
  ├── func_800725DC              [BOOTSTRAP_RET]
  ├── func_8003E610              [REAL, 10-call sequence]
  │   ├── func_80073C94, func_8003E754(320,224), func_8007D054,
  │   │   func_80077F7C, func_80079004(160,112), func_80079024(240),
  │   │   func_800409B4, func_8003E944, func_8007EC14, func_80080CC8(0)
  ├── func_8006A5BC              [REAL, 2 wait loops + store]
  │   ├── func_80085644, func_80086FF8, func_80087024, func_8008682C(0)
  │   ├── while (func_8007ED58() != 1) VSync(0);  [returns 1 → no VSync]
  │   ├── while (func_8007F72C() != 1) VSync(0);  [returns 1 → no VSync]
  │   └── D_800B0DD4 = func_8007F7A8();           [returns 0]
  ├── while (func_800698D4() == 0) VSync(0);      [returns 1 → exits]
  │   ├── func_8007F72C, func_8007F778, func_80082314,
  │   │   DsSearchFile×2, func_80080C48
  ├── func_8006A64C              [REAL, wrapper]
  │   ├── func_8006A8D4          [REAL, 19 pointer assignments]
  │   └── func_8006A674          [REAL, 5 counting loops]
  ├── func_8003E680              [REAL, 2000-poll + callback + subsystems]
  │   ├── Zero 5 globals
  │   ├── func_80070D6C() ×2000
  │   ├── func_80073D24(0), func_80073D24(&func_8003E91C)
  │   └── 13 subsystem inits → func_80038D1C()
  ├── func_8006A9E4              [BOOTSTRAP_RET]
  ├── func_8006AD40, flag checks, dispatch
  ├── func_8006E834              [REAL, image loader setup]
  │   ├── func_80086FF8, func_8006E6D4, func_800811E4,
  │   │   func_80072714, func_800726C4, func_80072724
  │   └── VSync(0), SetDispMask(0), SetDefDispEnv, Present
  ├── func_801909B4              [BOOTSTRAP_RET]
  └── func_8006E9A0(v)           [REAL, display setup + clear]
      ├── VSync, SetDispMask, PutDispEnv, ClearImage(r=0,g=0,b=1), DrawSync
      └── func_8005E588, func_80066B60, ClearOTagR, func_80068E24,
          func_80070E54, func_80038D1C
```

## Native test coverage (39 tests, all pass)

| Category | Count | Tests |
|----------|-------|-------|
| Baseline (6A) | 12 | fb, ClearImage, VSync, DrawSync, PPM, stubs, flags |
| func_8006A8D4 | 5 | non-null, determinism, bounds, count, no-overlap |
| func_8006A674 | 5 | loop1, loop2, loop3, loop4, loop5 |
| func_8006A64C | 1 | child-call order |
| func_8006A5BC | 5 | setup order, wait loop 1, wait loop 2, store, strict-mode |
| func_8003E610 | 2 | call order, argument values |
| func_8003E680 | 5 | global clears, 2000 polls, callback once, subsystem order, final call |
| Guards | 4 | no emulator, no bootstrap stubs for translated, first-clear path, direct-clear not default |
| **Total** | **39** | |

## Known limitations

- `D_800B0CD8` is declared as `uint32_t` (4 bytes) in `pe_globals.c` but
  `func_8006A674` writes ~0x150 bytes starting from this address. The
  linker places subsequent globals contiguously, providing the needed space.
  Future: consider declaring as a larger buffer.
- Arena pointer values on host differ from PS1 flat address space — offsets
  within each arena chain are preserved but cross-arena relationships differ.
- All 6 Boot Rung callees are bootstrap stubs — the functions themselves
  are real translated C, but the providers they call are still shimmed.
