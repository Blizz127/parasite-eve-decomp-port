/* Guest-code dispatch; contract in pe_guestcode.h. */
#include "pe_guestcode.h"
#include <stdio.h>
#include <string.h>

int PE_Decomp_Boundary(const char *symbol, unsigned vma, unsigned arity,
                       uintptr_t a0, uintptr_t a1, uintptr_t a2, uintptr_t a3);

static const PeGuestCodeTable *const *pe_gc_tables;
static unsigned pe_gc_table_count;

void PE_GuestCode_SetRegistry(const PeGuestCodeTable *const *tables, unsigned count)
{
    pe_gc_tables = tables;
    pe_gc_table_count = tables ? count : 0u;
}

#define PE_MAX_RESIDENT 8
static const PeGuestCodeTable *g_resident[PE_MAX_RESIDENT];

static const PeGuestCodeTable *find_table(const char *id)
{
    for (unsigned i = 0; i < pe_gc_table_count; i++)
        if (strcmp(pe_gc_tables[i]->id, id) == 0)
            return pe_gc_tables[i];
    return NULL;
}

void PE_Overlay_ResetResidency(void)
{
    memset(g_resident, 0, sizeof(g_resident));
}

int PE_Overlay_SetResident(const char *ovl)
{
    const PeGuestCodeTable *t = find_table(ovl);
    if (!t || strcmp(ovl, "exe") == 0)
        return -1;
    for (unsigned i = 0; i < PE_MAX_RESIDENT; i++)
        if (g_resident[i] && g_resident[i]->lo < t->hi && t->lo < g_resident[i]->hi)
            g_resident[i] = NULL;
    for (unsigned i = 0; i < PE_MAX_RESIDENT; i++)
        if (!g_resident[i]) {
            g_resident[i] = t;
            return 0;
        }
    return -1;
}

int PE_Overlay_IsResident(const char *ovl)
{
    for (unsigned i = 0; i < PE_MAX_RESIDENT; i++)
        if (g_resident[i] && strcmp(g_resident[i]->id, ovl) == 0)
            return 1;
    return 0;
}

/* ── SHA-1 (FIPS 180-4) over guest RAM ─────────────────────────────── */
static uint32_t rol(uint32_t v, int n) { return (v << n) | (v >> (32 - n)); }

static void sha1_block(uint32_t h[5], const uint8_t b[64])
{
    uint32_t w[80], a, bb, c, d, e, f, k, t;
    for (int i = 0; i < 16; i++)
        w[i] = (uint32_t)b[4*i] << 24 | (uint32_t)b[4*i+1] << 16 |
               (uint32_t)b[4*i+2] << 8 | b[4*i+3];
    for (int i = 16; i < 80; i++)
        w[i] = rol(w[i-3] ^ w[i-8] ^ w[i-14] ^ w[i-16], 1);
    a = h[0]; bb = h[1]; c = h[2]; d = h[3]; e = h[4];
    for (int i = 0; i < 80; i++) {
        if (i < 20)      { f = (bb & c) | (~bb & d);          k = 0x5A827999u; }
        else if (i < 40) { f = bb ^ c ^ d;                    k = 0x6ED9EBA1u; }
        else if (i < 60) { f = (bb & c) | (bb & d) | (c & d); k = 0x8F1BBCDCu; }
        else             { f = bb ^ c ^ d;                    k = 0xCA62C1D6u; }
        t = rol(a, 5) + f + e + k + w[i];
        e = d; d = c; c = rol(bb, 30); bb = a; a = t;
    }
    h[0] += a; h[1] += bb; h[2] += c; h[3] += d; h[4] += e;
}

