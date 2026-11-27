/* CD stream ring / issue / sync / ack leaves (guest 0x8007FA2C, 0x8007A4D0,
 * 0x8007A60C, 0x8007A740, 0x8007A3CC, 0x8007A8EC, 0x80080404, 0x8008080C,
 * 0x8007CE80, 0x8007FE24, 0x8007A2A4, 0x8007BBFC), written clean-room from the
 * black-box vector sets cdgen42 and cdgen42b (docs/evidence/native-cdx-20261126,
 * including the runtime-observed pump / progress / decode tuples), the recorded
 * seam contracts and psx-spx; no SDK source, decompiled source, tables or
 * disassembly were read.  Revision 2 (sw-cdx-stream2): pump 0x8007FE24, progress
 * handler 0x80080404 and decoder 0x8008080C rewritten from cdgen42b; the other
 * nine entries are the revision-1 code.  The guest callback slots
 * (0x800A36A0..36AC) are dispatched through PE_GuestHook_Call, never through a
 * bound host symbol.
 *
 * Every behaviour below is exactly what the vector sets show; nothing is
 * claimed about the original's source.  All words are little-endian guest RAM.
 * Branches the vectors never exercise are marked "unsampled" and keep the
 * no-op / zero-seed behaviour (see AUTHOR-REPORT.md for the open list).
 *
 * Issue results: pe_plat_cd_command_issue_host() returns 0 on success
 * (nonzero = refused / failed / port stop).  Retrying or stopping on a
 * nonzero result is reproduced; "continue anyway" is never done.
 *
 * Host contracts (port facts, not behaviour of the original; the checkers
 * never reach them):
 *  - every retry loop is bounded by the original's own limit (4 attempts) and
 *    also ends when PE_Port_ShouldStop() is set;
 *  - the interrupt-flag acknowledge loop of the device init (the original
 *    spins until the CD interrupt-flag register reads zero in bits 0..2)
 *    checks PE_Port_ShouldStop() on every pass, gives the port clock one
 *    pe_plat_sio_poll_tick() per pass, and has a stuck-wait budget of 0x40000
 *    passes that ends in PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY)
 *    (the port clock only moves on ticks and the device keeps raising events
 *    until it does);
 *  - the acknowledge loop only runs while the modelled CD device is enabled
 *    (PE_CdReg_DeviceEnabled): without the model the register window is plain
 *    memory and the written acknowledge value would read back forever;
 *  - stream shutdown also closes the host stream feeder (stream_feed.h:
 *    "called on stream teardown"); the original has no counterpart. */
#include <stdint.h>

#include "pe_guest_ram.h"
#include "pe_cdreg.h"
#include "pe_plat/cd.h"
#include "pe_plat/bios_sys.h"
#include "pe_plat/bios_file.h"
#include "pe_plat/irq_callback.h"
#include "pe_plat/stream_feed.h"
#include "pe_plat/cdx_stream.h"
#include "pe_guestcode.h"
#include "game_port.h"

#define POLL_BUDGET      0x40000u

/* CD command guard / debug-level words and the per-command argument table */
#define W_ISSUE_GUARD    0x8009AFB4u   /* zeroed around each issue attempt */
#define B_DRIVE_STATUS   0x8009AFC4u   /* last drive status byte; bit 4 = shell open */
#define W_CMD_TABLE      0x8009AF2Cu   /* word per command id (nonzero = needs Setloc first) */
#define W_STOP_LANE      0x8009AFD8u
#define W_CDREG_INDEX    0x8009AF1Cu   /* shutdown: pointer to the index register */
#define W_CDREG_REG3     0x8009AF28u   /* shutdown: pointer to register 3 */
#define W_REG_INDEX      0x8009B27Cu   /* pointer words to the controller registers */
#define W_REG_2          0x8009B284u
#define W_REG_3          0x8009B288u
#define W_REG_DELAY      0x8009B28Cu   /* pointer to the COM_DELAY word */

#define RAM_WORD(a)      PE_LoadU32(a)

