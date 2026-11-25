/*
 * pe_vlc_decode.h - PlayStation "BS" (STR video frame) bitstream -> MDEC
 * run-length word stream.
 *
 * Provenance: written clean-room from the psx-spx documentation and black-box
 * vectors of the original code, with no SDK or third-party decoder source.
 *   psx-spx "Macroblock Decoder (MDEC)": MDEC Decompression / MDEC Data Format
 *   psx-spx "CDROM File Formats": CDROM File Video BS Compression
 *     (Versions, Headers, DC Values, AC Values) and "Raw MDEC".
 *
 * Input  : one BS frame = the concatenated STR sector payloads of the frame
 *          (the 0x20-byte STR sector headers already stripped; trailing zero
 *          padding after the end-of-frame code is accepted and ignored).
 *            +0 u16 MDEC size/4 (informational; not used by the plain decoder)
 *            +2 u16 file id (ignored by the decoder; vectors: no error)
 *            +4 u16 quant, masked to 6 bits -> MDEC DCT word bits 15-10
 *            +6 u16 version: 0,1,2 decode as v2 (10-bit DC); >= 3 as v3
 *            +8 ... Huffman bitstream, MSB-first within each little-endian
 *                   16-bit halfword (first bit = bit 15 of the first halfword)
 *          Block order in the bitstream: Cr, Cb, Y1, Y2, Y3, Y4 per macroblock.
 *
 * Output : 16-bit MDEC(1) parameter halfwords in the order the MDEC consumes
 *          them: per block { DC word, AC run/level words..., 0xFE00 (EOB) }.
 *          DC word = (quant << 10) | (dc & 0x3FF); AC word = (run << 10) |
 *          (level & 0x3FF).
 *
 * Two entry points (the psx-spx text does not say which layout a given
 * vendor routine emits; both are offered, see REPORT.md):
 *   pe_vlc_decode_frame()       DATA ONLY: DC/AC/EOB words, no MDEC command
 *                               header, no FE00h padding.  (default)
 *   pe_vlc_decode_frame_mdec()  "Raw MDEC" layout of psx-spx: 2 header
 *                               halfwords (size/4 field copied from BS header
 *                               bytes 0-1, then the id word 0x3800 copied from
 *                               bytes 2-3 -- i.e. the low 32 bits of an
 *                               MDEC(1) command, bits 15-0 = number of
 *                               parameter WORDS), then the data, then 0xFE00
 *                               padding up to 2 + size_field*2 halfwords in
 *                               total.  Longer streams decode in full (the
 *                               count is not a bound).  This is the layout the
 *                               original libpress decoder was observed to
 *                               produce (vectors, see REPORT.md).
 *
 * End of frame: a DC value 0x1FF (v1/v2) / code 1111111111 (v3) at ANY block
 * start.  Only the "10" code ends a block; a raw FE00 escape is plain data.
 *
 * Return 0 on success, a negative PE_VLC_ERR_* code otherwise.  *rl_len (if
 * non-NULL) is always set to the number of halfwords written to rl_out.
 * Nothing is ever read beyond frame_bytes or written beyond rl_cap.
 */
#ifndef PE_VLC_DECODE_H
#define PE_VLC_DECODE_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PE_VLC_OK            0
#define PE_VLC_ERR_ARG      (-1) /* NULL frame / NULL rl_out with rl_cap > 0 */
#define PE_VLC_ERR_HEADER   (-2) /* fewer than 8 bytes                       */
#define PE_VLC_ERR_VERSION  (-3) /* unused since the vector alignment        */
#define PE_VLC_ERR_QUANT    (-4) /* unused since the vector alignment        */
#define PE_VLC_ERR_TRUNC    (-5) /* bitstream ended inside a code/macroblock,
                                    or no end-of-frame code before the end   */
#define PE_VLC_ERR_DESYNC   (-6) /* unused (never-assigned) Huffman code     */
#define PE_VLC_ERR_OVERFLOW (-7) /* rl_cap too small                         */
#define PE_VLC_ERR_SIZE     (-8) /* unused: header count is not a bound      */
#define PE_VLC_ERR_STATE    (-9) /* pe_vlc_decode_step() with nothing to resume */

int pe_vlc_decode_frame(const uint8_t *frame, size_t frame_bytes,
                        uint16_t *rl_out, size_t rl_cap, size_t *rl_len);

int pe_vlc_decode_frame_mdec(const uint8_t *frame, size_t frame_bytes,
                             uint16_t *rl_out, size_t rl_cap, size_t *rl_len);

