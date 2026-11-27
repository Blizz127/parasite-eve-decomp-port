/* CD stream data-ready handler (retail entry 0x8007C564, `void pe_plat_cdhost_stream(void)`): takes the next sector of the
 * CD stream, checks its header, assembles the sectors of one frame into a ring of 32-byte records plus one 2016-byte payload
 * buffer per record, and reports end of stream / ring wrap to the stream owner.
 *
 * Written clean-room from black-box vector sets only; no SDK source, decompiled source, tables or disassembly were read, and
 * no claim is made about how the original was written.  PROVENANCE (everything the author read for this file):
 *   - vector sets (retail-derived, never committed): own/cdgen47/stream.txt (778 cases, sha256 in
 *     docs/evidence/native-cdx-20261126/cdgen47/VECTORS_SHA256SUMS) and the deep set stream_deep.txt of the vector lane
 *     sw-vec-stream-deep (1080 cases, sha256 16748208e6d6d95c32034051f9df59978a655c5d75c5ccf13ec83a9450a79328), read through
 *     their L (ordered loads) / S (stores) / RETS / CALL columns and run through the checkers
 *     pc_port/tools/pe_cdx_cdgen47_check.c and pe_cdx_streamdeep_check.c with pe_cdx_common.h;
 *   - docs/evidence/native-cdx-20261126/cdgen47/REPORT.md and .../cdgen47_stream_deep/REPORT.md, the deep set's .classes file,
 *     pe_cdx_streamdeep_run.sh and the deep generator pe_cdx_streamdeep_vectors.py (read only for the map of seeded state words);
 *   - public headers: pe_guest_ram.h, pe_guestcode.h, pe_plat/cd.h, cd_regs.h, cdhost.h, framebuffer_api.h, bootstrap/game_port.h;
 *   - psx-spx (CD-ROM controller, DMA3 / CHCR, DICR).
 *   NOT read: the previous provider of this entry, src/, asm/, vendor/, decomp_hand/, game/boot bodies, pe_lib*.c, pe_stream.c,
 *   any *_retail.c, oracle sources, retail_*_cases.h.
 *
 * Guest words (little endian, each read at every use; "v" = deep-set line numbers that show it):
 *   0x800B89F4  busy: == 1 -> return at once (v11-14); set to 1 when the frame's last record is taken (v3); a nonzero value at the
 *               end calls the data-ready callback 0x8007C214 (v3 final record -> call; v67 value 2 -> call; v1 value 0 -> none)
 *   0x800A801C  mode flag: != 0 -> the MDEC-out channel is checked first and the CD bus timing pair 0x20943 / 0x1323 is used for
 *               the payload; == 0 -> a single bus word 0x21020843 (v9)
 *   0x8009B34C  pointer to the MDEC-out DMA1 channel control (0x1F801098); bit 24 busy -> defer (state 1)
 *   0x800B0CD0  defer flag (set to 1 on defer)
 *   0x800C0DB8  source buffer pointer (0 = the data arrive by DMA3 / FIFO); 0x800BCD7C sector index inside it: the sector
 *               is read at source + 2048 * index (v7: index 1 -> 0x80160800); the index is advanced by one after every sector taken
 *               or rejected by the frame logic (v1, v33, v41), reset to 0 for a bad sector (v37) and left alone without a source
 *   0x8009B374  state: 1 deferred (v15), 3 error status in the cd_ready result (v72), 4 record occupied (v27), 5 bad sector (v37, v39),
 *               6 frame aborted: part / frame mismatch (v41-45), 7 end of stream (v47-51), 8 ring wrap pad (v53, v55),
 *               9 wrap blocked (v57), 10 payload taken (v1)
 *   0x800BE998  index of the current ring record, 0x800C0DC8 ring base, 0x800C20C4 ring size, 0x800A34A0 current record address,
 *               record = base + 32 * index; payload buffer = base + 32 * ring size + 2016 * index, stored in 0x800C0DC4 (v1, v61)
 *   0x800BE9E4  ring index of the first record of the frame being assembled (v1; rewind loop v41)
 *   0x800A8018  halfword (signed): number of the next part of the frame; 0x800A5D54 the frame number in progress (0 = none)
 *   0x800C0DBC  frame number at which the stream ends (0 = none; v47 limit 7, v49 limit 6, v63 limit -1 = ignored); also the flag
 *               that allows a wrap onto the first record (v53 without limit pads the old record, v57 / v59 with limit)
 *   0x800C0DC0  == 1 while waiting for the start of a frame, 0x800B6918 the wanted frame number (0 = any): a record of another
 *               frame is dropped without a state store (v33); the match clears the flag (v35)
 *   0x800B8620  expected channel (record halfword 1 >> 10, v39); 0x800B6914 the channel after a completed frame (v3)
 *   0x800B0CCC  owner callback word, called with one argument through the guest-hook seam: at end of stream with the frame's first
 *               ring index (v51, v607 = 0, v608 = -1), at a wrap pad with the pad record's address (v55)
 *   record header (words at record + 0, the sector's first 20 bytes): halfword 0 magic 0x160 (v37), 1 channel << 10 (v39),
 *               2 part number (v41; compared with the signed counter, v43), 3 number of parts, 4 frame number (v45); the word at
 *               record + 0x1C is the first four bytes (little endian) of the 12 FIFO bytes read by hand when 0x800A8020 == 0, else 0.
 *               The vectors cannot tell which four: in all 106 FIFO cases (v29) the twelve register reads return one repeated value;
 *               the first four are the suite's evidence (the sector header ends up at record + 0x1C in its disc-fixture test).
 *
 * Flow (every step is a vector-verified store / load sequence; order of stores and calls is the original's):
 *   1. busy guard; MDEC gate (defer); cd_ready(1, frame buffer): result 5 -> return without a store (v69, v190); an error bit of the
 *      first result byte (& 0x1D) -> state 3 (see "open", only seen with a result aliasing the source window);
 *   2. record occupied (halfword at record != 0) -> sector index +1, state 4;
 *   3. CD request (index 0 / request 0 twice, then 0x80), bus 0x20943 / mailbox 0x1323, FIFO bytes, record fetch (DMA3 8 words, or
 *      8-word copy from the source), idle wait, word at record + 0x1C twice, bus 0x20843 / mailbox 0x1325;
 *   4. start gate, magic, channel -> bad sector (index reset, state 5, record freed);
 *   5. part number against the counter: mismatch -> abort the frame (frame / counter cleared, records of the frame freed, ring index back
 *      to the frame start), index +1, state 6; same for a different frame number;
 *   6. first record of a frame (no frame in progress, part 0): counter 0, frame number stored; frame limit reached -> abort, start-wait
 *      flag, owner callback, state 7; frame does not fit in the records left in the ring -> wrap (pad / wait / move to record 0);
 *      frame-first index stored;
 *   7. state 10, counter + 1, payload destination, bus words, busy flag when it is the last part, payload copy (504 words) or DMA3,
 *      sector index + 1, last part: counter / frame cleared and channel advanced, mailbox 0x1325, record halfword 3, ring index + 1,
 *      data-ready callback when the busy word is nonzero.
 *
 * Host contract (not observable by vectors): a DMA3 wait is bounded by the stuck-wait budget and ends in the named boundary
 * func_8007C564_dma3_wait (ShouldStop exit, per-poll tick + DMA3 service); no static state (every case is independent).
 *
 * Open / not representable:
 *   - Deep-set cases 72, 74, 205, 207: the source pointer 0x801FEFC0 puts a copy of the sector in the original's own stack frame, so
 *     the two result bytes cd_ready leaves in that frame read 0x55 / 0x35 and 0x66 / 0x46 and the original stops in state 3.  The
 *     checker compares frame addresses as offsets, and this handler's frame is a guest-frame arena buffer, not the retail
 *     stack address 0x801FEFE0 (inside the host scratch window), so the aliased bytes cannot reach it.  A frame at the retail stack
 *     address is a host decision (it collides with the host scratch windows), not taken here.  Which result bytes / bits decide
 *     state 3 is not distinguishable (all four have bit 0 set in byte 0, 0x1D mask used: the CD error bits of psx-spx).
 *   - The payload fetch by DMA3 (no source buffer) is never reached by a vector (every DMA-fetched record is rejected: the
 *     checker's DMA stand-in writes nothing); its call (channel 3, payload address, 0 blocks, 504 words, control 0x11000000,
 *     interrupt 1) follows the header fetch and the suite's DMA3 final-register expectations.
 *   - A wait on a permanently busy DMA3 channel and the suite's other named boundaries (sector FIFO, auxiliary callback) are
 *     host-side stops of the previous provider; the original continues in those states, so does this handler. */