/* ---- 0x8007FA2C: CD startup state ----------------------------------- */
void pe_plat_cdxstream_initialize_state(void)
{
    unsigned i;
    PE_StoreU32(0x8009B554u, 0);
    PE_StoreU8(0x8009B558u, 0);
    PE_StoreU8(0x8009B55Cu, 0);
    PE_StoreU8(0x8009B55Bu, 0);
    PE_StoreU8(0x8009B55Au, 0);
    PE_StoreU8(0x8009B559u, 0);
    PE_StoreU32(0x8009B560u, 0);
    for (i = 0; i < 8u; i++) PE_StoreU8(0x8009B56Bu - i, 0);   /* 0x56B down to 0x564 */
    PE_StoreU32(0x8009B574u, 2u);
    PE_StoreU32(0x8009B578u, 0xEu);
    PE_StoreU8(0x8009B56Cu, 0);
    PE_StoreU32(0x8009B570u, 0);
    PE_StoreU32(0x8009B57Cu, 0x15u);
    PE_StoreU8(0x8009B580u, 0);
    PE_StoreU8(0x8009B581u, 0);
    (void)pe_plat_cd_int_to_pos(0, 0x8009B582u);
    for (i = 0; i < 6u; i++) PE_StoreU8(0x8009B586u + i, 0);
    PE_StoreU32(0x8009B58Cu, 0);
    PE_StoreU32(0x8009B590u, 1u);
    PE_StoreU32(0x8009B594u, 0);
    PE_StoreU32(0x8009B59Cu, 0);
    PE_StoreU32(0x8009B598u, 0);
    PE_StoreU32(0x8009B5A0u, 0);
}

/* ---- 0x8007A4D0 / 0x8007A60C / 0x8007A740: command control ---------- */
/* Up to 4 attempts.  Each attempt: zero the guard word; when the command is
 * not 1 and the drive status byte has bit 4 set, issue command 1 (result
 * ignored); when the command's table word is nonzero, issue command 2 (Setloc)
 * first and start the next attempt if it fails; restore the guard word; issue
 * the command itself; a zero result ends with success.  After the last failed
 * attempt the guard word is restored once more.  The Setloc / command issues
 * pass (command, param, dst, mode) with dst = the caller's result pointer
 * (0 for the asynchronous entry) and mode 1 only for the asynchronous
 * command itself. */
static int cdxs_control(unsigned com, uint32_t param, uint32_t dst, uint32_t mode)
{
    uint32_t saved = RAM_WORD(W_ISSUE_GUARD);
    unsigned attempt;
    for (attempt = 0; attempt < 4u; attempt++) {
        PE_StoreU32(W_ISSUE_GUARD, 0);
        if (com != 1u && (PE_LoadU8(B_DRIVE_STATUS) & 0x10u))
            (void)pe_plat_cd_command_issue_host(1u, 0, 0, 0);
        if (RAM_WORD(W_CMD_TABLE + 4u * com) != 0) {
            if (pe_plat_cd_command_issue_host(2u, param, dst, 0) != 0) {
                if (PE_Port_ShouldStop()) break;
                continue;
            }
        }
        PE_StoreU32(W_ISSUE_GUARD, saved);
        if (pe_plat_cd_command_issue_host(com, param, dst, mode) == 0) return 1;
        if (PE_Port_ShouldStop()) break;
    }
    PE_StoreU32(W_ISSUE_GUARD, saved);
    return 0;
}

int pe_plat_cdxstream_control(unsigned char command, uint32_t data, uint32_t result)
{
    return cdxs_control(command, data, result, 0);
}

int pe_plat_cdxstream_control_async(unsigned char command, uint32_t data)
{
    return cdxs_control(command, data, 0, 1u);
}

/* Control with the caller's result pointer; after a success the status poll
 * runs with (0, result).  The return value is always 0 in the vectors. */
int pe_plat_cdxstream_control_wait(unsigned char command, uint32_t data, uint32_t result)
{
    if (cdxs_control(command, data, result, 0))
        (void)pe_plat_cd_status_poll_host(0, result);
    return 0;
}

/* ---- 0x8007A3CC / 0x8007A8EC: thin wrappers -------------------------- */
void pe_plat_cdxstream_reset_latches(void)
{
    pe_plat_cd_flush();
}

void pe_plat_cdxstream_set_dma3_callback(unsigned int callback)
{
    (void)pe_plat_dma_callback(3u, callback);
}

