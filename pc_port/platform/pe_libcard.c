/*
 * Host definition of func_800409B4, a game-class function: the game's
 * memory-card subsystem boot init.  Header rewritten 2026-11-27; it replaces
 * the earlier label "libcard: InitCARD + StartCARD / Classification: 2 (SDK
 * host implementation)", which was wrong.
 *
 * Defined here: one global, func_800409B4 (no static function), and the
 * static table kCardEvents.  Class and provider: game-class (audit3b,
 * docs/evidence/public-openness-20261127/audit3b.tsv: classified `game`,
 * MemCard_InitManager in the oracle layout; the audit notes that the body
 * follows the matched leaf of the same name).  tools/analysis/
 * gen_decomp_ports.py has no PE_PLAT_REPLACED row for 0x800409B4, so this
 * file is its provider.  It is not an SDK function: the libapi stubs
 * InitCARD2 / StartCARD2 are 0x8007E314 / 0x8007E324, and the libcard
 * internals the retail function calls (0x8007DDD4, 0x8007DE40) are
 * collapsed no-ops here.
 *
 * Retail structure:
 *   - word guard D_800A1850: skip init when already done.
 *   - EnterCriticalSection; 8x OpenEvent (BIOS B(08h) via func_800726E4)
 *     with classes 0xF4000001/0xF0000011, specs {4,0x8000,0x100,0x2000},
 *     mode 0x1000, handlers func_80042BD8..func_80042C64; handles stored to
 *     D_800BCDA8..D_800BCDC4 (word stride 4).
 *   - func_8007DDD4(0) (_card_init), func_8007DE40, func_800726D4 (A(70h)),
 *     func_8007DD64(0) (A(ADh)): memory-card hardware/kernel bring-up —
 *     collapsed no-ops (no guest-RAM effects).
 *   - 8x EnableEvent (func_80072704) over the stored handles;
 *     ExitCriticalSection.
 *   - unconditionally: sb 0 -> D_800A0ED4+0x418 and D_800A0ED4+0.
 *
 * Kernel Event Control Blocks live outside the 2 MiB guest window; the host
 * event shim (pe_libetc.c) supplies deterministic handles.  The retail
 * handler addresses are preserved verbatim as the OpenEvent argument even
 * though the host never invokes them.
 */
#include "psx_compat.h"
#include "pe_sdk.h"

static const struct {
    uint32_t cls;
    uint32_t spec;
    pe_addr_t handler;
} kCardEvents[8] = {
    { 0xF4000001u, 0x0004u, 0x80042BD8u },
    { 0xF4000001u, 0x8000u, 0x80042BECu },
    { 0xF4000001u, 0x0100u, 0x80042C00u },
    { 0xF4000001u, 0x2000u, 0x80042C14u },
    { 0xF0000011u, 0x0004u, 0x80042C28u },
    { 0xF0000011u, 0x8000u, 0x80042C3Cu },
    { 0xF0000011u, 0x0100u, 0x80042C50u },
    { 0xF0000011u, 0x2000u, 0x80042C64u },
};

void func_800409B4(void)
{
    int i;
    if (PE_LoadU32(0x800A1850u) == 0) {
        PE_StoreU32(0x800A1850u, 1);
        func_80072714();                    /* EnterCriticalSection */
        for (i = 0; i < 8; i++) {
            PE_StoreU32(0x800BCDA8u + (uint32_t)i * 4u,
                        (uint32_t)PE_Event_Open(kCardEvents[i].cls,
                                                kCardEvents[i].spec,
                                                0x1000,
                                                kCardEvents[i].handler));
        }
        /* func_8007DDD4(0), func_8007DE40, func_800726D4, func_8007DD64(0):
         * card hardware/kernel bring-up — collapsed no-ops */
        for (i = 0; i < 8; i++) {
            PE_Event_Enable((int)PE_LoadU32(0x800BCDA8u + (uint32_t)i * 4u));
        }
        func_80072724();                    /* ExitCriticalSection */
    }
    PE_StoreU8(0x800A0ED4u + 0x418u, 0);
    PE_StoreU8(0x800A0ED4u, 0);
}