#include <stdint.h>

#include "pe_guest_ram.h"
#include "game_port.h"
#include "pe_plat/cd_regs.h"
#include "pe_plat/cd.h"
#include "pe_plat/cdhost.h"
#include "pe_plat/framebuffer_api.h"
#include "pe_guestcode.h"

#define W_STREAM_BUSY      0x800B89F4u  /* == 1: nothing to do                                     */
#define W_MDEC_GATE        0x800A801Cu  /* != 0: check the MDEC-out DMA first                      */
#define W_PTR_MDEC_CHCR    0x8009B34Cu  /* pointer word -> 0x1F801098                              */
#define W_DEFER_FLAG       0x800B0CD0u  /* set to 1 when deferred                                  */
#define W_CALLBACK         0x800C0DB8u  /* data callback word (source of the 8-word copy)           */
#define W_SECTOR           0x800BCD7Cu  /* sector index inside the source buffer (2048 bytes each) */
#define W_STATE            0x8009B374u  /* stream state: 1 deferred, 4 waiting, 5 record fetched    */
#define W_SLOT_INDEX       0x800BE998u
#define W_SLOT_BASE        0x800C0DC8u
#define W_SLOT_CURRENT     0x800A34A0u  /* receives the slot address                               */
#define W_PTR_CD_INDEX     0x8009B32Cu  /* -> 0x1F801800                                           */
#define W_PTR_CD_REQUEST   0x8009B338u  /* -> 0x1F801803                                           */
#define W_PTR_BUS_CONTROL  0x8009B33Cu  /* -> 0x1F801018                                           */
#define W_PTR_CD_DELAY     0x8009B340u  /* -> 0x1F801020                                           */
#define W_PTR_CD_DATA      0x8009B334u  /* -> 0x1F801802                                           */
#define W_PTR_DMA3_CHCR    0x8009B35Cu  /* -> 0x1F8010B8                                           */
#define W_FIFO_MODE        0x800A8020u  /* == 0: read the data FIFO by hand                        */
#define W_START_WAIT       0x800C0DC0u  /* == 1: ignore sectors until the wanted frame number      */
#define W_START_NUMBER     0x800B6918u  /* the wanted frame number (0: any)                        */
#define W_CHANNEL          0x800B8620u  /* expected channel of the sectors (header halfword 1 >> 10) */
#define W_CHANNEL_NEXT     0x800B6914u  /* channel after a completed frame                         */
#define W_PART             0x800A8018u  /* halfword: number of the next record of the frame        */
#define W_FRAME            0x800A5D54u  /* frame number being assembled (0: none)                  */
#define W_FRAME_LIMIT      0x800C0DBCu  /* frame number at which the stream ends (0: none)         */
#define W_RING_SIZE        0x800C20C4u  /* records in the ring                                     */
#define W_FRAME_FIRST      0x800BE9E4u  /* ring index of the first record of the frame             */
#define W_USER_CALLBACK    0x800B0CCCu  /* callback word of the stream owner                       */
#define W_PAYLOAD          0x800C0DC4u  /* destination of the payload of the record just taken     */

