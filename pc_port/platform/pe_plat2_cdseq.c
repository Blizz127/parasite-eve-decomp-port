/* libds positioned-command sequence (retail entry 0x8007F0C8, public name pe_plat_cd_command_sequence_host).
 *
 * PROVENANCE (clean-room, black-box).  Written by the sw-cdrw-c author (Sonnet 5.5, 2026-11-27) from:
 *   - the retail-derived black-box vector sets cdgen55 (sequence.txt, 782 cases, early exit only) and cdgen55b
 *     (sequence_deep.txt, 1500 cases, 542 deep-arm cases; the variant that logs the original's partly initialised stack local
 *     as external bytes), read through their columns only (A arguments, L ordered loads, S stores, RETS, CALL); the lane
 *     reports docs/evidence/native-cdx-20261126/cdgen55/REPORT.md and cdgen55b/REPORT.md; the checker
 *     pc_port/tools/pe_cdx_cdgen55_check.c on pe_cdx_common_canary.h;
 *   - the suite test pc_port/tests/test_cd_sequence_native.h (four records, serial wrap, mode padding, capacity rejection and
 *     the location scratch word);
 *   - public headers pe_guest_ram.h, pe_plat/cd.h, and psx-spx (CD-ROM commands 0x02 Setloc, 0x09 Pause, 0x0E Setmode, 0x03
 *     Play, 0x15 SeekL, 0x16 SeekP, 0x1B ReadS).
 *   NOT read: the previous provider pe_plat_cd_command_sequence_host.c (never opened), src/, asm/, vendor/, game/decomp_hand,
 *   game/boot, pe_lib*.c, pe_stream.c, *_retail.c, oracle sources, retail_*_cases.h, the vector generator.  Nothing is
 *   claimed about how the original was written.
 *
 * Arguments (mode, location, command, extra, callback): location = guest pointer to the position record (the three BCD bytes
 * plus one), command = the drive command (only its low byte counts).  Behaviour:
 *   1. position conversion pe_plat_cd_pos_to_int(location) (0x80080C48); a negative result returns 0 with nothing else done
 *      (cdgen55b: all 600+ negative cases read nothing but the callback argument);
 *   2. only commands 3, 0x15, 0x16, 0x1B continue; any other low byte returns 0.  The position record word is loaded and kept in
 *      the host scratch word 0x801FFE80 (the suite test expects it there even for a rejected sequence);
 *   3. capacity: the queue depth word 0x800A3608, taken as a signed word plus four, must be at most 8 (so depth 5..8 reject;
 *      the wrap of 0x7FFFFFFF accepts, as do negative depths, as cdgen55b shows); a rejected sequence returns 0, the serial is not
 *      touched;
 *   4. serial 0x8009B53C := old + 1 (zero skipped to 1), a single store; all four records carry it;
 *   5. four records appended with pe_plat_ds_cq_next_slot() (0x8007E6B0), each laid out as: +0 serial, +4 command byte,
 *      +5..+8 payload, +0xC parameter pointer, +0x14 callback, +0x10 extra; then the depth word := depth + 1 after each:
 *        Pause 9 (no payload, pointer 0, extra 0);
 *        Setmode 0xE (payload: mode byte, then three zero bytes, pointer = record + 5, extra 0);
 *        Setloc 2 (payload: the four bytes of the position record word, pointer = record + 5, extra 0);
 *        command (low byte, pointer 0, extra = the extra argument);
 *      the callback argument is stored in all four records;
 *      a slot pointer of 0 ends the routine at once with result 0 (the serial and the records already added stay;
 *      cdgen55c lines 96, 98: allocator results of zero for the first and the third record);
 *   6. one pe_plat_ds_status_lane(0) (0x8007FBF0) after the fourth record; result = the serial.
 * The lane value is not used by anything the vectors reach (scripted 5, 2, 0x10, 0x80, ...). */
#include <stdint.h>

#include "pe_guest_ram.h"
#include "pe_plat/cd.h"

#define W_SERIAL      0x8009B53Cu
#define W_QUEUE_DEPTH 0x800A3608u
#define W_LOC_SCRATCH 0x801FFE80u
#define W_QUEUE_HEAD  0x800A3604u
#define QUEUE_BASE    0x800A3540u
#define QUEUE_STRIDE  24u

#define CMD_SETLOC    2u
#define CMD_PAUSE     9u
#define CMD_SETMODE   0xEu

static int sequence_command(uint32_t cmd)
{
    return cmd == 3u || cmd == 6u || cmd == 0x15u || cmd == 0x16u || cmd == 0x1Bu;
}

static void finish_record(uint32_t slot, uint32_t pointer, uint32_t extra, uint32_t callback)
{
    PE_StoreU32(slot + 0xCu, pointer);
    PE_StoreU32(slot + 0x14u, callback);
    PE_StoreU32(slot + 0x10u, extra);
    PE_StoreU32(W_QUEUE_DEPTH, PE_LoadU32(W_QUEUE_DEPTH) + 1u);
}

int pe_plat_cd_command_sequence_host(uint32_t mode, uint32_t location, int command, uint32_t extra, uint32_t callback)
{
    uint32_t cmd = (uint32_t)command & 0xFFu, depth, serial, loc, slot, i;
    int pos;

    pos = pe_plat_cd_pos_to_int(location);
    if (pos < 0) return 0;
    if (!sequence_command(cmd)) return 0;
    loc = PE_LoadU32(location);
    PE_StoreU32(W_LOC_SCRATCH, loc);
    depth = PE_LoadU32(W_QUEUE_DEPTH);
    if ((int32_t)(depth + 4u) > 8) return 0;
    serial = PE_LoadU32(W_SERIAL) + 1u;
    if (serial == 0u) serial = 1u;
    PE_StoreU32(W_SERIAL, serial);

    slot = pe_plat_ds_cq_next_slot();
    if (slot == 0u) return 0;
    PE_StoreU32(slot, serial);
    PE_StoreU8(slot + 4u, (uint8_t)CMD_PAUSE);
    finish_record(slot, 0u, 0u, callback);

    slot = pe_plat_ds_cq_next_slot();
    if (slot == 0u) return 0;
    PE_StoreU32(slot, serial);
    PE_StoreU8(slot + 4u, (uint8_t)CMD_SETMODE);
    PE_StoreU8(slot + 5u, (uint8_t)mode);
    PE_StoreU8(slot + 6u, 0u);
    PE_StoreU8(slot + 7u, 0u);
    PE_StoreU8(slot + 8u, 0u);
    finish_record(slot, slot + 5u, 0u, callback);

    slot = pe_plat_ds_cq_next_slot();
    if (slot == 0u) return 0;
    PE_StoreU32(slot, serial);
    PE_StoreU8(slot + 4u, (uint8_t)CMD_SETLOC);
    for (i = 0; i < 4u; i++) PE_StoreU8(slot + 5u + i, (uint8_t)(loc >> (8u * i)));
    finish_record(slot, slot + 5u, 0u, callback);

    slot = pe_plat_ds_cq_next_slot();
    if (slot == 0u) return 0;
    PE_StoreU32(slot, serial);
    PE_StoreU8(slot + 4u, (uint8_t)cmd);
    finish_record(slot, 0u, extra, callback);

    if (pe_plat_ds_status_lane(0) == 1 && PE_LoadU32(QUEUE_BASE + QUEUE_STRIDE * PE_LoadU32(W_QUEUE_HEAD)) == serial)
        (void)pe_plat_ds_text_360();
    return (int)serial;
}
