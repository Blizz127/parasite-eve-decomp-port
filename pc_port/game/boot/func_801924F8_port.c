/*
 * Phase 6E-B54K-AF — authenticated prefix of overlay func_801924F8.
 *
 * Complete retail function: [0x801924F8,0x80192934), 271 words, SHA-256
 * ef825dccdbfd2a74941203d37739c713ad1e3bd8de48ca747f55d0e75a92f00a.
 * Translated prefix: [0x801924F8,0x801927B0), 174 words, SHA-256
 * 6cdaa4c16368ca81f53aeb6d194eca170cb13116b7476ad2e1e42f24d5aa9918.
 * Supersedes strict frontier func_801924F8_80192790_cut.
 * Supersedes strict frontier func_801924F8_801927A0_cut.
 */
#include "psx_compat.h"
#include "game_port.h"
#include "pe_sdk.h"
#include "pe_plat/framebuffer_api.h"

#include <stdio.h>
#include <stdlib.h>

#define FMV_DBG(...) do { if (getenv("PE_FMV_DEBUG")) fprintf(stderr, "[FMV] " __VA_ARGS__); } while (0)

int func_801924F8(int index)
{
    uint16_t record_index = (uint16_t)index;
    pe_addr_t record;
    pe_addr_t suffix;
    pe_addr_t active_pair;
    uint8_t kind;
    uint16_t movie_x;
    uint16_t movie_y;
    uint32_t display_buffer;
    char filename[32] = "";
    int status;
    int32_t s1 = 0; /* E0 poll result; live into got_frame (retail $s1) */
    int s_frame_complete = 0; /* Also set on any nonzero slot result below. */

    if (record_index >= 47u)
        return 0;

    PE_StoreU8(0x800B0DBFu, (uint8_t)index);
    record = 0x801D0E00u + (uint32_t)record_index * 20u;
    PE_StoreU32(0x801D11ACu, record);
    kind = PE_LoadU8(record + 4u);
    PE_StoreU8(0x800B0DBBu, kind);

    func_801918F8(0, (int8_t)kind);
    if (PE_Port_ShouldStop()) return 0;
    func_801918F8(1, (int8_t)kind);
    if (PE_Port_ShouldStop()) return 0;

    (void)func_800719F4(filename,
                        record_index < 21u ? "\\FMV1" : "\\FMV2");
    suffix = PE_LoadU32(record);
    (void)func_800719F4(
        filename, (const char *)PE_TranslateConst(suffix, 16u));

    do {
        if (PE_Port_ShouldStop()) return 0;
        status = 0;
        int ready = func_8007F72C();
        if (PE_Port_ShouldStop()) return 0;
        int depth = ready == 1 ? func_8007F778() : 1;
        if (PE_Port_ShouldStop()) return 0;
        if (ready == 1 && depth == 0)
            status = func_80081414(0x801D0DC4u, filename);
        if (PE_Port_ShouldStop()) return 0;
    } while (status == 0 || status == -1);

    FMV_DBG("924F8 file found %s stop=%d\n", filename, PE_Port_ShouldStop());
    PE_StoreU32(0x801D0DDCu, PE_LoadU32(0x801D0DC4u));
    record = PE_LoadU32(0x801D11ACu);
    movie_x = PE_LoadU16(record + 10u);
    movie_y = PE_LoadU16(record + 12u);
    display_buffer = PE_LoadU32(0x8009CDDCu);

    PE_StoreU8(0x801D148Au, (uint8_t)display_buffer);
    PE_StoreU32(0x801D1464u, PE_LoadU32(0x801D0DE8u));
    PE_StoreU8(0x801D146Cu, 0u);
    PE_StoreU32(0x801D1470u, PE_LoadU32(0x801D0DF0u));
    PE_StoreU32(0x801D1474u, PE_LoadU32(0x801D0DF4u));
    PE_StoreU8(0x801D1478u, 0u);
    PE_StoreU16(0x801D147Au, movie_x);
    PE_StoreU16(0x801D1482u, movie_x);
    PE_StoreU32(0x801D1468u, PE_LoadU32(0x801D0DECu));
    PE_StoreU16(0x801D147Cu, (uint16_t)(movie_y + 240u));
    PE_StoreU16(0x801D1484u, movie_y);

    active_pair = 0x801D1464u + ((display_buffer & 0xFFu) << 3);
    PE_StoreU16(0x801D148Cu, PE_LoadU16(active_pair + 22u));
    PE_StoreU16(0x801D148Eu, PE_LoadU16(active_pair + 24u));
    PE_StoreU16(0x801D1490u,
                PE_LoadU8(0x800B0DBBu) != 0u ? 24u : 16u);
    PE_StoreU8(0x801D1494u, 0u);

    func_8010BE3C(0);
    if (PE_Port_ShouldStop()) {
        FMV_DBG("924F8 stop after BE3C\n");
        return 0;
    }

    func_8010C0D8(0x80191DC8u);
    if (PE_Port_ShouldStop()) return 0;

    func_8007A214(PE_LoadU32(0x801D0DFCu), 0x40u);
    if (PE_Port_ShouldStop()) return 0;

    record = PE_LoadU32(0x801D11ACu);
    func_8007C304(1u, (int32_t)(int16_t)PE_LoadU16(record + 6u),
                  -1, 0u, 0u);
    if (PE_Port_ShouldStop()) return 0;

cdready_wait:
    FMV_DBG("924F8 cdready_wait stop=%d\n", PE_Port_ShouldStop());
    for (;;) {
        if (PE_Port_ShouldStop()) return 0;
        while (func_8007F72C() != 1) {
            if (PE_Port_ShouldStop()) return 0;
        }
        if (PE_Port_ShouldStop()) return 0;
        int depth = func_8007F778();
        if (PE_Port_ShouldStop()) return 0;
        if (depth == 0) break;
    }

    /* Retail passes an eight-byte stack result to the blocking command, but
     * the complete 271-word CFG never reads it.  Passing null preserves the
     * live CdlSetloc effect without inventing unobserved response bytes. */
    if (func_80080D5C(2, 0x801D0DC4u, 0u) == 0)
        return 0;
    if (PE_Port_ShouldStop()) return 0;

    status = func_80081314(0x801D0DC4u, 0x1E0u);
    if (PE_Port_ShouldStop())
        return 0;
    if (status == 0)
        goto cdready_wait; /* B0-exhaustion: re-poll. */
    /* B8: v0 = [146C]; a1 = film id; mode = 3. */
    FMV_DBG("924F8 ReadS ok\n");
    func_800870F0(PE_LoadU8(0x800B0DBEu));
    if (PE_Port_ShouldStop())
        return 0;
    /* D0: RLE frame decode (2000 records at D0DF8).  Relanded in
     * CDQ2d with the E0 delivery pump (see below). */
    func_8010BD4C(PE_LoadU32(0x801D0DF8u), 2000u);
    if (PE_Port_ShouldStop())
        return 0;
e0_poll:
    FMV_DBG("924F8 e0_poll\n");
    /* E0 polls the streaming slot table, with 2000 tries. The empty-VLC
     * fixture pumps 7C214 synchronously; real STR advances the CD/IRQ model.
     * IRQ delivery can clear B89F4 before this loop samples it, so this path
     * assumes a nonzero 91B64 slot represents a completed frame. That
     * invariant has not been established for every current producer or IRQ
     * ordering; see native-movie-boundary-reconciliation-20261104. Do not
     * call 7C214 on every poll: that can promote mid-frame records. */
    {
        uint32_t s0 = 2000u;
        int empty_vlc = (PE_LoadU8(0x8010CBFCu) == 0xFFu &&
                         PE_LoadU8(0x8010CBFDu) == 0xFFu);
        for (;;) {
            uint32_t left;
            if (empty_vlc) {
                s_frame_complete = 1;
                func_8007C214();
                if (PE_Port_ShouldStop())
                    return 0;
            } else {
                HostFB_StreamTick();
                if (PE_Port_ShouldStop())
                    return 0;
                if (PE_LoadU32(0x800B89F4u) != 0u) {
                    s_frame_complete = 1;
                    func_8007C214();
                    if (PE_Port_ShouldStop())
                        return 0;
                }
            }
            s1 = func_80191B64(0x801D1464u);
            if (PE_Port_ShouldStop())
                return 0;
            left = s0 - 1u;
            s0 = left;
            if (s1 != 0) {
                /* Assumes slot readiness implies last-chunk promotion. */
                s_frame_complete = 1;
                goto got_frame;
            }
            if (((left << 16) & 0xFFFFFFFFu) != 0u)
                continue;
            break;
        }
    }
    /* Give-up path (0x80192878): copy D0DC4 to D0DDC, then CD-ready waits
     * against $s2 == 1 (set at 0x801927CC), re-issue Setloc(D0DDC) +
     * ReadS(480).  beqz → back to the 0x8019289C wait; nonzero → E0 with
     * s0 = 2000. */
    FMV_DBG("924F8 give-up path\n");
    PE_StoreU32(0x801D0DDCu, PE_LoadU32(0x801D0DC4u));
    for (;;) {
        if (PE_Port_ShouldStop()) return 0;
        while (func_8007F72C() != 1) {
            if (PE_Port_ShouldStop())
                return 0;
        }
        if (PE_Port_ShouldStop()) return 0;
        int depth = func_8007F778();
        if (PE_Port_ShouldStop()) return 0;
        if (depth != 0)
            continue;
        func_80080D5C(2, 0x801D0DDCu, 0u);
        if (PE_Port_ShouldStop()) return 0;
        status = func_80081314(0x801D0DDCu, 480u);
        if (PE_Port_ShouldStop())
            return 0;
        if (status != 0)
            goto e0_poll;
    }
got_frame:
    FMV_DBG("924F8 got_frame s1=%d\n", s1);
    /* 80192814..80192930: authenticated got_frame tail (ov133 carve).
     * Order is load-bearing: a2 from [D0DF8], bump B0DBC into $v1, XOR
     * toggle [146C] then index s3+bank*4 for a1, store the bumped
     * B0DBC, jal C89C(a0=s1) / jal 7C394(a0=s1).  Retail then forces
     * v0=0 and always takes the EC success arm (bne vs -1): clear
     * [D0DBD], set B0DBC=1, increment B0DBA, epilogue return.
     * Disassembly evidence: pe_mv1d_c89c_oracle.py callsite anchors +
     * live PE.IMG carve at 80192814.
     *
     * The retained complete-frame check below does not establish that gate:
     * the sole jump here sets s_frame_complete for every nonzero slot. Slot
     * completeness remains an open producer/IRQ proof obligation. */
    {
        uint32_t a2 = PE_LoadU32(0x801D0DF8u);
        uint16_t dbc = (uint16_t)(PE_LoadU16(0x800B0DBCu) + 1u);
        uint8_t bank = (uint8_t)(PE_LoadU8(0x801D146Cu) ^ 1u);
        pe_addr_t a1;
        int empty_vlc = (PE_LoadU8(0x8010CBFCu) == 0xFFu &&
                         PE_LoadU8(0x8010CBFDu) == 0xFFu);

        PE_StoreU8(0x801D146Cu, bank);
        PE_StoreU16(0x800B0DBCu, dbc);
        a1 = (pe_addr_t)PE_LoadU32(0x801D1464u + ((uint32_t)bank << 2));
        if (!empty_vlc && !s_frame_complete) {
            Bootstrap_ReturnVoid("func_8010C89C_needs_complete_frame",
                                 "func_801924F8");
            PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
            return 0;
        }
        (void)func_8010C89C((uint32_t)s1, a1, a2, 0u);
        if (PE_Port_ShouldStop())
            return 0;
        func_8007C394((uint32_t)s1);
        /* EC stores (801928EC): retail's dead v0=0/-1 compare always
         * lands here after 7C394. */
        PE_StoreU8(0x801D0DBDu, 0u);
        PE_StoreU16(0x800B0DBCu, 1u);
        PE_StoreU8(0x800B0DBAu,
                   (uint8_t)(PE_LoadU8(0x800B0DBAu) + 1u));
        return 0;
    }
}
