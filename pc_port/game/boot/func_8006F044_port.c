/*
 * func_8006F044 — port of the matched C (src/func_8006F044.c; audit item 11,
 * audio2 lane).  Called from func_8001220C on the 0xAA108448 branch.
 * Resets the six D_800B0DB2..DB7 bytes to -1, clears D_800B0CD8 bits
 * 0x000000F0, stops all sound (func_80086FF8, AKAO 0xF0), then streams the
 * two resident blocks back (D_8009315E / D_80093166 LBA pairs into
 * D_8001160C / D_80011610), each under the retail read/poll retry and the
 * 72714/726C4/72724 critical-section + cache-flush sequence.  The earlier
 * psx_compat.h stub only recorded a bootstrap boundary.
 *
 * Host adaptation: retail's 8-byte result buffer is a stack local; the
 * host passes the guest scratch func_8006E7E8 uses (0x801FFEA0; the two
 * never nest).
 */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "pe_sdk.h"
#include "game_port.h"

#define GA_SYNC 0x801FFEA0u

static void reload_block(pe_addr_t table, pe_addr_t dest_ptr)
{
    for (;;) {
        int r;
        uint32_t lo = PE_LoadU16(table), hi = PE_LoadU16(table + 2u);
        do {
            r = func_8006E6D4((int)(PE_LoadU32(0x800B0DD8u) + lo), 0,
                              PE_LoadU32(dest_ptr), (int)(hi - lo));
        } while (r == -1 && !PE_Port_ShouldStop());
        for (;;) {
            int t = func_800811E4(GA_SYNC);
            if ((unsigned)(t + 1) < 2u)
                PE_StoreU32(0x800B0CD8u, PE_LoadU32(0x800B0CD8u) & 0xFEFFBFFFu);
            if (t == 0 || PE_Port_ShouldStop())
                goto done;
            if (t == -1)
                break;          /* retail: goto retryN (re-issue the read) */
        }
    }
done:
    func_80072714();
    func_800726C4();
    func_80072724();
}

void func_8006F044(void)
{
    for (uint32_t a = 0x800B0DB2u; a <= 0x800B0DB7u; a++)
        PE_StoreU8(a, 0xFFu);
    PE_StoreU32(0x800B0CD8u, PE_LoadU32(0x800B0CD8u) & ~0xF0u);
    func_80086FF8();
    reload_block(0x8009315Eu, 0x8001160Cu);
    if (PE_Port_ShouldStop()) return;
    reload_block(0x80093166u, 0x80011610u);
}