/* ---- 0x8007CE80: stream word copy ----------------------------------- */
/* Forward word-at-a-time copy (an overlapping destination above the source
 * re-reads words it has already overwritten); no copy for a count <= 0. */
void pe_plat_cdxstream_stream_copy_words(uint32_t dst, uint32_t src, uint32_t words)
{
    int32_t n = (int32_t)words, i;
    for (i = 0; i < n; i++)
        PE_StoreU32(dst + 4u * (uint32_t)i, PE_LoadU32(src + 4u * (uint32_t)i));
}

/* ---- 0x8007A2A4: stream shutdown ------------------------------------ */
void pe_plat_cdxstream_shutdown_stream(void)
{
    (void)pe_plat_bios_enter_critical();
    if (RAM_WORD(W_STOP_LANE) == 1u) {
        (void)pe_plat_ds_data_callback(0);
        (void)pe_plat_cd_ds_set_ready_callback(0);
    } else {
        (void)pe_plat_dma_callback(3u, 0);
        (void)pe_plat_cd_set_ready_callback(0);
    }
    PE_CdStoreU8(RAM_WORD(W_CDREG_INDEX), 0);
    PE_CdStoreU8(RAM_WORD(W_CDREG_REG3), 0);
    PE_StrFeed_Close();
    pe_plat_bios_exit_critical();
}

/* ---- shared words of the status machine ------------------------------ */
#define W_GATE           0x8009B554u   /* callback gate: slots run only while nonzero */
#define B_STATE_LAST     0x8009B558u   /* decoder: index into the response-length table */
#define W_ISSUE_CMD      0x8009B570u   /* poll command word (0x20 / 0x21) */
#define W_PHASE          0x8009B574u   /* 1, 2 (state machine active), 3 */
#define W_STATE          0x8009B578u   /* machine state 0xB..0x14 */
#define W_SUBSTATE       0x8009B57Cu   /* sub-step of state 0xE (0x14..0x19) */
#define B_FLAG_588       0x8009B588u   /* decoded status bits 7, 6, 5, 1 of the last byte */
#define B_FLAG_58A       0x8009B58Au
#define B_FLAG_58B       0x8009B58Bu
#define W_SPIN_COUNT     0x8009B58Cu   /* ticks spent in sub-step 0x16 */
#define W_ODD_COUNT      0x8009B590u   /* incremented each time state 0xD is left */
#define W_TICK_A         0x8009B594u   /* countdowns decremented by the pump while > 0 */
#define W_TICK_B         0x8009B598u
#define W_TICK_C         0x8009B5A0u
#define B_DRIVE_STAT     0x8009B56Cu   /* last drive status byte: bit 1 motor, bit 4 shell open */
#define W_ALT_FLAG       0x8009B6A4u   /* toggled by state 0xC */
#define PE_CDXS_SCRATCH_PARAM 0x801FFE90u   /* 0xE command parameter byte (host scratch) */
#define W_SLOT_PUMP      0x800A36A0u   /* callback slots (guest code pointers) */
#define W_SLOT_PROG_A    0x800A36A4u
#define W_SLOT_PROG_B    0x800A36A8u
#define W_SLOT_PROG_C    0x800A36ACu

/* Dispatch one guest callback slot (the retail `jalr` through a hook word).
 * Returns 0 when the call did not happen or a stop was requested. */
static int cdxs_call_slot(const char *slot, const char *caller, uint32_t fn, unsigned arity, uint32_t a0, uint32_t a1)
{
    int ret = 0;
    if (!PE_GuestHook_Call(slot, caller, fn, arity, a0, a1, 0, 0, &ret)) return 0;
    return !PE_Port_ShouldStop();
}

