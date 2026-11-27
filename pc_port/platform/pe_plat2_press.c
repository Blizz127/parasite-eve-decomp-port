/* libpress host bindings (guest 0x8010C0D8 DecDCToutCallback, 0x8010BD4C VLC table
 * expander), written clean-room from the black-box vector sets gpu2d (docs/evidence/native-gpu2-20261126), the port's
 * host GPU/DMA model API and psx-spx; no SDK source, decompiled source, tables
 * or disassembly were read.
 *
 * DecDCToutCallback (guest 0x8010C0D8): the vectors (300 cases) show exactly one
 * observable action and no guest RAM access: a call of the libetc DMA
 * callback registration (guest 0x80073CF4, pe_plat_dma_callback) with channel
 * 1 (the MDEC-out DMA channel, psx-spx "DMA Channels") and the caller's
 * handler word, whose result is returned unchanged.  The other argument
 * registers do not matter.  A zero handler is passed through like any other
 * word (vector cases 202211333 and 202211334 call with 0 and 0x80000000). */
#include "pe_plat/press.h"
#include "pe_plat/irq_callback.h"

#define PRESS_DMA_CHANNEL_MDEC_OUT 1u

uint32_t pe_plat_press_out_callback(uint32_t callback)
{
    return pe_plat_dma_callback(PRESS_DMA_CHANNEL_MDEC_OUT, callback);
}

/* ------------------------------------------------------------------------
 * VLC table expander (guest 0x8010BD4C): pe_plat_press_expand_table(dst, count)
 *
 * Derived from the 104 expander vectors plus targeted single-table probes of
 * the original executed as a black box (gpu2d-probe: ordered table-byte read
 * and RAM-store traces of crafted tables; the probes and their hashes stay in
 * scratch).  The second argument is ignored and the result is always 1 on
 * every defined case.  The compact table at 0x8010CBFC (read byte by byte, in
 * order, each byte once) is expanded in two passes over the 0x11000-byte
 * destination, whose position does not matter (odd and KSEG1 destinations
 * behave like any other):
 *
 * Pass 1 decodes a byte stream of tokens into the destination, sequentially
 * from its first byte (a byte-oriented LZ77 with a literal run):
 *   b < 0xF0        literal run: the next b+1 table bytes are stored.
 *   0xF0            literal run with an explicit count: if the next byte c is
 *                   below 0xF0 it is consumed and c+1 table bytes are stored,
 *                   otherwise nothing is stored (that byte is the next token).
 *   0xF1..0xFF  a  copy: the distance is (op-0xF1)*256 + a + 1 bytes back from
 *                   the current output position; the length is the sum of
 *                   (e+1) over the run of consecutive following bytes e that
 *                   are below 0xF0 (zero when the next byte is 0xF0 or more);
 *                   the copy is forward and byte-wise, so a distance shorter
 *                   than the length repeats the pattern.
 *   0xFF 0xFF       end of stream (after these two bytes pass 1 is over).
 * Pass 2 turns the stored differences back into values: for every halfword k
 * from 4 to 0x887F of the destination, h[k] ^= h[k-4] (little-endian, exactly
 * the bytes 8..0x10FFF are rewritten), so the stream is a chain of XOR deltas
 * between consecutive 8-byte records.  Bytes the stream did not reach are
 * taken as they stand in guest RAM.
 *
 * Undefined edges (no vector defines them): a destination window that is not wholly
 * inside main RAM, a stream that never ends, reads
 * the table range beyond 0x8011ECF8, writes beyond dst+0x11000 or copies from
 * before dst.  The first four request a stop and return 0; a copy before dst
 * reads guest RAM there (as the original would).
 * ------------------------------------------------------------------------ */
#include "game_port.h"
#include "pe_guest_ram.h"

#define PRESS_TABLE_BASE  0x8010CBFCu
#define PRESS_TABLE_SIZE  0x120FCu
#define PRESS_OUT_SIZE    0x11000u

