/*
 * Phase 6E-B54K-Y — overlay-local func_80192CE8.
 *
 * Complete retail function: [0x80192CE8,0x80192F98), 172 words, SHA-256
 * ed89408bffde43742781c89f1d16b7a98c52dc165b67cafd62cfdcf9319e03e7.
 * Translated through first-frame setup (91FB8 + 924F8), 3EB04, the
 * continue-frame worker (92934), skip check, and 70E54.  Finite host
 * budgets (PE_Port_SetMovieContinueBudget) stop at
 * func_80192CE8_media_loop; default -1 runs until B0DBA==0.  Epilogue
 * clears busy bit 0x200 on D_800B0CD8 and returns $s3 (0 unless skip).
 */
#include "psx_compat.h"
#include "game_port.h"
#include "pe_sdk.h"

#include <stdio.h>
#include <stdlib.h>

#define FMV_DBG(...) do { if (getenv("PE_FMV_DEBUG")) fprintf(stderr, "[FMV] " __VA_ARGS__); } while (0)

#define GA_READ_DEST       0x8001160Cu
#define GA_ARENA           0x80011610u
#define GA_READ_TABLE      0x8009315Eu
#define GA_PEIMG_LBA       0x800B0DD8u
#define GA_OVERLAY_FLAGS   0x800B0CD8u
#define GA_RECORD_BASE     0x801D0E04u

extern int PE_func_80191FB8_Values(int count, const pe_addr_t *sources);

/* HOST_ADAPTED: retail libpad fills the BIOS pad buffer from the VSync
 * interrupt; the port fills it from the host pad source right before the
 * guest's func_8003EB04 read (same contract as func_8003F3C4's field
 * tick), so title/movie loops see window or scripted input. */
static void HostPadFill(void)
{
    if (PE_Port_HasPadSource()) {
        PE_StoreU16(0x800BE9A0u, 0x4100u);
        PE_StoreU16(0x800BE9A2u, PE_Port_ReadPadRaw());
    }
}

static void ClearMovieArenas(void)
{
    PE_StoreU32(0x801D0DE8u, 0u);
    PE_StoreU32(0x801D0DECu, 0u);
    PE_StoreU32(0x801D0DFCu, 0u);
    PE_StoreU32(0x801D0DF8u, 0u);
    PE_StoreU32(0x801D0DF0u, 0u);
    PE_StoreU32(0x801D0DF4u, 0u);
    PE_StoreU8(0x800B0DBAu, 0u);
}

int func_80192CE8(int index)
{
    uint16_t start;
    uint16_t end;
    uint32_t stream;
    int status;
    int result = 0;

    PE_StoreU32(GA_OVERLAY_FLAGS,
                PE_LoadU32(GA_OVERLAY_FLAGS) | 0x200u);
    PE_StoreU8(GA_RECORD_BASE + (uint32_t)index * 20u, 1u);

    func_80074D28(0); /* SetDispMask */
    (void)func_80074DC0(0); /* DrawSync */
    (void)func_80074A44(1); /* ResetGraph light path */

retry_issue:
    FMV_DBG("92CE8 issue index=%d\n", index);
    start = PE_LoadU16(GA_READ_TABLE);
    end = PE_LoadU16(GA_READ_TABLE + 2u);
    do {
        status = func_8006E6A8(
            (int)(PE_LoadU32(GA_PEIMG_LBA) + start),
            PE_LoadU32(GA_READ_DEST),
            (int)((uint32_t)end - (uint32_t)start));
    } while (status == -1);

    for (;;) {
        status = func_8006E7E8();
        if (status == 0)
            break;
        if (status == -1)
            goto retry_issue;
    }

    FMV_DBG("92CE8 read done\n");
    (void)func_80072714();
    func_800726C4();
    func_80072724();

    stream = PE_LoadU32(GA_ARENA) +
             ((uint32_t)(PE_LoadU16(0x80093162u) -
                         PE_LoadU16(0x80093160u)) << 11);
    (void)PE_func_80191FB8_Values(1, &stream);
    if (PE_Port_ShouldStop())
        return -1;

    FMV_DBG("92CE8 91FB8 done stream=%08X\n", stream);
    (void)func_801924F8((int16_t)index);
    FMV_DBG("92CE8 924F8 done BA=%u BC=%d\n", PE_LoadU8(0x800B0DBAu), (int16_t)PE_LoadU16(0x800B0DBCu));
    if (PE_Port_ShouldStop())
        return -1;

    /* 80192E08: enter media loop when B0DBA != 0. */
    if (PE_LoadU8(0x800B0DBAu) != 0u) {
        for (;;) {
            int16_t decoded;
            int step;

            decoded = (int16_t)PE_LoadU16(0x800B0DBCu);
            if (decoded <= 0)
                break;

            if (!PE_Port_ConsumeMovieContinue()) {
                Bootstrap_ReturnVoid("func_80192CE8_media_loop",
                                     "func_80192CE8");
                PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
                return -1;
            }

            HostPadFill();
            func_8003EB04();
            if (PE_Port_ShouldStop())
                return -1;

            step = func_80192934();
            FMV_DBG("92CE8 step=%d BA=%u BC=%d\n", step, PE_LoadU8(0x800B0DBAu), (int16_t)PE_LoadU16(0x800B0DBCu));
            if (PE_Port_ShouldStop())
                return -1;

            if ((uint8_t)step == 0u) {
                ClearMovieArenas();
            } else if (PE_LoadU32(0x8009D26Cu) & 0x20000004u) {
                PE_StoreU8(0x800B0DBAu,
                           (uint8_t)(PE_LoadU8(0x800B0DBAu) - 1u));
                func_800870F0(0u);
                if (PE_Port_ShouldStop())
                    return -1;
                func_8010C0D8(0u);
                if (PE_Port_ShouldStop())
                    return -1;
                func_8007A2A4();
                if (PE_Port_ShouldStop())
                    return -1;
                (void)func_80080DC4(9, 0u, 0u);
                decoded = (int16_t)PE_LoadU16(0x800B0DBCu);
                ClearMovieArenas();
                if (decoded < 1400) {
                    (void)func_80073A44(0);
                    func_80074D28(0);
                }
                result = 1;
            }

            func_80070E54();
            if (PE_Port_ShouldStop())
                return -1;
            if (PE_LoadU8(0x800B0DBAu) == 0u)
                break;
        }
    }

    /* 80192F60: clear overlay busy bit 0x200 and return $s3. */
    PE_StoreU32(GA_OVERLAY_FLAGS,
                PE_LoadU32(GA_OVERLAY_FLAGS) & ~0x200u);
    return result;
}