#define CHCR_BUSY          0x01000000u
#define STUCK_POLL_BUDGET  0x40000u     /* DMA3 idle polls before the host gives up                 */

/* Wait for the DMA3 channel to go idle (hardware-wait contract: ShouldStop exit, per-poll clock tick + DMA3 service, stuck-wait budget).
 * A channel that never goes idle ends in the named boundary the suite looks for (diagnostic line, order-log entry, unresolved-boundary stop). */
static int dma3_wait_idle(void)
{
    pe_addr_t chcr = PE_LoadU32(W_PTR_DMA3_CHCR);
    uint32_t polls = 0;
    while (PE_CdLoadU32(chcr) & CHCR_BUSY) {
        if (PE_Port_ShouldStop()) return 0;
        HostFB_StreamTick();
        PE_CdReg_ServiceDMA3();
        if (++polls >= STUCK_POLL_BUDGET) {
            PE_GuestHook_Refuse("func_8007C564_dma3_wait", "pe_plat_cdhost_stream", chcr, 0, polls, 0, 0, 0,
                                "DMA3 channel control stayed busy: stuck wait");
            return 0;
        }
    }
    return 1;
}

/* Index of the sector inside the source buffer: advanced after every sector taken (only with a source pointer). */
static void advance_sector(void)
{
    if (PE_LoadU32(W_CALLBACK) != 0u) PE_StoreU32(W_SECTOR, PE_LoadU32(W_SECTOR) + 1u);
}

