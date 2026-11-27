/*
 * Phase 6E-B47 — func_800851A8: SPU DMA upload (extended prefix)
 * (58 retail words / 0xE8 bytes, exe 0x800851A8–0x8008528F,
 * file offset 0x759A8, live split asm/disc1/74FB0.s:800–863;
 * all 58 instruction words verified exact against the SHA-exact
 * retail executable).
 *
 * Retail signature:
 *   int func_800851A8(pe_addr_t buffer, int transfer_count)
 *
 * ROM-order operation map:
 *   Prologue: save $s0/$s1/$s2/$s3/$ra on 0x28-frame.
 *   $s0 = a0 (buffer), $s3 = a1 (count).
 *
 *   1. func_80085174() — completion barrier (spin while D_8009D24C == 1).
 *
 *   2. func_80085084(buffer) — magic number check (5 instructions):
 *      v0 = *buffer + 0xB0BEB4BF.  Returns 0 if valid.
 *      Inlined.
 *
 *   3. If v0 != 0: store -1 to D_8009D24C, return -1 (error path).
 *
 *   4. If v0 == 0 (success path):
 *      a. Read 4 transfer params from buffer+0x10..+0x1C.
 *      b. Call func_80085EB4(param0) — SPU address validation (translated).
 *      c. Compute copy source/destination/size from params.
 *      d. Call func_800850F4(copy_src, param1) — DMA transfer (HARDWARE).
 *      e. Copy payload words to D_800B2900.
 *      f. If count != 0: call func_80085174() again (completion barrier).
 *      g. Return 0.
 *
 * Classification: 1 — translated retail logic (extended prefix).
 * The completion barrier, magic check, error path, SPU address
 * validation, parameter reads, copy computation, payload copy, and
 * second barrier are all deterministic retail logic.
 * The DMA transfer (func_800850F4 → func_80085E54 → func_8007D9F8)
 * routes through the controlled asynchronous DMA4 event provider.
 *
 * Dependency boundary:
 *   func_80085174  TRANSLATED (inlined: spin on D_8009D24C)
 *   func_80085084  TRANSLATED (inlined: magic number check)
 *   func_80085EB4  TRANSLATED (SPU address validation)
 *   func_800850F4  TRANSLATED (DMA issue; completion is a later event)
 */
#include "psx_compat.h"
#include "game_port.h"
#include "pe_plat/spu_dma_hw.h"

/* ── Guest globals ──────────────────────────────────────────────────── */
#define GA_D_8009D24C  0x8009D24Cu   /* transfer completion flag          */
#define GA_D_800B2900  0x800B2900u   /* transfer data buffer              */

/* Magic number constant for buffer validation. */
#define PE_851A8_MAGIC  0xB0BEB4BFu

/* func_80085084: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80085084_port.c (src/func_80085084.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/* ── func_800851A8: SPU DMA upload (extended prefix) ─────────────────── */
int func_800851A8(pe_addr_t buffer, int count)
{
    uint32_t check;
    pe_addr_t s0;
    uint32_t param0, param1, param2, param3;
    uint32_t v0, s1, copy_words;
    uint32_t i;

    /* 1. func_80085174 — completion barrier.  A stop ends the run: return
     *    retail's success value with no guest write (no -1 busy store, no
     *    issue, no payload copy). */
    PE_SpuDma_WaitForCompletion();
    if (PE_Port_ShouldStop()) return 0;

    /* 2. func_80085084 — magic number check. */
    check = PE_LoadU32(buffer) + PE_851A8_MAGIC;

    /* 3. Error path. */
    if (check != 0) {
        PE_StoreU32(GA_D_8009D24C, 0xFFFFFFFFu);
        return -1;
    }

    /* 4. Success path: read transfer params from buffer+0x10. */
    s0 = buffer + 0x10;
    param0 = PE_LoadU32(s0);       /* SPU address */
    s0 += 4;
    param1 = PE_LoadU32(s0);       /* size */
    s0 += 4;
    param2 = PE_LoadU32(s0);       /* offset */
    s0 += 4;
    param3 = PE_LoadU32(s0);       /* mode/count */

    /* 4a. SPU address validation (func_80085EB4 — translated). */
    func_80085EB4(param0);

    /* 4b. Compute copy source and word count from retail param layout.
     *     If param3 == 0: default 0x100, source = s0 + 0x24.
     *     Else: source = s0 + 0x24, count = (param3 - param2) << 4. */
    if (param3 != 0) {
        s1 = s0 + 0x24;
        v0 = param3;
    } else {
        s1 = s0 + 0x24;
        v0 = 0x100;
    }
    copy_words = (v0 - param2) << 4;

    /* 4c. DMA transfer (func_800850F4 — translated DMA issue).
     *     Retail calls func_800850F4(s1 + ((v0 - param2) << 6), param1).
     *     It returns busy; an explicit completion barrier services the IRQ. */
    func_800850F4(s1 + ((v0 - param2) << 6), param1);
    if (PE_Port_ShouldStop()) return 0;

    /* 4d. Copy payload to D_800B2900.
     *     Retail: v1 = param2 << 6, dest = D_800B2900 + v1.
     *     Copy `copy_words` words from s1 to dest. */
    {
        pe_addr_t dst = GA_D_800B2900 + (param2 << 6);
        for (i = 0; i < copy_words; i++) {
            PE_StoreU32(dst + i * 4, PE_LoadU32(s1 + i * 4));
        }
    }

    /* 4e. Second completion barrier if count != 0. */
    if (count != 0) {
        PE_SpuDma_WaitForCompletion();
    }

    return 0;
}