typedef struct {
    uint32_t cur;      /* next table offset */
    int have;          /* a byte has been read ahead */
    uint8_t ahead;
    int overrun;
} PressTable;

static int press_peek(PressTable *t, uint8_t *b)
{
    if (!t->have) {
        if (t->cur >= PRESS_TABLE_SIZE) { t->overrun = 1; return 0; }
        t->ahead = PE_LoadU8(PRESS_TABLE_BASE + t->cur);
        t->cur++;
        t->have = 1;
    }
    *b = t->ahead;
    return 1;
}

static int press_take(PressTable *t, uint8_t *b)
{
    if (!press_peek(t, b)) return 0;
    t->have = 0;
    return 1;
}

/* The destination window must lie wholly inside the 2 MiB of main RAM (KUSEG, KSEG0 or KSEG1 alias); a window that does
 * not is a guest fault on the original, so it is a stop here instead of an abort in the accessors. */
static int press_window_ok(uint32_t dst)
{
    uint32_t seg = dst & 0xE0000000u, off = dst & 0x1FFFFFFFu;
    return (seg == 0x00000000u || seg == 0x80000000u || seg == 0xA0000000u) &&
           off < PE_RAM_SIZE && PRESS_OUT_SIZE <= PE_RAM_SIZE - off;
}

uint32_t pe_plat_press_expand_table(uint32_t dst, uint32_t count)
{
    PressTable t = {0, 0, 0, 0};
    uint32_t pos = 0, k;
    uint16_t ring[4] = {0, 0, 0, 0};
    (void)count;
    if (!press_window_ok(dst)) return (PE_Port_RequestStop(PE_PORT_STOP_GUEST_TRAP), 0u);

    for (;;) {
        uint8_t op, a, e;
        uint32_t n;
        if (!press_take(&t, &op)) return (PE_Port_RequestStop(PE_PORT_STOP_GUEST_TRAP), 0u);
        if (op <= 0xF0u) {
            if (op == 0xF0u) {
                if (press_peek(&t, &e) && e < 0xF0u) { t.have = 0; n = (uint32_t)e + 1u; }
                else if (t.overrun) return (PE_Port_RequestStop(PE_PORT_STOP_GUEST_TRAP), 0u);
                else n = 0;
            } else {
                n = (uint32_t)op + 1u;
            }
            if (pos + n > PRESS_OUT_SIZE) return (PE_Port_RequestStop(PE_PORT_STOP_GUEST_TRAP), 0u);
            while (n--) {
                if (!press_take(&t, &e)) return (PE_Port_RequestStop(PE_PORT_STOP_GUEST_TRAP), 0u);
                PE_StoreU8(dst + pos++, e);
            }
        } else {
            uint32_t dist;
            if (!press_take(&t, &a)) return (PE_Port_RequestStop(PE_PORT_STOP_GUEST_TRAP), 0u);
            if (op == 0xFFu && a == 0xFFu) break;
            n = 0;
            while (press_peek(&t, &e) && e < 0xF0u) { t.have = 0; n += (uint32_t)e + 1u; }
            if (t.overrun) return (PE_Port_RequestStop(PE_PORT_STOP_GUEST_TRAP), 0u);
            dist = (uint32_t)(op - 0xF1u) * 256u + a + 1u;
            if (pos + n > PRESS_OUT_SIZE) return (PE_Port_RequestStop(PE_PORT_STOP_GUEST_TRAP), 0u);
            while (n--) {
                PE_StoreU8(dst + pos, PE_LoadU8(dst + pos - dist));
                pos++;
            }
        }
    }

    for (k = 0; k < PRESS_OUT_SIZE / 2u; k++) {
        uint16_t v = PE_LoadU16(dst + 2u * k);
        if (k >= 4u) {
            v ^= ring[k & 3u];
            PE_StoreU16(dst + 2u * k, v);
        }
        ring[k & 3u] = v;
    }
    return 1u;
}