/* ---- 0x8007FE24: status pump (called once per tick) ----------------- */
/* All words are little-endian guest RAM; derived from the cdgen42/42b vectors.
 *  1. Countdown C0 = word 0x8009B598: when it is exactly 1 it is cleared, the
 *     retry command runs and the call ends.  Otherwise each countdown
 *     (0x8009B598, 0x8009B594, 0x8009B5A0) that is > 0 (signed) is decremented.
 *  2. Phase 2 (word 0x8009B574): the state word 0x8009B578 selects a step:
 *       0xC  toggle word 0x8009B6A4 (0 -> 1, nonzero -> 0) after the optional
 *            issue below: when C0 <= 0 the command word 0x8009B570 = 0x20 and
 *            the issue wrapper runs with (1, 0) when 0x8009B6A4 was 0 and with
 *            (0xE, scratch pointer 0x801FFE90) when it was nonzero;
 *       0xD  0x8009B6A4 = 0, then (C0 <= 0) 0x20 and issue (1, 0);
 *       0xE  by sub-step 0x8009B57C: 0x15 / 0x17 / 0x18 (C0 <= 0): 0x20 and issue
 *            (1, 0) -- (0x13, 0) for 0x17; 0x16: 0x8009B58C += 1, then the same
 *            (1, 0) issue; 0x14, 0x19 and other values do nothing;
 *       0xF  when countdown 0x8009B594 (re-read) is 0: phase = 1, state = 0xB;
 *       0x10 / 0x11  when byte 0x8009B588 / 0x8009B58A is nonzero: phase 1,
 *            state 0xB; otherwise (C0 <= 0) 0x20 and issue (1, 0);
 *       other states do nothing.
 *  3. When the callback slot 0x800A36A0 holds a nonzero code pointer and the
 *     gate word 0x8009B554 is nonzero, the slot (re-read) is called with no
 *     arguments.
 *  4. Phase (re-read): 1 -- when byte 0x8009B58B is 0, or 3 -- when C0 (re-read)
 *     is <= 0 the command word is set to 0x21 and issue (1, 0) runs.  Phase 1
 *     with 0x8009B58B nonzero, phase 0 and any other phase do nothing. */
static void cdxs_issue_poll(uint32_t cmd, uint32_t data)
{
    PE_StoreU32(W_ISSUE_CMD, 0x20u);
    (void)pe_plat_cd_issue_wrapper(cmd, data);
}

void pe_plat_cdxstream_status_pump(void)
{
    uint32_t t = RAM_WORD(W_TICK_B);
    int32_t v;
    if (t == 1u) {
        PE_StoreU32(W_TICK_B, 0);
        pe_plat_cd_retry_command();
        return;
    }
    if ((int32_t)t > 0) PE_StoreU32(W_TICK_B, t - 1u);
    v = (int32_t)RAM_WORD(W_TICK_A);
    if (v > 0) PE_StoreU32(W_TICK_A, (uint32_t)(v - 1));
    v = (int32_t)RAM_WORD(W_TICK_C);
    if (v > 0) PE_StoreU32(W_TICK_C, (uint32_t)(v - 1));

    if (RAM_WORD(W_PHASE) == 2u) {
        switch (RAM_WORD(W_STATE)) {
        case 0xCu: {
            uint32_t alt = RAM_WORD(W_ALT_FLAG);
            /* With the alt flag set the pump zeroes the single mode byte of the 0xE command in the host scratch slot
             * 0x801FFE90 (whether or not the issue is blocked) and passes that address as the command's parameter
             * pointer (the project fixtures enter the original at the standard sp 0x801FFF00; this is its 0x70-byte
             * frame's slot).  The cdgen42b vectors show the same slot as an own-frame pointer (F+n). */
            if (alt != 0) PE_StoreU8(PE_CDXS_SCRATCH_PARAM, 0);
            if ((int32_t)RAM_WORD(W_TICK_B) <= 0) {
                if (alt == 0) cdxs_issue_poll(1u, 0);
                else cdxs_issue_poll(0xEu, PE_CDXS_SCRATCH_PARAM);
            }
            PE_StoreU32(W_ALT_FLAG, alt == 0 ? 1u : 0u);
            break;
        }
        case 0xDu:
            PE_StoreU32(W_ALT_FLAG, 0);
            if ((int32_t)RAM_WORD(W_TICK_B) <= 0) cdxs_issue_poll(1u, 0);
            break;
        case 0xEu:
            switch (RAM_WORD(W_SUBSTATE)) {
            case 0x15u: case 0x18u:
                if ((int32_t)RAM_WORD(W_TICK_B) <= 0) cdxs_issue_poll(1u, 0);
                break;
            case 0x16u:
                PE_StoreU32(W_SPIN_COUNT, RAM_WORD(W_SPIN_COUNT) + 1u);
                if ((int32_t)RAM_WORD(W_TICK_B) <= 0) cdxs_issue_poll(1u, 0);
                break;
            case 0x17u:
                if ((int32_t)RAM_WORD(W_TICK_B) <= 0) cdxs_issue_poll(0x13u, 0);
                break;
            default:
                break;
            }
            break;
        case 0xFu:
            if (RAM_WORD(W_TICK_A) == 0) {
                PE_StoreU32(W_PHASE, 1u);
                PE_StoreU32(W_STATE, 0xBu);
            }
            break;
        case 0x10u: case 0x11u:
            if (PE_LoadU8(RAM_WORD(W_STATE) == 0x10u ? B_FLAG_588 : B_FLAG_58A) != 0) {
                PE_StoreU32(W_PHASE, 1u);
                PE_StoreU32(W_STATE, 0xBu);
            } else if ((int32_t)RAM_WORD(W_TICK_B) <= 0) {
                cdxs_issue_poll(1u, 0);
            }
            break;
        default:
            break;
        }
    }

    if (RAM_WORD(W_SLOT_PUMP) != 0 && RAM_WORD(W_GATE) != 0) {
        if (!cdxs_call_slot("CD_vblank_callback", "func_8007FE24", RAM_WORD(W_SLOT_PUMP), 0, 0, 0)) return;
    }

    {
        uint32_t phase = RAM_WORD(W_PHASE);
        if ((phase == 1u && PE_LoadU8(B_FLAG_58B) == 0) || phase == 3u) {
            if ((int32_t)RAM_WORD(W_TICK_B) <= 0) {
                PE_StoreU32(W_ISSUE_CMD, 0x21u);
                (void)pe_plat_cd_issue_wrapper(1u, 0);
            }
        }
    }
}

