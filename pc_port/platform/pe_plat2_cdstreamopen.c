/* libds read-stream open (retail entry 0x80081314, public name pe_plat_cd_read_stream_host).
 *
 * PROVENANCE (clean-room, black-box).  Written by the sw-cdrw-c author (Sonnet 5.5, 2026-11-27) from:
 *   - the retail-derived black-box vector set cdgen57 (open.txt, 800 cases), read through its columns only (A arguments,
 *     S stores, RETS scripted callee returns, CALL recorded callee calls with arguments); the lane report
 *     docs/evidence/native-cdx-20261126/cdgen57/REPORT.md; the checker pc_port/tools/pe_cdx_cdgen57_check.c on
 *     pe_cdx_common_canary.h;
 *   - the suite test pc_port/tests/test_cd_read_stream_native.h (host feeder opens, stream flag, serial result);
 *   - public headers pe_guest_ram.h, pe_plat/cd.h, pe_plat/cd_regs.h, pe_plat/stream_feed.h (PE_StrFeed_Probe / PE_StrFeed_Open
 *     comments), psx-spx (CD-ROM setmode bit 5 "sector size", XA streaming).
 *   NOT read: the previous provider pe_plat_cd_read_stream_host.c (never opened), src/, asm/, vendor/, game/decomp_hand,
 *   game/boot, pe_lib*.c, pe_stream.c, *_retail.c, oracle sources, retail_*_cases.h.  Nothing is claimed about how the
 *   original was written.
 *
 * Arguments (location, mode): location is the guest pointer to the three-byte BCD position, mode the drive mode word.
 * Behaviour (cdgen57; "stream" = bit 0x100 of mode, true in 267 of 800 cases; the two registration entries are called 404 times in total: 267 registrations + 137 undo calls):
 *   non-stream: one call, the command sequence (0x8007F0C8) with (mode & 0xFF, location, 0x1B, 0, 0xFFFFFFFF); its result is the
 *       result; no store, no other call (533 cases).
 *   stream: 0x800A8020 := (mode & 0x20) ? 0 : 1 (the only store, before every call); data callback registration
 *       (0x800824F0)(0x8007C214); ready callback registration (0x800824C8)(0x800813E8); the command sequence as above; when
 *       its result is 0 both registrations are undone by registering the values the two registration calls returned (data callback
 *       first, then the ready callback; the scripted returns are copied back, cdgen57 304 cases);
 *       any other result (1 and 0xFFFFFFFF both) leaves them in place.  The result of the sequence is returned in every arm.
 * Host path (absent from the vectors, which run with the controller model enabled; found by running the port: with the host
 * sector feeder, route and suite): in the stream arm, after the two registrations, when the modeled controller is disabled and the
 * position is a real STR/XA sector (PE_StrFeed_Probe), NO command sequence is issued: the serial word 0x8009B53C advances as a
 * queued sequence would advance it (old + 1, zero skipped), the host sector feeder is opened (PE_StrFeed_Open(location, mode),
 * stream_feed.h: "called when the guest opens a stream while the device is disabled") and the serial is the result
 * (suite CDREAD_native_stream_sequence; observed in the route: no sequence / issue call at a movie start, registrations
 * 0x8007C214 / 0x800813E8 then the feeder open). */
#include <stdint.h>

#include "pe_guest_ram.h"
#include "pe_plat/cd.h"
#include "pe_plat/cd_regs.h"
#include "pe_plat/stream_feed.h"

#define W_STREAM_FLAG      0x800A8020u
#define W_SERIAL           0x8009B53Cu
#define MODE_STREAM        0x100u
#define MODE_SECTOR_SIZE   0x20u
#define CMD_READ_STREAM    0x1Bu
#define CB_DATA_READY      0x8007C214u
#define CB_READY_SETTER    0x800813E8u
#define CB_NONE            0xFFFFFFFFu

int pe_plat_cd_read_stream_host(uint32_t location, uint32_t mode)
{
    int stream = (mode & MODE_STREAM) != 0u;
    int result;
    uint32_t prev_data = 0u;
    int prev_ready = 0;

    if (stream) {
        PE_StoreU32(W_STREAM_FLAG, (mode & MODE_SECTOR_SIZE) ? 0u : 1u);
        prev_data = pe_plat_ds_data_callback(CB_DATA_READY);
        prev_ready = pe_plat_cd_ds_set_ready_callback((int)CB_READY_SETTER);
        if (!PE_CdReg_DeviceEnabled() && PE_StrFeed_Probe(location)) {
            /* host feeder takeover: no command sequence reaches the (disabled) drive model; the serial word advances
             * exactly like a queued sequence and is the result */
            uint32_t serial = PE_LoadU32(W_SERIAL) + 1u;
            if (serial == 0u) serial = 1u;
            PE_StoreU32(W_SERIAL, serial);
            PE_StrFeed_Open(location, mode);
            return (int)serial;
        }
    }
    result = pe_plat_cd_command_sequence_host(mode & 0xFFu, location, (int)CMD_READ_STREAM, 0u, CB_NONE);
    if (stream) {
        if (result == 0) {
            (void)pe_plat_ds_data_callback(prev_data);
            (void)pe_plat_cd_ds_set_ready_callback(prev_ready);
        }
    }
    return result;
}
