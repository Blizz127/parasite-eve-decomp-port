/* libds queue command (retail entry 0x8007EE84, public name pe_plat_cd_queue_command_host).
 *
 * PROVENANCE (clean-room, black-box).  Written by the sw-cdrw-c author (Sonnet 5.5, 2026-11-27) from:
 *   - the retail-derived black-box vector sets cdgen56 (queue.txt, 1000 cases) and cdgen56b (queue_deep.txt, 1500 cases: NULL
 *     parameter, queue depth edges, status-lane result 1, the queue head match that reaches the text helper), read through
 *     their columns only (A arguments, L ordered loads, S stores, RETS, CALL); the lane reports
 *     docs/evidence/native-cdx-20261126/cdgen56/REPORT.md and cdgen56b/REPORT.md; the checker
 *     pc_port/tools/pe_cdx_cdgen56_check.c on pe_cdx_common_canary.h;
 *   - the suite tests pc_port/tests/test_cd_queue_native.h (NULL parameter, serial wrap, full queue) and
 *     test_cd_command_queue.h (the command table words at 0x8009B4BC, queue head / depth words);
 *   - public headers pe_guest_ram.h, pe_plat/cd.h and psx-spx (CD-ROM command 0x02 Setloc).
 *   NOT read: the previous provider pe_plat_cd_queue_command_host.c (never opened), src/, asm/, vendor/, game/decomp_hand,
 *   game/boot, pe_lib*.c, pe_stream.c, *_retail.c, oracle sources, retail_*_cases.h, the vector generator.  Nothing is
 *   claimed about how the original was written.
 *
 * Arguments (command, param, extra, callback): the drive command (only its low byte counts), the guest pointer to the command's
 * four parameter bytes or 0, a user word and a user callback.  Guest words (little endian):
 *   0x8009B4BC + 4*cmd  per-command table word (nonzero: the command needs a preceding Setloc);
 *   0x800A3608          queue depth (signed); 0x800A3600 / 0x800A3604 queue head words; 0x8009B53C serial;
 *   0x800A3540 + 24*i   the 24-byte records (+0 serial, +4 command byte, +5..+8 parameter copy, +0xC parameter, +0x10 extra,
 *                       +0x14 callback; written through pe_plat_ds_cq_next_slot() (0x8007E6B0)).
 * Behaviour (cdgen56 / 56b):
 *   1. entries needed: two (a Setloc, command 2, carrying the parameter, then the command) when the table word of the command is
 *      nonzero and the parameter is nonzero, otherwise one.  Each entry is added by the same step, the capacity test applied to
 *      each one in turn (a signed depth of 8 or more rejects: result 0, no store for that entry; the entry before it stays);
 *   2. step: serial word := old + 1 (an old value of 0xFFFFFFFF gives 0 and then 1: two stores), slot = next slot; slot +0 := serial,
 *      +4 := command byte, when the parameter is nonzero the four-byte copy (0x80080950)(slot + 5, parameter), then +0xC :=
 *      parameter, +0x14 := callback (0 for the Setloc), +0x10 := extra (0 for the Setloc), depth := depth + 1;
 *      then the status lane (0x8007FBF0)(0); when it answers 1 and the record at queue head (word 0x800A3604) holds the serial just
 *      assigned the text helper (0x8007E8F4) runs (cdgen56b: 159 cases, always lane 1 and a matching head);
 *   3. result = the serial of the last entry added.
 * The text helper's stand-in returns nothing the vectors use. */
#include <stdint.h>

#include "pe_guest_ram.h"
#include "pe_plat/cd.h"

#define W_SERIAL      0x8009B53Cu
#define W_QUEUE_HEAD  0x800A3604u
#define W_QUEUE_DEPTH 0x800A3608u
#define T_NEEDS_LOC   0x8009B4BCu
#define QUEUE_BASE    0x800A3540u
#define QUEUE_STRIDE  24u
#define CMD_SETLOC    2u

/* Returns the serial of the entry added, or 0 when the queue is full. */
static uint32_t add_entry(uint32_t cmd, uint32_t param, uint32_t extra, uint32_t callback)
{
    uint32_t serial, slot, head;
    if ((int32_t)PE_LoadU32(W_QUEUE_DEPTH) >= 8) return 0u;
    serial = PE_LoadU32(W_SERIAL) + 1u;
    PE_StoreU32(W_SERIAL, serial);
    if (serial == 0u) { serial = 1u; PE_StoreU32(W_SERIAL, serial); }
    slot = pe_plat_ds_cq_next_slot();
    PE_StoreU32(slot, serial);
    PE_StoreU8(slot + 4u, (uint8_t)cmd);
    if (param != 0u) pe_plat_cd_copy4(slot + 5u, param);
    PE_StoreU32(slot + 0xCu, param);
    PE_StoreU32(slot + 0x14u, callback);
    PE_StoreU32(slot + 0x10u, extra);
    PE_StoreU32(W_QUEUE_DEPTH, PE_LoadU32(W_QUEUE_DEPTH) + 1u);
    if (pe_plat_ds_status_lane(0) == 1) {
        head = PE_LoadU32(W_QUEUE_HEAD);
        if (PE_LoadU32(QUEUE_BASE + QUEUE_STRIDE * head) == serial) (void)pe_plat_ds_text_360();
    }
    return serial;
}

uint32_t pe_plat_cd_queue_command_host(uint32_t command, uint32_t param, uint32_t extra, uint32_t callback)
{
    uint32_t cmd = command & 0xFFu, serial;

    if (param != 0u && PE_LoadU32(T_NEEDS_LOC + 4u * cmd) != 0u) {
        serial = add_entry(CMD_SETLOC, param, 0u, 0u);
        if (serial == 0u) return 0u;
    }
    return add_entry(cmd, param, extra, callback);
}
