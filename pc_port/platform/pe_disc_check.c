/*
 * First-run disc check — see pe_disc_check.h.
 */
#include "pe_disc_check.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* USA Disc 1 and Disc 2 boot EXEs are byte-identical (and so is PE.IMG);
 * the discs differ in boot file name and FMV tracks. */
const PE_KnownDisc g_pe_known_discs[] = {
    { "USA", 1, "SLUS-00662", "\\SLUS_006.62;1", "452fb033f2eaa4b18aa20a5bca60b8125af3a37b",
      "c339455d5b1dae04f77c2ee847d0932adaf2e84b" },
    { "USA", 2, "SLUS-00668", "\\SLUS_006.68;1", "452fb033f2eaa4b18aa20a5bca60b8125af3a37b",
      "6dc5b537527aa0d54bdbbd0c14b458083b0743e4" },
    { "JPN", 1, "SLPS-01230", "\\SLPS_012.30;1", NULL, NULL },
    { "JPN", 2, "SLPS-01231", "\\SLPS_012.31;1", NULL, NULL },
};
const unsigned g_pe_known_disc_count =
    (unsigned)(sizeof(g_pe_known_discs) / sizeof(g_pe_known_discs[0]));

/* ── SHA-1 (FIPS 180-4) over a host buffer ─────────────────────────── */
static uint32_t Rol(uint32_t v, int n) { return (v << n) | (v >> (32 - n)); }

static void Sha1Block(uint32_t h[5], const uint8_t b[64])
{
    uint32_t w[80], a, bb, c, d, e, f, k, t;
    int i;
    for (i = 0; i < 16; i++)
        w[i] = (uint32_t)b[4*i] << 24 | (uint32_t)b[4*i+1] << 16 |
               (uint32_t)b[4*i+2] << 8 | b[4*i+3];
    for (i = 16; i < 80; i++)
        w[i] = Rol(w[i-3] ^ w[i-8] ^ w[i-14] ^ w[i-16], 1);
    a = h[0]; bb = h[1]; c = h[2]; d = h[3]; e = h[4];
    for (i = 0; i < 80; i++) {
        if (i < 20)      { f = (bb & c) | (~bb & d);          k = 0x5A827999u; }
        else if (i < 40) { f = bb ^ c ^ d;                    k = 0x6ED9EBA1u; }
        else if (i < 60) { f = (bb & c) | (bb & d) | (c & d); k = 0x8F1BBCDCu; }
        else             { f = bb ^ c ^ d;                    k = 0xCA62C1D6u; }
        t = Rol(a, 5) + f + e + k + w[i];
        e = d; d = c; c = Rol(bb, 30); bb = a; a = t;
    }
    h[0] += a; h[1] += bb; h[2] += c; h[3] += d; h[4] += e;
}

void PE_DiscCheck_Sha1Hex(const void *data, size_t size, char out[41])
{
    uint32_t h[5] = {0x67452301u, 0xEFCDAB89u, 0x98BADCFEu, 0x10325476u, 0xC3D2E1F0u};
    const uint8_t *p = (const uint8_t *)data;
    uint8_t blk[64];
    uint64_t bits = (uint64_t)size * 8u;
    size_t i = 0, n = 0;
    int j;
    for (; i + 64 <= size; i += 64)
        Sha1Block(h, p + i);
    for (; i < size; i++)
        blk[n++] = p[i];
    blk[n++] = 0x80;
    if (n > 56) { while (n < 64) blk[n++] = 0; Sha1Block(h, blk); n = 0; }
    while (n < 56) blk[n++] = 0;
    for (j = 7; j >= 0; j--) blk[n++] = (uint8_t)(bits >> (8 * j));
    Sha1Block(h, blk);
    for (j = 0; j < 5; j++)
        snprintf(out + 8 * j, 9, "%08x", h[j]);
}

int PE_DiscCheck_ReadFile(const PE_Disc *d, const char *iso_path,
                          uint8_t **data, uint32_t *size)
{
    uint32_t lba, len;
    uint8_t *buf;
    *data = NULL;
    *size = 0;
    if (!d || !PE_Disc_FindFile(d, iso_path, &lba, &len, NULL, 0) || len == 0 ||
        len > 16u * 1024u * 1024u)
        return -1;
    buf = (uint8_t *)malloc(len);
    if (!buf)
        return -1;
    if (!PE_Disc_ReadUserData(d, lba, 0, buf, len)) {
        free(buf);
        return -1;
    }
    *data = buf;
    *size = len;
    return 0;
}

