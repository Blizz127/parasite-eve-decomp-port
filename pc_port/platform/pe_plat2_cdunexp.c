/* libds unexpected-status handler (retail entry 0x8008068C, public name pe_plat_cd_unexpected_status).
 *
 * PROVENANCE (clean-room, black-box).  Written by the sw-cdrw-c author (Sonnet 5.5, 2026-11-27) from:
 *   - the retail-derived black-box vector sets cdgen60 (800 cases) and cdgen60b (status_dispatch.txt, 1200 cases, 177 reach the
 *     callback dispatcher), read through their columns only (A arguments, L ordered loads, S stores, RETS, CALL); the lane
 *     reports docs/evidence/native-cdx-20261126/cdgen60/REPORT.md and cdgen60b/REPORT.md; the checker
 *     pc_port/tools/pe_cdx_cdgen60_check.c on pe_cdx_common_canary.h (strict and mapped builds);
 *   - the suite test pc_port/tests/test_cd_status_native.h (CDSTATUS_mode_change_bit) as the contract of the status-2 arm;
 *   - the public headers pe_guest_ram.h, pe_sdk.h (PE_Cd_DispatchDataCallback prototype and comment), pe_plat/cd.h,
 *     pe_plat/cdx_stream_seams.h, and psx-spx (CD-ROM controller: INT2 acknowledge / INT5 error, command 0x0E Setmode).
 *   NOT read: the previous provider pe_plat_cd_unexpected_status.c (never opened), src/, asm/, vendor/, game/decomp_hand,
 *   game/boot, pe_lib*.c, pe_stream.c, *_retail.c, oracle sources, retail_*_cases.h.  Nothing is claimed about how the original
 *   was written; the behaviour below is what the vectors show.
 *
 * The routine is the "status the command layer did not expect" step of the libds command queue.  Only the low byte of the
 * first argument counts; the second argument (the response pointer) is not read.  Guest words (little endian):
 *   0x8009B56C  byte, drive status copy; bit 4 selects the INT5 arm   (status 5, cdgen60 172 + 48 cases)
 *   0x8009B574  lane word, 0x8009B578  state word                      (status 5 stores)
 *   0x800A36A4  data callback word, 0x8009B554  host-booted gate       (status 5 dispatch, cdgen60b)
 *   0x8009B558  byte, last command; 0x8009B559 its first parameter byte; 0x8009B581 the saved mode byte;
 *   0x8009B594  state word                                             (status 2, command 0x0E)
 * Behaviour, with the evidence:
 *   status 5, byte 0x8009B56C bit 4 clear:  lane 1, state 0xB.  Nothing else is read or stored (cdgen60: 172 cases).
 *   status 5, bit 4 set:  lane 2, state 0xC, then the callback word is loaded; when it is nonzero the gate word is loaded, and
 *       when that is nonzero too the callback is called with the status 5 (cdgen60b: one call per case, argument 5, target
 *       taken from the word as it stands after the two stores).  Result of the call is ignored (void entry).
 *   status 2:  only when the byte 0x8009B558 is 0x0E (Setmode) anything happens: the saved mode byte 0x8009B581 and the
 *       parameter byte 0x8009B559 are read; when their bit 7 differs, 0x8009B578 = 0xF, 0x8009B574 = 2, 0x8009B594 = 3;
 *       in every such case 0x8009B581 := the parameter byte (all eight bits; cdgen60b 148 cases, suite test: parameter 0x20
 *       over saved 0 stores 0x20 and nothing else).
 *   any other status: no access at all (cdgen60: ~540 cases over 0..0x20, 0x80, 0xFF, 0x100, 0x7FFFFFFF, ...).
 * Host seam: PE_Cd_DispatchDataCallback(target, status, response) is the address-keyed dispatcher; the original calls the
 * callback with ONE argument.  The seam takes the response as a second argument (known callbacks that use it read it), so the
 * checker's strict build differs from the vectors by the recorded argument count only; its mapped build (one argument)
 * is the criterion agreed with the vector lane. */
#include <stdint.h>

#include "pe_guest_ram.h"
#include "pe_plat/cd.h"
#include "pe_plat/cdx_stream_seams.h"
#include "pe_sdk.h"

#define B_DRIVE_STATUS 0x8009B56Cu
#define W_LANE         0x8009B574u
#define W_STATE        0x8009B578u
#define W_CALLBACK     0x800A36A4u
#define W_GATE         0x8009B554u
#define B_LAST_CMD     0x8009B558u
#define B_CMD_PARAM    0x8009B559u
#define B_MODE_SAVED   0x8009B581u
#define W_MODE_STATE   0x8009B594u

void pe_plat_cd_unexpected_status(uint32_t status, uint32_t response)
{
    status &= 0xFFu;
    if (status == 5u) {
        if (!(PE_LoadU8(B_DRIVE_STATUS) & 0x10u)) {
            PE_StoreU32(W_LANE, 1u);
            PE_StoreU32(W_STATE, 0xBu);
            return;
        }
        PE_StoreU32(W_LANE, 2u);
        PE_StoreU32(W_STATE, 0xCu);
        if (PE_LoadU32(W_CALLBACK) != 0u && PE_LoadU32(W_GATE) != 0u)
            (void)PE_Cd_DispatchDataCallback(PE_LoadU32(W_CALLBACK), 5u, response);
        return;
    }
    if (status == 2u && PE_LoadU8(B_LAST_CMD) == 0x0Eu) {
        uint32_t saved = PE_LoadU8(B_MODE_SAVED);
        uint32_t param = PE_LoadU8(B_CMD_PARAM);
        if ((saved ^ param) & 0x80u) {
            PE_StoreU32(W_STATE, 0xFu);
            PE_StoreU32(W_LANE, 2u);
            PE_StoreU32(W_MODE_STATE, 3u);
        }
        PE_StoreU8(B_MODE_SAVED, (uint8_t)param);
    }
}