/* ---- 0x8008080C: response status decode ----------------------------- */
/* The index n of the status byte in the response is -1 + the word at
 * 0x8009B624 + 4 * byte[0x8009B558] (a response-length table); status 5
 * always uses index 0 without reading the byte or the table word.  Nothing
 * happens unless n >= 0 as a signed word.  The status byte l = response[n]:
 * bytes 0x8009B588..B58B become bits 7, 6, 5 and 1 of l (0 or 1), byte
 * 0x8009B56C = l, and the first 8 response bytes are copied to 0x8009B564..B56B
 * (a null response pointer instead clears only byte 0x8009B564).  The status
 * argument matters only as (status & 0xFF) == 5. */
void pe_plat_cdxstream_decode_status(int status, uint32_t response)
{
    int32_t n;
    uint8_t last;
    unsigned i;
    if ((status & 0xFF) == 5) {
        n = 0;
    } else {
        uint32_t w = RAM_WORD(0x8009B624u + 4u * (uint32_t)PE_LoadU8(B_STATE_LAST));
        n = (int32_t)(w - 1u);
    }
    if (n < 0) return;
    last = PE_LoadU8(response + (uint32_t)n);
    PE_StoreU8(B_FLAG_588, (uint8_t)((last >> 7) & 1u));
    PE_StoreU8(B_FLAG_588 + 1u, (uint8_t)((last >> 6) & 1u));
    PE_StoreU8(B_FLAG_58A, (uint8_t)((last >> 5) & 1u));
    PE_StoreU8(B_FLAG_58B, (uint8_t)((last >> 1) & 1u));
    PE_StoreU8(B_DRIVE_STAT, last);
    if (response == 0) { PE_StoreU8(0x8009B564u, 0); return; }
    for (i = 0; i < 8u; i++) PE_StoreU8(0x8009B564u + i, PE_LoadU8(response + i));
}