static void AcceptedList(unsigned mask, char *out, size_t n)
{
    unsigned i;
    size_t used = 0;
    out[0] = '\0';
    for (i = 0; i < g_pe_known_disc_count; i++) {
        const PE_KnownDisc *k = &g_pe_known_discs[i];
        if (!k->exe_sha1 || !(mask & (1u << (k->disc - 1))))
            continue;
        used += (size_t)snprintf(out + used, used < n ? n - used : 0, "%s%s Disc %d (%s)",
                                 used ? ", " : "", k->region, k->disc, k->serial);
        if (used >= n)
            break;
    }
}

int PE_DiscCheck_Identify(const PE_Disc *d, unsigned accept_mask,
                          PE_DiscIdentity *out, char *err, size_t err_size)
{
    unsigned i;
    char accepted[160];
    PE_DiscIdentity id;

    memset(&id, 0, sizeof(id));
    AcceptedList(accept_mask, accepted, sizeof(accepted));
    if (out)
        *out = id;
    if (!d) {
        snprintf(err, err_size, "no disc image (pass the Disc 1 .bin; supported: %s)", accepted);
        return -1;
    }
    if (!PE_Disc_VerifyPVD(d)) {
        snprintf(err, err_size, "image has no ISO9660 volume — not a PlayStation data disc "
                 "(use the .bin of a BIN/CUE dump; supported: %s)", accepted);
        return -1;
    }
    for (i = 0; i < g_pe_known_disc_count; i++) {
        const PE_KnownDisc *k = &g_pe_known_discs[i];
        uint32_t lba, size;
        uint8_t *exe;
        if (!PE_Disc_FindFile(d, k->boot, &lba, &size, NULL, 0))
            continue;
        {
            /* SYSTEM.CNF must boot this very file (serial cross-check). */
            uint8_t *cnf;
            uint32_t cnf_size, j;
            int ok = 0;
            if (PE_DiscCheck_ReadFile(d, "\\SYSTEM.CNF;1", &cnf, &cnf_size) == 0) {
                const char *want = k->boot + 1;           /* skip leading '\\' */
                size_t wl = strlen(want);
                for (j = 0; j + wl <= cnf_size && !ok; j++) {
                    size_t c;
                    for (c = 0; c < wl; c++) {
                        char a = (char)cnf[j + c], b = want[c];
                        if (a >= 'a' && a <= 'z') a = (char)(a - 32);
                        if (a != b) break;
                    }
                    ok = (c == wl);
                }
                free(cnf);
            }
            if (!ok) {
                snprintf(err, err_size, "SYSTEM.CNF does not boot %s (%s): damaged or "
                         "modified image", k->boot + 1, k->serial);
                return -1;
            }
        }
        if (!k->exe_sha1) {
            snprintf(err, err_size, "this is Parasite Eve %s Disc %d (%s), which is not "
                     "supported yet (supported: %s)", k->region, k->disc, k->serial, accepted);
            return -1;
        }
        if (PE_DiscCheck_ReadFile(d, k->boot, &exe, &size) != 0) {
            snprintf(err, err_size, "%s Disc %d boot file %s is unreadable (truncated dump?)",
                     k->region, k->disc, k->serial);
            return -1;
        }
        PE_DiscCheck_Sha1Hex(exe, size, id.exe_sha1);
        free(exe);
        id.exe_size = size;
        if (strcmp(id.exe_sha1, k->exe_sha1) != 0) {
            snprintf(err, err_size, "%s Disc %d (%s) executable SHA-1 %s does not match the "
                     "expected %s — the dump is damaged or a different revision",
                     k->region, k->disc, k->serial, id.exe_sha1, k->exe_sha1);
            return -1;
        }
        if (!(accept_mask & (1u << (k->disc - 1)))) {
            snprintf(err, err_size, "this is Parasite Eve %s Disc %d (%s); start the game "
                     "with %s", k->region, k->disc, k->serial, accepted);
            return -1;
        }
        id.known = k;
        if (out)
            *out = id;
        return 0;
    }
    snprintf(err, err_size, "not a Parasite Eve disc (no known boot executable; supported: %s)",
             accepted);
    return -1;
}
