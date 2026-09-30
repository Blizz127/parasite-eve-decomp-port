/*
 * Phase 6E-A — libcard: InitCARD + StartCARD.
 *
 * func_800409B4 (asm/disc1/307CC.s @ file 0x311B4).
 * Classification: 2 (SDK host implementation).
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
