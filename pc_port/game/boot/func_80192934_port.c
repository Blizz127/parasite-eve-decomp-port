/*
 * Overlay continue-frame worker func_80192934 (PE.IMG ov133,
 * [0x80192934, 0x80192C48), 197 words). Twin of func_80122040.
 *
 * Disassembly: PE.IMG LBA 1013 + 0x03D2*0x800, load VA 0x8018EFF0.
 * Host: the 91B64 poll has no StreamTick in retail; the modeled drive
 * needs HostFB_StreamTick or the second frame never assembles.  The
 * 1494 wait seeds remaining=0 (wraps 2^32); pump VSync and take the
 * retail timeout arm after 0x800000 host pumps (same bound as 122040).
 */
#include "psx_compat.h"
#include "game_port.h"
#include "pe_sdk.h"
#include "pe_plat/cd_regs.h"
#include "pe_plat/framebuffer_api.h"

#include <string.h>

int func_80192934(void)
{
    int32_t next;
    uint32_t remaining;
    uint32_t pumps;
    int s_frame_complete = 0;

    if (PE_LoadU8(0x800B0DBAu) < 2u)
        return 0;

    if (PE_LoadU8(0x801D0DC0u) == 2u) {
        func_801918F8((int8_t)(PE_LoadU32(0x8009CDDCu) ^ 1u),
                      (int8_t)PE_LoadU8(0x800B0DBBu));
        if (PE_Port_ShouldStop())
            return 0;
        PE_StoreU8(0x801D0DC0u, 0u);
    }

    memcpy(PE_Translate(0x801D0DDCu, 4u),
           PE_TranslateConst(0x801D0DC4u, 4u), 4u);

    func_8010BFA0(PE_LoadU32(0x801D1464u + PE_LoadU8(0x801D146Cu) * 4u),
                  PE_LoadU8(0x801D0DBEu));
    if (PE_Port_ShouldStop())
        return 0;

    {
        int32_t words =
            ((int32_t)(int16_t)PE_LoadU16(0x801D1490u) *
             (int32_t)(int16_t)PE_LoadU16(0x801D1492u)) / 2;
        func_8010C01C(PE_LoadU32(0x801D1470u +
                                PE_LoadU8(0x801D1478u) * 4u),
                      (uint32_t)words);
        if (PE_Port_ShouldStop())
            return 0;
    }

    next = 0;
    for (;;) {
        uint32_t tries = 2000u;
        for (;;) {
            HostFB_StreamTick();
            if (PE_Port_ShouldStop())
                return 0;
            if (PE_LoadU32(0x800B89F4u) != 0u) {
                s_frame_complete = 1;
                func_8007C214();
                if (PE_Port_ShouldStop())
                    return 0;
            }
            next = func_80191B64(0x801D1464u);
            if (PE_Port_ShouldStop())
                return 0;
            tries--;
            if (next != 0) {
                s_frame_complete = 1;
                break;
            }
            if (((tries << 16) & 0xFFFFFFFFu) == 0u)
                break;
        }
        if (next != 0)
            break;

        (void)func_8007C2A0(0x801D0DDCu);
        for (unsigned attempts = 0;; attempts++) {
            if (PE_Port_ShouldStop())
                return 0;
            int ready = func_8007F72C();
            if (PE_Port_ShouldStop())
                return 0;
            int depth = ready == 1 ? func_8007F778() : 1;
            if (PE_Port_ShouldStop())
                return 0;
            if (ready == 1 && depth == 0) {
                (void)func_80080D5C(2, 0x801D0DDCu, 0x801FFE70u);
                if (PE_Port_ShouldStop())
                    return 0;
                int started = func_80081314(0x801D0DDCu, 0x1E0u);
                if (PE_Port_ShouldStop())
                    return 0;
                if (started)
                    break;
            }
            HostFB_StreamTick();
            if (PE_Port_ShouldStop())
                return 0;
            if (!PE_CdReg_DeviceEnabled() || attempts == 0x100000u) {
                Bootstrap_ReturnVoid("movie_retry_wait", "func_80192934");
                PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
                return 0;
            }
        }
    }

    {
        uint32_t table = PE_LoadU32(0x801D0DF8u);
        uint16_t count = PE_LoadU16(0x800B0DBCu);
        uint32_t bank = PE_LoadU8(0x801D146Cu) ^ 1u;
        int empty_vlc = (PE_LoadU8(0x8010CBFCu) == 0xFFu &&
                         PE_LoadU8(0x8010CBFDu) == 0xFFu);

        PE_StoreU8(0x801D146Cu, (uint8_t)bank);
        PE_StoreU16(0x800B0DBCu, (uint16_t)(count + 1u));
        /* A nonzero next sets s_frame_complete before reaching this check;
         * the check alone does not establish that the slot is complete. */
        if (!empty_vlc && !s_frame_complete) {
            Bootstrap_ReturnVoid("func_8010C89C_needs_complete_frame",
                                 "func_80192934");
            PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
            return 0;
        }
        (void)func_8010C89C((uint32_t)next,
                            PE_LoadU32(0x801D1464u + bank * 4u),
                            table, 0u);
        if (PE_Port_ShouldStop())
            return 0;
        func_8007C394((uint32_t)next);
    }

    /* Retail stores v0=0 as the wait counter (wraps 2^32). */
    remaining = 0u;
    pumps = 0u;
    if (!PE_LoadU8(0x801D1494u)) {
        for (;;) {
            remaining--;
            if (remaining == 0u || ++pumps >= 0x800000u) {
                uint32_t bank = PE_LoadU8(0x801D148Au) ^ 1u;
                PE_StoreU8(0x801D1494u, 1u);
                PE_StoreU8(0x801D148Au, (uint8_t)bank);
                PE_StoreU16(0x801D148Cu,
                            PE_LoadU16(0x801D1464u + bank * 8u + 0x16u));
                PE_StoreU16(0x801D148Eu,
                            PE_LoadU16(0x801D1464u + bank * 8u + 0x18u));
            }
            if (PE_LoadU8(0x801D1494u))
                break;
            HostFB_VSync(-1);
            if (PE_Port_ShouldStop())
                return 0;
        }
    }

    {
        uint8_t end = PE_LoadU8(0x801D0DBDu);
        PE_StoreU8(0x801D1494u, 0u);
        if (end != 1u)
            return 1;
        PE_StoreU8(0x800B0DBAu,
                   (uint8_t)(PE_LoadU8(0x800B0DBAu) - 1u));
        func_8010C0D8(0u);
        if (PE_Port_ShouldStop())
            return 0;
        func_8007A2A4();
        if (PE_Port_ShouldStop())
            return 0;
        (void)func_80080DC4(9, 0u, 0u);
        return 0;
    }
}
