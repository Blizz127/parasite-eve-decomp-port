/* libds command issue wrapper (retail entry 0x8007FCFC, public name pe_plat_cd_issue_wrapper).
 *
 * PROVENANCE (clean-room, black-box).  Written by the sw-cdrw-c author (Sonnet 5.5, 2026-11-27) from:
 *   - the retail-derived black-box vector sets cdgen58 (issue.txt, 800 cases) and cdgen58b (the byte-sweep variant), read
 *     through their columns only (A arguments, L ordered loads, S stores, RETS, CALL); the lane reports
 *     docs/evidence/native-cdx-20261126/cdgen58/REPORT.md and cdgen58b/REPORT.md; the checker
 *     pc_port/tools/pe_cdx_cdgen58_check.c on pe_cdx_common_canary.h;
 *   - the suite test pc_port/tests/test_cd_issue_native.h (stopped flush: no state write);
 *   - public headers pe_guest_ram.h, game_port.h, pe_plat/cd.h, pe_plat/cdhost.h, and psx-spx (CD-ROM command list: 0x01
 *     Nop, 0x07 MotorOn, 0x08 Stop).
 *   NOT read: the previous provider pe_plat_cd_issue_wrapper.c (never opened), src/, asm/, vendor/, game/decomp_hand,
 *   game/boot, pe_lib*.c, pe_stream.c, *_retail.c, oracle sources, retail_*_cases.h.  Nothing is claimed about how the
 *   original was written.
 *
 * Arguments (command, data): command byte = command & 0xFF; data = guest pointer to the command's parameter bytes or 0.
 * Behaviour (cdgen58; the flush result is not used in any of the 800 cases):
 *   1. the CD flush (0x8007B9EC); a requested stop afterwards returns 0 with no state write (CDWRAP_stopped_flush);
 *   2. last command byte 0x8009B558 := command byte;
 *   3. data != 0: the four-byte copy (0x80080950)(0x8009B559, data) and parameter pointer word 0x8009B560 := 0x8009B559;
 *      data == 0: no copy, 0x8009B560 := 0;
 *   4. the command's wait count: word 0x8009B598 := 0x3C0 when the command's table word 0x8009B5A4 + 4*command is nonzero,
 *      0x1E when it is zero (239 of 800 cases); word 0x8009B59C := 0;
 *   5. commands 7 and 8 read the status byte 0x8009B58B (cdgen58b: all 256 values of it for each of the two commands) and
 *      are replaced by command 1 (Nop) without parameter: command 7 only when the byte is 1, command 8 whenever the byte is
 *      not 1; 0x8009B558 := 1, 0x8009B560 := 0 (the two stores are repeated); every other command, 6 and 9 included, never;
 *   6. the issue (0x8007B558)(command, parameter pointer, 0, 1) -- four arguments, always the last call;
 *   7. issue result 0: 0x8009B580 := command byte (the replaced one for the Nop case), result 1;
 *      any other issue result: 0x8009B59C := 0, 0x8009B598 := 0, result 0 (0x8009B580 untouched). */
#include <stdint.h>

#include "pe_guest_ram.h"
#include "game_port.h"
#include "pe_plat/cd.h"
#include "pe_plat/cdhost.h"

#define B_LAST_CMD     0x8009B558u
#define W_PARAM_PTR    0x8009B560u
#define B_PARAM_BUF    0x8009B559u
#define W_WAIT_COUNT   0x8009B598u
#define W_WAIT_STATE   0x8009B59Cu
#define B_ISSUED_CMD   0x8009B580u
#define B_STATUS_BYTE  0x8009B58Bu
#define T_CMD_TABLE    0x8009B5A4u
#define WAIT_SHORT     0x1Eu
#define WAIT_LONG      0x3C0u

/* Whether command 7 / 8 is replaced by the Nop, from the status byte. */
static int replaced_by_nop(uint32_t cmd)
{
    uint32_t st;
    if (cmd != 7u && cmd != 8u) return 0;
    st = PE_LoadU8(B_STATUS_BYTE);
    return cmd == 8u ? st != 1u : st == 1u;
}

int pe_plat_cd_issue_wrapper(uint32_t command, uint32_t data)
{
    uint32_t cmd = command & 0xFFu, param;
    int r;

    pe_plat_cd_flush();
    if (PE_Port_ShouldStop()) return 0;
    PE_StoreU8(B_LAST_CMD, (uint8_t)cmd);
    if (data != 0u) {
        pe_plat_cd_copy4(B_PARAM_BUF, data);
        param = B_PARAM_BUF;
    } else {
        param = 0u;
    }
    PE_StoreU32(W_PARAM_PTR, param);
    PE_StoreU32(W_WAIT_COUNT, PE_LoadU32(T_CMD_TABLE + 4u * cmd) != 0u ? WAIT_LONG : WAIT_SHORT);
    PE_StoreU32(W_WAIT_STATE, 0u);
    if (replaced_by_nop(cmd)) {
        cmd = 1u;
        param = 0u;
        PE_StoreU8(B_LAST_CMD, (uint8_t)cmd);
        PE_StoreU32(W_PARAM_PTR, param);
    }
    r = pe_plat_cdhost_cmd(cmd, param, 0u, 1u);
    if (r == 0) {
        PE_StoreU8(B_ISSUED_CMD, (uint8_t)cmd);
        return 1;
    }
    PE_StoreU32(W_WAIT_STATE, 0u);
    PE_StoreU32(W_WAIT_COUNT, 0u);
    return 0;
}