/* As pe_vlc_decode_frame_mdec(); also reports the number of bitstream bits
 * (after the 8-byte header) consumed up to and including the end-of-frame
 * code (0 on error).  The original's input cursor, as observed on all vectors,
 * is frame + 8 + 2*ceil((stream_end_bits + 7) / 16) bytes.
 *
 * Streams the original never terminates on (no end-of-frame code, truncated
 * data, unused codes, a v3 stream under version 0..2 or a v2 stream under
 * version >= 3 that happens to desync) get a negative return here when the
 * decoder detects them; a mis-versioned stream that still parses as valid
 * code words can return 0 with garbage data (not detectable here). */
int pe_vlc_decode_frame_mdec_ex(const uint8_t *frame, size_t frame_bytes,
                                uint16_t *rl_out, size_t rl_cap, size_t *rl_len,
                                size_t *stream_end_bits);


/* ------------------------------------------------------------------ */
/* Resumable / bounded decoding (vector-derived; REPORT.md "resumable API"). */
/* ------------------------------------------------------------------ */
/* The original decoder takes a bound B (output halfwords) that applies to the
 * CURRENT call.  After it writes a block's DC word it exits (returns 1) when
 * the number of halfwords this call has advanced its output pointer by exceeds
 * B: that count includes the DC word and, in the first call only, the two MDEC
 * header halfwords (so the first call exits when data halfwords + 1 >= B, a
 * resume call when data halfwords > B).  The exit lies between the DC word and
 * the block's AC words/EOB, so a bounded call can end right after a DC word.
 * B = 0, 1, 2 exit after the first DC; a B at least the total output never
 * bounds (0x00FFFFFF, the shipped default, never does).  A resume call
 * continues from the saved input bit position, block position and DC
 * predictors; its bound counts afresh.
 *
 * The state is plain data (copyable).  Usage:
 *     pe_vlc_decoder_t st;
 *     pe_vlc_decode_begin(&st, frame, n);
 *     do { rc = pe_vlc_decode_step(&st, buf, cap, B, &hw, &bits); ... } while (rc == 1);
 * rc 0: frame complete (the call also appended the FE00 padding up to
 * 2 + 2*count halfwords in total, as pe_vlc_decode_frame_mdec() does);
 * rc 1: bounded exit, call again to resume; rc < 0: error, the decoder is
 * dead (further steps return PE_VLC_ERR_STATE, like the original that never
 * returns from a resume without a preceding bounded exit).
 *
 * rl_out receives only this call's DATA halfwords (DC/AC/EOB/pad); the two
 * MDEC header halfwords, the end word and the guest-visible registers belong
 * to the adapter (the original's output pointer advances by 4 for the header
 * in the first call, then by 2 per halfword).  *halfwords_written = this
 * call's count.  *end_bits = input bit position (after the 8-byte header)
 * reached by this call, defined as (position of the next code to be read) +
 * 10: that is, after the DC word at a bounded exit, after the end-of-frame
 * code on completion (0 on error).  The original's input pointer is
 * frame + 8 + 2*ceil((*end_bits + 7)/16), i.e. its window always holds
 * 17 bits from the next code (vectors).
 * rl_cap is the capacity of this call's rl_out. */
typedef struct pe_vlc_decoder {
    const uint8_t *frame;
    size_t frame_bytes;
    size_t total_bits;    /* valid bits after the 8-byte header          */
    size_t bit_pos;       /* current bit position (saved across steps)    */
    size_t out_halfwords; /* data halfwords written by all steps so far   */
    int old_cr, old_cb, old_y; /* v3 DC predictors                        */
    unsigned version;     /* 2 (versions 0..2) or 3 (>= 3)                */
    unsigned quant;
    unsigned blk;         /* block in the macroblock: 0 Cr, 1 Cb, 2..5 Y   */
    unsigned in_ac;       /* 1 after the DC word of block blk was written  */
    unsigned calls;       /* steps made so far (the first one counts the header) */
    int state;            /* PE_VLC_ST_*                                  */
} pe_vlc_decoder_t;

#define PE_VLC_ST_FAILED 0 /* also the zero state: nothing to resume */
#define PE_VLC_ST_RUN    1 /* begun, no step yet                     */
#define PE_VLC_ST_PAUSED 2 /* last step returned 1                   */
#define PE_VLC_ST_DONE   3 /* last step returned 0                   */

/* Starts a frame (the original's call with a0 = frame pointer).  Returns 0,
 * PE_VLC_ERR_ARG or PE_VLC_ERR_HEADER (st is then dead). */
int pe_vlc_decode_begin(pe_vlc_decoder_t *st, const uint8_t *frame, size_t frame_bytes);

/* One call of the original (first call after begin, or a resume call).
 * Returns 0 complete, 1 bounded exit, negative error. */
int pe_vlc_decode_step(pe_vlc_decoder_t *st, uint16_t *rl_out, size_t rl_cap,
                       uint32_t bound_halfwords, size_t *halfwords_written, size_t *end_bits);

#ifdef __cplusplus
}
#endif

#endif /* PE_VLC_DECODE_H */