/* Record rejected as not a stream sector (bad magic / channel): sector index reset, state 5, record freed. */
static void reject_sector(pe_addr_t slot)
{
    if (PE_LoadU32(W_CALLBACK) != 0u) PE_StoreU32(W_SECTOR, 0u);
    PE_StoreU32(W_STATE, 5u);
    PE_StoreU16(slot, 0u);
}

/* Drop the frame being assembled: counters cleared, the records of the frame (from its first record up to the current one)
 * freed, the record index back at the frame start. */
static void abort_frame(pe_addr_t slot)
{
    uint32_t i, first, cur;
    PE_StoreU32(W_FRAME, 0u);
    PE_StoreU16(W_PART, 0u);
    first = PE_LoadU32(W_FRAME_FIRST);
    cur = PE_LoadU32(W_SLOT_INDEX);
    for (i = first; (int32_t)i < (int32_t)cur; i++) PE_StoreU32(PE_LoadU32(W_SLOT_BASE) + (i << 5), 0u);   /* signed order: an index below the current one frees the records in between; otherwise nothing (deep v395 / v422 show no such stores, v608 / v829 with first = -1 do) */
    PE_StoreU32(W_SLOT_INDEX, PE_LoadU32(W_FRAME_FIRST));
    PE_StoreU16(slot, 0u);
}

/* The CD data callback word 0x800B0CCC (set by the stream owner): called with one argument when present. */
static void stream_callback(const char *what, uint32_t argument)
{
    uint32_t target = PE_LoadU32(W_USER_CALLBACK);
    int ignored = 0;
    if (target != 0u)
        (void)PE_GuestHook_Call(what, "pe_plat_cdhost_stream", target, 1, argument, 0, 0, 0, &ignored);
}

/* Guest-visible frame of the handler.  Retail keeps it on its stack: the result bytes of the CD-ready call at sp+0x10 and the twelve hand-read
 * FIFO bytes at sp+8 (the vectors show them at 0x801FEFF0 / 0x801FEFE8 in the oracle's stack).  The native handler models that frame at a
 * fixed scratch window, STREAM_FRAME, the stack position of the retail chain (the same window convention as the other host buffers at
 * 0x801FFE90): a guest source buffer in the stack address range can then overlap it as it does in retail.  The window is mapped as the
 * handler's own frame by the checkers (-DCDX47_SCRATCH_FRAME / -DSD_SCRATCH_FRAME, offsets compared). */
