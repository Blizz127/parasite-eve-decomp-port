# Boot to Black — Phase 6D-S milestone

## What was achieved

A native Linux executable (`parasite-eve-port`) routes translated `main`
(`func_8001220C`) through six real Boot Rung functions and reaches the first
deterministic native clear frame without an emulator.  Phase 6D-S moves all
guest-visible state onto a host-safe memory foundation: a contiguous 2 MiB
guest RAM with `pe_addr_t` typed addresses, bounds-checked translation only
at real access sites, a full-width callback registry, and a centralized
bootstrap/strict-mode policy.

The Boot Rung has been verified and hardened in Phase 6D-S:
- 81 native tests (12 baseline + 69 guest-RAM / Boot Rung / policy), all passing
- ASan + UBSan builds clean (tests and headless run)
- 3 deterministic headless runs producing identical framebuffer SHA-256
- Strict mode centrally exits at first unresolved provider (func_800725DC)
- Windowed mode produces framebuffer matching headless
- Matching repo preserved at SHA `452fb033` (227 C leaves)

No PS1 emulator is involved at runtime. PCSX-Redux is only the retail oracle.

## Boot trace (actual runtime, Phase 6D-S)

```
0001 native_executable_start
0002 call_func_8001220C
  → func_800725DC              ← BOOTSTRAP_RET
  → func_8003E610              ← REAL TRANSLATED (28 words, 10 call sequence)
  → func_8006A5BC              ← REAL TRANSLATED (36 words, 2 wait loops)
  → func_800698D4              ← BOOTSTRAP (disc mount)
  → func_8006A64C              ← REAL TRANSLATED (10 words, 2-cal child)
  → func_8003E680              ← REAL TRANSLATED (53 words, 2000 polls + callback)
  → func_8006A9E4              ← BOOTSTRAP_RET
  → func_8006AD40              ← BOOTSTRAP_RET
  → func_8006E834              ← REAL TRANSLATED (91 words, image loader)
  → func_8006E9A0              ← REAL TRANSLATED (display setup + clear)
    → ClearImage(0,0,320,240,0,0,1) ← THE BLACK FRAME
0003 func_8001220C_returned
0004 shutdown_begin
0005 shutdown_end
```

## Deterministic framebuffer proof

| Property | Value |
|----------|-------|
| Dimensions | 320×240 |
| Format | RGB 8:8:8 |
| SHA-256 (headless, 3 runs) | `fb28dc21dd1e41eb72b8fe22dd3295bb8ed0c040aa88f7885a68dedc2629dfdb` |
| Windowed SHA matches | ✅ |
| main iterations | 1 (all runs) |
| VSync count | 2 (all runs) |
| func_8003E680 poll count | 2000 (all runs) |
| Unsupported traps | 0 |

## What this milestone does NOT prove

- Disc I/O (bootstrap disc mode only — Phase 6E)
- Real PE.IMG asset loading (Phase 6E)
- MDEC/logo rendering (Phase 6F+)
- Any playable game state
- Audio, input, or save/load
