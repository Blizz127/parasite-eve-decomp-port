/*
 * Phase 6E-B39 — func_80051CC4: resource command-state initializer.
 *
 * Retail body: 77 instructions / 0x134 bytes,
 * 0x80051CC4..0x80051DF7 (exclusive end 0x80051DF8), file offset
 * 0x424C4.  The live disc1 split is asm/disc1/420A8.s:314-401 under
 * configs/USA/disc1.yaml:298 [0x420A8, asm].  tools/b39_oracle.py contains
 * the complete executable-verified transcription and the 8-entry jump
 * table at 0x800111F8.
 *
 * True contract: void func_80051CC4(void).  No instruction consumes an
 * incoming argument register and all three executable call sites use a nop
 * delay slot with no argument setup:
 *   func_800512AC @ 0x800514C0 (conditional command 12 path)
 *   func_80051E64 @ 0x80052170 (unconditional epilogue call)
 *   func_800527C8 @ 0x80052844 (operation-map position 15; 14th jal)
 * Every caller discards the single void return at 0x80051DF0/0x80051DF4.
 *
 * ROM-order operation map:
 *   1. saved = func_80052F0C(): exact 0/1 comparison of authoritative
 *      D_8009D048 against guest buffer address 0x800C0E48.
 *   2. func_80052E30(0): translated resource-buffer allocate path.  This
 *      reads the OLD D_8009D018 through func_80052F70 before step 3.
 *   3. Clear authoritative D_8009D018 ($gp+0x2A8, retail gp 0x8009CD70),
 *      then clear seven words in descending order at
 *      0x800A1B48,44,40,3C,38,34,30.
 *   4. Signed-byte load from 0x800C0E22 and translated
 *      func_8005332C(source_id).  B40 proves its pe_addr_t record lookup.
 *   5. If the result is nonzero and lbu(result+0x14) is nonzero, scan that
 *      many command bytes at result+0x15.  For cmd = byte & 0x1F:
 *        8/9/10 -> D_8009D018 = 1 << (cmd-8)
 *        11     -> word 0x800A1B30 = 3
 *        12     -> word 0x800A1B34 = 2
 *        13     -> word 0x800A1B44 = 0xFFFFFFFE
 *        14     -> no operation
 *        15     -> word 0x800A1B34 = 0xFFFFFFFE
 *      All other command values are no-ops.  The loop uses the retail
 *      signed index<count comparison; count is an unsigned byte (0..255).
 *   6. Invoke func_8005218C(). B44 translates all seven stat outputs;
 *      EQP1 completes the capacity and level publication.
 *   7. func_80052E30(0), then func_80052E30(saved), preserving the retail
 *      resource-buffer restore order and all partial state at a boundary.
 *
 * Direct persistent guest writes are seven aligned words covering the exact
 * byte range 0x800A1B30..0x800A1B4B. Their fixed addresses are valid 2 MiB
 * guest RAM; every access is a checked PE_StoreU32. The retail prologue also
 * saves $ra/$s0 at incoming_sp-4/incoming_sp-8 and reloads them at return;
 * native C correctly uses its host ABI stack rather than duplicating those
 * transient frame accesses in guest RAM. Data-dependent guest reads are
 * checked PE_LoadU8 operations at result+0x14 and result+0x15+i. There are
 * no direct hardware/SDK calls, callbacks, multiplication, division,
 * unaligned access, host pointers in guest RAM, clamping, fallback pointers,
 * or low-address mirrors. The complete call graph does not poll or block.
 *
 * Classification: 1 — translated retail resource/table initialization.
 */
#include "psx_compat.h"
#include "pe_sdk.h"
#include "pe_bootstrap.h"

#define GA_B39_PARAM_BASE  0x800A1B30u
#define GA_B39_PARAM_LAST  0x800A1B48u
#define GA_B39_SOURCE_ID   0x800C0E22u

void func_80051CC4(void)
{
    uint32_t saved_buffer_kind = func_80052F0C();
    pe_addr_t record;
    uint32_t count;
    uint32_t i;

    func_80052E30(0u);

    D_8009D018 = 0u;
    for (i = 0u; i < 7u; i++) {
        PE_StoreU32(GA_B39_PARAM_LAST - i * 4u, 0u);
    }

    record = func_8005332C(
        (int32_t)(int8_t)PE_LoadU8(GA_B39_SOURCE_ID));
    if (record != 0u) {
        count = PE_LoadU8(record + 0x14u);
        for (i = 0u; (int32_t)i < (int32_t)count; i++) {
            uint32_t command = PE_LoadU8(record + 0x15u + i) & 0x1Fu;
            switch (command) {
            case 8u:
            case 9u:
            case 10u:
                D_8009D018 = 1u << (command - 8u);
                break;
            case 11u:
                PE_StoreU32(GA_B39_PARAM_BASE + 0x00u, 3u);
                break;
            case 12u:
                PE_StoreU32(GA_B39_PARAM_BASE + 0x04u, 2u);
                break;
            case 13u:
                PE_StoreU32(GA_B39_PARAM_BASE + 0x14u, 0xFFFFFFFEu);
                break;
            case 14u:
                break;
            case 15u:
                PE_StoreU32(GA_B39_PARAM_BASE + 0x04u, 0xFFFFFFFEu);
                break;
            default:
                break;
            }
        }
    }

    func_8005218C();
    func_80052E30(0u);
    func_80052E30(saved_buffer_kind);
}