/* ---- 0x80080404: command progression -------------------------------- */
/* Word 0x8009B578 = state, 0x8009B574 = phase, byte 0x8009B56C = drive status
 * (bit 1 motor, bit 4 shell open).  Steps, in order:
 *  1. state 0xC becomes 0xD.
 *  2. (status & 0xFF) != 2: when the shell bit (0x10) is set phase = 2 and
 *     state = 0xC and the call ends; otherwise state (re-read) 0x10 / 0x11 gives
 *     phase 1, state 0xB.  Nothing else happens for these statuses.
 *  3. status 2, by state (re-read):
 *       0xD  when the shell bit is clear: word 0x8009B590 += 1, state 0xE,
 *            phase 2, sub-step 0x8009B57C = 0x15;
 *       0xE  by sub-step: 0x15 -- shell bit clear: phase 2, state 0xE, sub-step
 *            0x16, word 0x8009B58C = 0;  0x16 -- when drive bit 1 is set: phase 2,
 *            state 0xE, sub-step 0x17; otherwise when word 0x8009B58C > 0x12C
 *            (signed): phase 3 and slot 0x800A36AC (if nonzero) is called with
 *            (5, response);  0x17 -- phase 2, state 0xE, sub-step 0x18;  0x18 --
 *            when bit 1 is set and the shell bit clear: phase 1, state 0xB,
 *            sub-step 0 and slot 0x800A36AC (if nonzero) is called with
 *            (2, response);  other sub-steps do nothing;
 *       0x10 / 0x11  when countdown 0x8009B5A0 is 0 and drive bit 1 is clear:
 *            phase 1, state 0xB, then slot 0x800A36A8 and slot 0x800A36A4 are
 *            each called with (5, response) when nonzero and the gate word
 *            0x8009B554 is nonzero;
 *       other states do nothing.
 * All stores of a step precede its callback calls.  Unsampled choices: the
 * comparison of word 0x8009B5A0 (== 0 vs <= 0) and of the drive byte for the
 * 0x18 and 0x15 tests (bit tests were used; the vectors only carry the values
 * 0x00 / 0x02 / 0x10 / 0x12 there). */
static int cdxs_progress_slot(uint32_t slot_addr, const char *name, uint32_t status, uint32_t response, int gated)
{
    uint32_t fn = RAM_WORD(slot_addr);
    if (fn == 0) return 1;
    if (gated && RAM_WORD(W_GATE) == 0) return 1;
    fn = RAM_WORD(slot_addr);
    return cdxs_call_slot(name, "func_80080404", fn, 2, status, response);
}

void pe_plat_cdxstream_command_progress(unsigned char status, int response)
{
    uint32_t state = RAM_WORD(W_STATE);
    uint32_t resp = (uint32_t)response;
    if (state == 0xCu) PE_StoreU32(W_STATE, 0xDu);
    if (status != 2u) {
        if (PE_LoadU8(B_DRIVE_STAT) & 0x10u) {
            PE_StoreU32(W_PHASE, 2u);
            PE_StoreU32(W_STATE, 0xCu);
            return;
        }
        state = RAM_WORD(W_STATE);
        if (state == 0x10u || state == 0x11u) {
            PE_StoreU32(W_PHASE, 1u);
            PE_StoreU32(W_STATE, 0xBu);
        }
        return;
    }
    state = RAM_WORD(W_STATE);
    switch (state) {
    case 0xDu:
        if (!(PE_LoadU8(B_DRIVE_STAT) & 0x10u)) {
            PE_StoreU32(W_ODD_COUNT, RAM_WORD(W_ODD_COUNT) + 1u);
            PE_StoreU32(W_STATE, 0xEu);
            PE_StoreU32(W_PHASE, 2u);
            PE_StoreU32(W_SUBSTATE, 0x15u);
        }
        break;
    case 0xEu:
        switch (RAM_WORD(W_SUBSTATE)) {
        case 0x15u:
            if (!(PE_LoadU8(B_DRIVE_STAT) & 0x10u)) {
                PE_StoreU32(W_PHASE, 2u);
                PE_StoreU32(W_STATE, 0xEu);
                PE_StoreU32(W_SUBSTATE, 0x16u);
                PE_StoreU32(W_SPIN_COUNT, 0);
            }
            break;
        case 0x16u:
            if (PE_LoadU8(B_DRIVE_STAT) & 0x02u) {
                PE_StoreU32(W_PHASE, 2u);
                PE_StoreU32(W_STATE, 0xEu);
                PE_StoreU32(W_SUBSTATE, 0x17u);
            } else if ((int32_t)RAM_WORD(W_SPIN_COUNT) > 0x12C) {
                PE_StoreU32(W_PHASE, 3u);
                (void)cdxs_progress_slot(W_SLOT_PROG_C, "CD_data_callback", 5u, resp, 0);
            }
            break;
        case 0x17u:
            PE_StoreU32(W_PHASE, 2u);
            PE_StoreU32(W_STATE, 0xEu);
            PE_StoreU32(W_SUBSTATE, 0x18u);
            break;
        case 0x18u: {
            uint8_t s = PE_LoadU8(B_DRIVE_STAT);
            if ((s & 0x02u) && !(s & 0x10u)) {
                PE_StoreU32(W_PHASE, 1u);
                PE_StoreU32(W_STATE, 0xBu);
                PE_StoreU32(W_SUBSTATE, 0);
                (void)cdxs_progress_slot(W_SLOT_PROG_C, "CD_data_callback", 2u, resp, 0);
            }
            break;
        }
        default:
            break;
        }
        break;
    case 0x10u: case 0x11u:
        if ((int32_t)RAM_WORD(W_TICK_C) <= 0 && !(PE_LoadU8(B_DRIVE_STAT) & 0x02u)) {
            PE_StoreU32(W_PHASE, 1u);
            PE_StoreU32(W_STATE, 0xBu);
            if (cdxs_progress_slot(W_SLOT_PROG_B, "CD_data_callback", 5u, resp, 1))
                (void)cdxs_progress_slot(W_SLOT_PROG_A, "CD_data_callback", 5u, resp, 1);
        }
        break;
    default:
        break;
    }
}

