/* Shared checker core: a pure BS-frame VLC decoder (pe_vlc_decode_frame_mdec)
 * against retail_mdec_vlc_cases.h (outputs of the ORIGINAL libpress VLC
 * decoder, obtained black-box).  Framework-free: used by the standalone
 * pe_mdec_vlc_vectors_check.c and by the native suite (test_mdec_vlc.h).
 * Include pe_vlc_decode.h and retail_mdec_vlc_cases.h before this file.  Real frames are re-read from
 * the disc through the callback.
 *   exact   : real frames and every synthetic stream must equal the original's
 *             complete output buffer and fit its returned input cursor;
 *   known   : always 0 now (the earlier out-of-spec divergences were resolved);
 *   hang    : streams the original never terminates on must not return OK. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef int (*MdecVlcReadRaw)(void *ctx, uint32_t lba, uint8_t raw[2352]);

static uint64_t MdecVlcFnv(const uint8_t *p, size_t n)
{
    uint64_t h = UINT64_C(14695981039346656037);
    while (n--) { h ^= *p++; h *= UINT64_C(1099511628211); }
    return h;
}
/* decode through the pure API and return the little-endian byte image of the halfwords */
static size_t MdecVlcEndBits;
static int MdecVlcRun(const uint8_t *in, size_t n, uint8_t *bytes, size_t cap_bytes, size_t *out_bytes)
{
    static uint16_t hw[1u << 19];
    size_t len = 0, i;
    int rc;
    MdecVlcEndBits = 0;
    rc = pe_vlc_decode_frame_mdec_ex(in, n, hw, sizeof(hw) / sizeof(hw[0]), &len, &MdecVlcEndBits);
    *out_bytes = len * 2u;
    if (len * 2u > cap_bytes) return -100;
    for (i = 0; i < len; i++) { bytes[2u * i] = (uint8_t)hw[i]; bytes[2u * i + 1u] = (uint8_t)(hw[i] >> 8); }
    return rc;
}
#define MDECVLC_FAIL(...) do { snprintf(err, errsz, __VA_ARGS__); free(out); free(frame); return 0; } while (0)

/* Returns 1 on success; on failure fills `err`.  Counters: real, exact
 * synthetic, known-divergent synthetic, original-hang vectors. */
static int MdecVlcCheckAll(MdecVlcReadRaw rd, void *ctx, char *err, size_t errsz,
                           unsigned *nreal, unsigned *nsyn, unsigned *nknown, unsigned *nhang)
{
    const size_t CAP = 1u << 20;
    uint8_t *out = malloc(CAP), *frame = malloc(64u * 2016u);
    size_t outlen;
    *nreal = *nsyn = *nknown = *nhang = 0;
    if (!out || !frame) { snprintf(err, errsz, "allocation"); free(out); free(frame); return 0; }
    for (unsigned k = 0; k < sizeof(MDECVLC_real) / sizeof(MDECVLC_real[0]); k++) {
        size_t n = 0;
        for (uint32_t lba = MDECVLC_real[k].first; lba <= MDECVLC_real[k].last; lba++) {
            uint8_t raw[2352];
            if (!rd(ctx, lba, raw)) MDECVLC_FAIL("real %u: sector %u unreadable", k, (unsigned)lba);
            if (!(raw[18] & 8u) || raw[24] != 0x60 || raw[25] != 1 || raw[26] != 1 || raw[27] != 0x80) continue;
            if (n + 2016u > 64u * 2016u) MDECVLC_FAIL("real %u: frame too large", k);
            memcpy(frame + n, raw + 56, 2016u); n += 2016u;
        }
        if (n < MDECVLC_real[k].size) MDECVLC_FAIL("real %u: frame short", k);
        n = MDECVLC_real[k].size;
        if (MdecVlcFnv(frame, n) != MDECVLC_real[k].in_hash) MDECVLC_FAIL("real %u (frame %u): input hash differs", k, MDECVLC_real[k].frame);
        if (MdecVlcRun(frame, n, out, CAP, &outlen) != PE_VLC_OK)
            MDECVLC_FAIL("real %u (frame %u): decoder error", k, MDECVLC_real[k].frame);
        if (outlen != MDECVLC_real[k].outlen || MdecVlcFnv(out, outlen) != MDECVLC_real[k].out_hash)
            MDECVLC_FAIL("real %u (frame %u): output differs from the original", k, MDECVLC_real[k].frame);
        if (8u + 2u * ((MdecVlcEndBits + 7u + 15u) / 16u) != MDECVLC_real[k].cursor)
            MDECVLC_FAIL("real %u (frame %u): fitted input cursor differs from the original's", k, MDECVLC_real[k].frame);
        (*nreal)++;
    }
    for (unsigned k = 0; k < sizeof(MDECVLC_syn) / sizeof(MDECVLC_syn[0]); k++) {
        int rc = MdecVlcRun(MDECVLC_syn[k].in, MDECVLC_syn[k].insize, out, CAP, &outlen);
        int same = rc == PE_VLC_OK && outlen == MDECVLC_syn[k].outlen && MdecVlcFnv(out, outlen) == MDECVLC_syn[k].out_hash &&
                   8u + 2u * ((MdecVlcEndBits + 7u + 15u) / 16u) == MDECVLC_syn[k].cursor;
        if (same) { (*nsyn)++; continue; }
        MDECVLC_FAIL("syn %s: differs from the original (rc %d, len %u want %u)", MDECVLC_syn[k].name, rc, (unsigned)outlen, (unsigned)MDECVLC_syn[k].outlen);
    }
    for (unsigned k = 0; k < sizeof(MDECVLC_hang) / sizeof(MDECVLC_hang[0]); k++) {
        if (MdecVlcRun(MDECVLC_hang[k].in, MDECVLC_hang[k].insize, out, CAP, &outlen) == PE_VLC_OK)
            MDECVLC_FAIL("hang %s: claimed success where the original never terminates", MDECVLC_hang[k].name);
        (*nhang)++;
    }
    free(out); free(frame);
    return 1;
}