void PE_GuestCode_Sha1Hex(pe_addr_t addr, uint32_t size, char out[41])
{
    uint32_t h[5] = {0x67452301u, 0xEFCDAB89u, 0x98BADCFEu, 0x10325476u, 0xC3D2E1F0u};
    uint8_t blk[64];
    uint64_t bits = (uint64_t)size * 8u;
    uint32_t i = 0, n = 0;
    for (; i < size; i++) {
        blk[n++] = PE_LoadU8(addr + i);
        if (n == 64) { sha1_block(h, blk); n = 0; }
    }
    blk[n++] = 0x80;
    if (n > 56) { while (n < 64) blk[n++] = 0; sha1_block(h, blk); n = 0; }
    while (n < 56) blk[n++] = 0;
    for (int j = 7; j >= 0; j--) blk[n++] = (uint8_t)(bits >> (8 * j));
    sha1_block(h, blk);
    for (int j = 0; j < 5; j++)
        snprintf(out + 8 * j, 9, "%08x", h[j]);
}

int PE_Overlay_SetResidentVerified(const char *ovl)
{
    const PeGuestCodeTable *t = find_table(ovl);
    char got[41];
    if (!t || !t->sha1 || !t->load_size)
        return -1;
    PE_GuestCode_Sha1Hex(t->lo, t->load_size, got);
    if (strcmp(got, t->sha1) != 0) {
        fprintf(stderr, "[OVERLAY] %s: guest RAM SHA-1 %s != manifest %s; "
                "not marked resident\n", ovl, got, t->sha1);
        return -2;
    }
    return PE_Overlay_SetResident(ovl);
}

uint32_t PE_GuestCode_Fnv1a(pe_addr_t addr, uint32_t size)
{
    uint32_t h = 0x811C9DC5u;
    for (uint32_t i = 0; i < size; i++) {
        h ^= PE_LoadU8(addr + i);
        h *= 0x01000193u;
    }
    return h;
}

static const PeGuestCodeEntry *find_entry(const PeGuestCodeTable *t, pe_addr_t addr)
{
    unsigned lo = 0, hi = t->count;          /* entries sorted by vma */
    while (lo < hi) {
        unsigned mid = (lo + hi) / 2;
        if (t->entries[mid].vma < addr) lo = mid + 1; else hi = mid;
    }
    return (lo < t->count && t->entries[lo].vma == addr) ? &t->entries[lo] : NULL;
}

const PeGuestCodeEntry *PE_GuestCode_Resolve(pe_addr_t addr)
{
    const PeGuestCodeEntry *by_fp = NULL;
    int fp_matches = 0;

    for (unsigned i = 0; i < pe_gc_table_count; i++) {
        const PeGuestCodeTable *t = pe_gc_tables[i];
        if (addr < t->lo || addr >= t->hi)
            continue;
        const PeGuestCodeEntry *e = find_entry(t, addr);
        if (!e)
            continue;
        if (strcmp(t->id, "exe") == 0)
            return e;
        if (PE_Overlay_IsResident(t->id))
            return e;
        if (e->fingerprint && PE_RangeIsRam(addr, e->size) &&
            PE_GuestCode_Fnv1a(addr, e->size) == e->fingerprint) {
            /* The same resident bytes at the same address are the same
             * code whichever room table lists them: the 8-byte `return 0`
             * behaviour-table stubs every room carries count as one match.
             * Longer bodies stay ambiguous (their callees are room-local). */
            if (!by_fp || e->size != 8u || by_fp->size != e->size ||
                by_fp->fingerprint != e->fingerprint)
                fp_matches++;
            if (!by_fp) by_fp = e;
        }
    }
    return fp_matches == 1 ? by_fp : NULL;
}

int PE_GuestCall(const char *site, pe_addr_t fn, unsigned arity,
                 uintptr_t a0, uintptr_t a1, uintptr_t a2, uintptr_t a3)
{
    const PeGuestCodeEntry *e = fn ? PE_GuestCode_Resolve(fn) : NULL;
    if (e && e->thunk)
        return e->thunk(a0, a1, a2, a3);
    return PE_Decomp_Boundary(site, fn, arity, a0, a1, a2, a3);
}