/* ---- 0x8007BBFC: device init ---------------------------------------- */
/* Clear the callback / guard words, reset the callback table, register the
 * interrupt callback (source 2, handler 0x8007C13C), select register index 1
 * and acknowledge every pending interrupt (while the flag register's bits
 * 0..2 are set: index 1, flag 7, enable 7), clear the sync bytes, select
 * index 0, clear the request register, write the COM_DELAY word 0x1325, then
 * issue commands 1 and 0xA (zero arguments); only when 0xA succeeded
 * (result 0) command 0xC is issued and the poll runs with (0, 0).  The result
 * is always -1.  The vectors do not show whether command 1's or command 0xA's
 * result gates 0xC (the scripted result is constant per case). */
int pe_plat_cdxstream_initialize_device(void)
{
    uint32_t budget = 0;
    PE_StoreU8(0x8009AFD5u, 0);
    PE_StoreU8(0x8009AFD4u, 0);
    PE_StoreU32(0x8009AFB8u, 0);
    PE_StoreU32(0x8009AFB4u, 0);
    PE_StoreU32(0x8009AFC8u, 0);
    PE_StoreU32(0x8009AFC4u, 0);
    (void)pe_plat_reset_callback();
    (void)pe_plat_interrupt_callback(2u, 0x8007C13Cu);
    PE_CdStoreU8(RAM_WORD(W_REG_INDEX), 1u);
    if (PE_Port_ShouldStop()) return -1;   /* a host quit raised before the acknowledge loop ends the init at once */
    if (PE_CdReg_DeviceEnabled()) {
        while ((PE_CdLoadU8(RAM_WORD(W_REG_3)) & 7u) != 0) {
            if (PE_Port_ShouldStop()) return -1;
            if (budget++ >= POLL_BUDGET) {
                PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
                return -1;
            }
            PE_CdStoreU8(RAM_WORD(W_REG_INDEX), 1u);
            PE_CdStoreU8(RAM_WORD(W_REG_3), 7u);
            PE_CdStoreU8(RAM_WORD(W_REG_2), 7u);
            pe_plat_sio_poll_tick();
        }
    }
    PE_StoreU8(0x8009B296u, 0);
    PE_StoreU8(0x8009B295u, 0);
    PE_StoreU8(0x8009B294u, 2u);
    PE_CdStoreU8(RAM_WORD(W_REG_INDEX), 0);
    PE_CdStoreU8(RAM_WORD(W_REG_3), 0);
    PE_CdStoreU32(RAM_WORD(W_REG_DELAY), 0x1325u);
    (void)pe_plat_cd_command_issue_host(1u, 0, 0, 0);
    if (pe_plat_cd_command_issue_host(0xAu, 0, 0, 0) == 0) {
        (void)pe_plat_cd_command_issue_host(0xCu, 0, 0, 0);
        (void)pe_plat_cd_status_poll_host(0, 0);
    }
    return -1;
}