#define STREAM_FRAME 0x801FFE80u
#define STREAM_RESULT(frame) ((frame) + 0x10u)
#define STREAM_FIFO(frame)   ((frame) + 8u)
static pe_addr_t stream_frame_enter(void) { return STREAM_FRAME; }
static void stream_frame_leave(pe_addr_t frame) { (void)frame; }

void pe_plat_cdhost_stream(void)
{
    pe_addr_t frame, slot, bus, delay, index_reg, req_reg, fifo;
    uint32_t i, w0 = 0, w1 = 0, callback;
    uint32_t h0, h1, part, parts, number, frame_now, idx, ring, limit, payload, source;
    int ok, final = 0;

    if (PE_LoadU32(W_STREAM_BUSY) == 1u) return;

    /* MDEC-out DMA still running: defer. */
    if (PE_LoadU32(W_MDEC_GATE) != 0u &&
        (PE_CdLoadU32(PE_LoadU32(W_PTR_MDEC_CHCR)) & CHCR_BUSY)) {
        PE_StoreU32(W_DEFER_FLAG, 1u);
        advance_sector();
        PE_StoreU32(W_STATE, 1u);
        return;
    }

    frame = stream_frame_enter();
    if (pe_plat_cd_ready(1, STREAM_RESULT(frame)) == 5) {        /* ready status 5 (disk error event): nothing is taken, no state change */
        stream_frame_leave(frame);
        return;
    }
    {   /* the two result bytes the call leaves in the frame: an error status stops the handler in state 3 */
        uint32_t status = PE_LoadU8(STREAM_RESULT(frame));
        if (status & 0x1Du) {
            PE_StoreU32(W_STATE, 3u);
            stream_frame_leave(frame);
            return;
        }
    }
    slot = PE_LoadU32(W_SLOT_BASE) + (PE_LoadU32(W_SLOT_INDEX) << 5);
    PE_StoreU32(W_SLOT_CURRENT, slot);

    if (PE_LoadU16(slot) != 0u) {                 /* slot still occupied */
        advance_sector();
        PE_StoreU32(W_STATE, 4u);
        stream_frame_leave(frame);
        return;
    }

    /* Ask the controller for data and set the CD bus timing for the transfer. */
    index_reg = PE_LoadU32(W_PTR_CD_INDEX);
    req_reg = PE_LoadU32(W_PTR_CD_REQUEST);
    PE_CdStoreU8(index_reg, 0u);
    PE_CdStoreU8(req_reg, 0u);
    index_reg = PE_LoadU32(W_PTR_CD_INDEX);
    req_reg = PE_LoadU32(W_PTR_CD_REQUEST);
    PE_CdStoreU8(index_reg, 0u);
    PE_CdStoreU8(req_reg, 0x80u);
    bus = PE_LoadU32(W_PTR_BUS_CONTROL);
    delay = PE_LoadU32(W_PTR_CD_DELAY);
    PE_CdStoreU32(bus, 0x20943u);
    PE_CdStoreU32(delay, 0x1323u);

    if (PE_LoadU32(W_FIFO_MODE) == 0u) {
        /* hand-read FIFO bytes: twelve reads (five through a freshly loaded pointer, seven more through the last
         * one); see the header for which four form the word stored at slot+0x1C */
        /* the twelve bytes land in the frame buffer (frame+8..+0x13 = STREAM_FIFO), and the word is read back from it */
        for (i = 0; i < 5u; i++) PE_StoreU8(STREAM_FIFO(frame) + i, PE_CdLoadU8(PE_LoadU32(W_PTR_CD_DATA)));
        fifo = PE_LoadU32(W_PTR_CD_DATA);
        for (i = 5; i < 12u; i++) PE_StoreU8(STREAM_FIFO(frame) + i, PE_CdLoadU8(fifo));
    }
    /* the word stored twice at slot+0x1C is the frame word at +8, loaded twice: the first four FIFO bytes when they were just read, otherwise whatever the
     * window still holds from the previous call (retail: the stale stack word; the vector fixtures seed it, v2 1558 / 1558 cases) */
    w0 = PE_LoadU32(STREAM_FIFO(frame));
    w1 = PE_LoadU32(STREAM_FIFO(frame));

    callback = PE_LoadU32(W_CALLBACK);
    if (callback == 0u) {
        /* no source buffer: let DMA3 fetch the 8-word record into the slot */
        pe_plat_cdhost_dma(3u, slot, 0u, 8u, 0x11000000u, 0u, 0u);
    } else {
        source = callback + (PE_LoadU32(W_SECTOR) << 11);
        for (i = 0; i < 8u; i++) PE_StoreU32(slot + 4u * i, PE_LoadU32(source + 4u * i));
    }

    ok = dma3_wait_idle();
    PE_StoreU32(slot + 0x1Cu, w0);
    PE_StoreU32(slot + 0x1Cu, w1);
    bus = PE_LoadU32(W_PTR_BUS_CONTROL);
    delay = PE_LoadU32(W_PTR_CD_DELAY);
    PE_CdStoreU32(bus, 0x20843u);
    PE_CdStoreU32(delay, 0x1325u);
    if (!ok) { stream_frame_leave(frame); return; }

    /* ---- the record is in the slot: validate it and assemble the frame ---- */
    if (PE_LoadU32(W_START_WAIT) == 1u && PE_LoadU32(W_START_NUMBER) != 0u) {
        /* waiting for the start of a frame: the record must carry the wanted frame number */
        if (PE_LoadU32(W_START_NUMBER) != PE_LoadU16(slot + 8u)) {
            PE_StoreU16(slot, 0u);
            advance_sector();
            goto done;
        }
        PE_StoreU32(W_START_WAIT, 0u);
    }
    h0 = PE_LoadU16(slot);
    if (h0 != 0x160u) { reject_sector(slot); goto done; }
    h1 = PE_LoadU16(slot + 2u);
    if (((h1 >> 10) & 0x1Fu) != PE_LoadU32(W_CHANNEL))   /* five channel bits: the physical fixtures' video header has bit 15 set (h1 = 0x8001); no vector sets it */ { reject_sector(slot); goto done; }

    part = PE_LoadU16(slot + 4u);
    parts = PE_LoadU16(slot + 6u);
    number = PE_LoadU16(slot + 8u);
    if ((uint32_t)(int32_t)(int16_t)PE_LoadU16(W_PART) != part) {
        abort_frame(slot); advance_sector(); PE_StoreU32(W_STATE, 6u); goto done;
    }
    idx = PE_LoadU32(W_SLOT_INDEX);
    frame_now = PE_LoadU32(W_FRAME);
    if (frame_now == 0u && part == 0u) {
        /* first record of a frame (no frame in progress and the record is its part 0) */
        PE_StoreU16(W_PART, 0u);
        PE_StoreU32(W_FRAME, number);
        limit = PE_LoadU32(W_FRAME_LIMIT);
        if (limit != 0u && limit <= number) {                   /* frame number reached the limit: end of stream */
            abort_frame(slot);
            PE_StoreU32(W_START_WAIT, 1u);
            stream_callback("CD stream end-of-frames callback (0x800B0CCC)", PE_LoadU32(W_FRAME_FIRST));
            advance_sector(); PE_StoreU32(W_STATE, 7u); goto done;
        }
        ring = PE_LoadU32(W_RING_SIZE);
        if ((uint32_t)(ring - idx - 1u) < parts) {               /* the frame does not fit in the records left before the end of the ring */
            if (PE_LoadU32(W_FRAME_LIMIT) == 0u) {
                PE_StoreU16(slot, 1u);
                PE_StoreU32(W_START_WAIT, 1u);
                stream_callback("CD stream ring-wrap callback (0x800B0CCC)", slot);
                advance_sector(); PE_StoreU32(W_STATE, 8u); goto done;
            }
            if (PE_LoadU16(PE_LoadU32(W_SLOT_BASE)) != 0u) {     /* the first record is still occupied */
                PE_StoreU16(slot, 0u);
                advance_sector(); PE_StoreU32(W_STATE, 9u); goto done;
            }
            /* wrap: mark the old record, move the frame to the first record */
            PE_StoreU16(slot, 1u);
            PE_StoreU32(W_SLOT_INDEX, 0u);
            {
                pe_addr_t first = PE_LoadU32(W_SLOT_BASE);
                for (i = 0; i < 8u; i++) PE_StoreU32(first + 4u * i, PE_LoadU32(slot + 4u * i));
                slot = first;
            }
            idx = 0;
            PE_StoreU32(W_SLOT_CURRENT, slot);
        }
        PE_StoreU32(W_FRAME_FIRST, idx);
    } else if (frame_now != 0u && frame_now != number) {
        abort_frame(slot); advance_sector(); PE_StoreU32(W_STATE, 6u); goto done;
    }

    /* the record belongs to the frame: take the payload */
    PE_StoreU32(W_STATE, 10u);
    PE_StoreU16(W_PART, (uint16_t)(PE_LoadU16(W_PART) + 1u));
    payload = PE_LoadU32(W_SLOT_BASE) + (PE_LoadU32(W_RING_SIZE) << 5) + idx * 0x7E0u;
    PE_StoreU32(W_PAYLOAD, payload);
    if (PE_LoadU32(W_MDEC_GATE) != 0u) {
        PE_CdStoreU32(PE_LoadU32(W_PTR_BUS_CONTROL), 0x20943u);
        PE_CdStoreU32(PE_LoadU32(W_PTR_CD_DELAY), 0x1323u);
    } else {
        PE_CdStoreU32(PE_LoadU32(W_PTR_BUS_CONTROL), 0x21020843u);
    }
    if ((int32_t)(int16_t)PE_LoadU16(W_PART) == (int32_t)parts) final = 1;
    if (final) PE_StoreU32(W_STREAM_BUSY, 1u);
    if (PE_LoadU32(W_CALLBACK) != 0u) {
        source = PE_LoadU32(W_CALLBACK) + (PE_LoadU32(W_SECTOR) << 11) + 0x20u;
        for (i = 0; i < 504u; i++) PE_StoreU32(payload + 4u * i, PE_LoadU32(source + 4u * i));
    } else {
        pe_plat_cdhost_dma(3u, payload, 0u, 504u, PE_LoadU32(W_MDEC_GATE) == 0u ? 0x11400100u : 0x11000000u, final ? 1u : 0u, 0u);   /* control word 0x11400100 when the MDEC gate word is 0, else 0x11000000 (v2: 46 / 46 and 100 / 100 payload DMAs) */   /* completion interrupt only for the last part of the frame (0x800B89F4 stored as 1 just before; v2 146 / 146) */
        /* no wait here: the payload transfer completes under the DMA3 completion interrupt (the vectors read the channel register only for the record fetch) */
    }
    advance_sector();
    if (final) {
        PE_StoreU16(W_PART, 0u);
        PE_StoreU32(W_FRAME, 0u);
        PE_StoreU32(W_CHANNEL, PE_LoadU32(W_CHANNEL_NEXT));
    }
    PE_CdStoreU32(PE_LoadU32(W_PTR_CD_DELAY), 0x1325u);
    PE_StoreU16(slot, 3u);
    PE_StoreU32(W_SLOT_INDEX, PE_LoadU32(W_SLOT_INDEX) + 1u);
    /* a source buffer delivers the data itself, so the data-ready callback runs here; with DMA it is left to the DMA3 completion (v2 146 / 146 payload-DMA cases) */
    if (PE_LoadU32(W_CALLBACK) != 0u && PE_LoadU32(W_STREAM_BUSY) != 0u) func_8007C214();

done:
    stream_frame_leave(frame);
}
