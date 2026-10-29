/* Guest-code dispatch; contract in pe_guestcode.h. */
#include "pe_guestcode.h"
#include "game_port.h"
#include "pe_bios_string.h"
#include "pe_bootstrap.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

uint8_t PE_LoadU8(pe_addr_t a);

/* Differential-test recorder (test aid; off unless the env var is set):
 * PE_RECORD_GUESTCALL=VMA:DIR[:N[:EVERY]] writes, for every EVERY-th guest call
 * to VMA (at most N), DIR/rec_NNNN.bin = 2 MiB guest RAM + 1 KiB scratchpad +
 * a0..a3 (little-endian u32) taken at entry, before the native body runs.
 * tools/analysis/e21_diff.py --recorded DIR replays them against the retail asm. */
static void record_call(pe_addr_t fn, const uintptr_t a[4])
{
    static int init;
    static unsigned vma, n_max = 64, every = 1, seen, written;
    static char dir[512];
    if (!init) {
        const char *e = getenv("PE_RECORD_GUESTCALL");
        init = 1;
        if (e && *e) {
            char *end;
            vma = (unsigned)strtoul(e, &end, 16);
            if (*end == ':') {
                const char *d = end + 1, *c = strchr(d, ':');
                size_t len = c ? (size_t)(c - d) : strlen(d);
                if (len < sizeof dir) { memcpy(dir, d, len); dir[len] = 0; }
                if (c) { n_max = (unsigned)strtoul(c + 1, &end, 10);
                         if (*end == ':') every = (unsigned)strtoul(end + 1, 0, 10); }
            }
            if (!every) every = 1;
        }
    }
    if (!vma || fn != vma || !dir[0] || written >= n_max || (seen++ % every))
        return;
    {
        char path[600];
        FILE *f;
        unsigned i;
        uint32_t w;
        snprintf(path, sizeof path, "%s/rec_%04u.bin", dir, written);
        f = fopen(path, "wb");
        if (!f) return;
        for (i = 0; i < 0x200000u; i++) fputc(PE_LoadU8(0x80000000u + i), f);
        for (i = 0; i < 0x400u; i++) fputc(PE_LoadU8(0x1F800000u + i), f);
        for (i = 0; i < 4; i++) { w = (uint32_t)a[i]; fwrite(&w, 4, 1, f); }
        fclose(f);
        written++;
    }
}

int PE_Decomp_BoundaryFrom(const char *caller, const char *symbol, unsigned vma,
                           unsigned arity, uintptr_t a0, uintptr_t a1,
                           uintptr_t a2, uintptr_t a3);
void PE_Decomp_NoteStopDetail(const char *text);   /* pe_guest_decomp.c */

/* BIOS TTY vectors missing from the generated EXE registry.  The retail
 * trampolines select A(3Fh) printf at 80071A74 and B(3Fh) puts at 80073C5C.
 * They are different operations: puts never interprets percent operands.
 * Contract: https://psx-spx.consoledev.net/ps1/kernelbios/tty-console-std-io/
 * EXE code is always resident, so these bind by address like the exe table.
 * printf consumes only the words named by its format (up to seven words,
 * the existing native formatter's limit). */
static int bios_printf_thunk8(uintptr_t fmt, uintptr_t a1, uintptr_t a2, uintptr_t a3,
                              uintptr_t a4, uintptr_t a5, uintptr_t a6, uintptr_t a7)
{
    return (int)PE_Printf_Guest((pe_addr_t)fmt, (uint32_t)a1, (uint32_t)a2, (uint32_t)a3,
                                (uint32_t)a4, (uint32_t)a5, (uint32_t)a6, (uint32_t)a7);
}

static int bios_puts_thunk(uintptr_t src, uintptr_t a1, uintptr_t a2, uintptr_t a3)
{
    pe_addr_t addr = (pe_addr_t)src;
    unsigned char c;
    (void)a1; (void)a2; (void)a3;
    /* B(3Fh) has no specified return value.  Match its literal string
     * output, empty-string silence and special NULL spelling; the host
     * TTY backend owns the destination instead of a BIOS character device. */
    if (!addr) {
        fputs("<NULL>", stderr);
        return 0;
    }
    while ((c = PE_LoadU8(addr++)) != 0)
        fputc(c, stderr);
    return 0;
}

static const PeGuestCodeEntry k_bios_tty[] = {
    {0x80071A74u, 12u, 0u, 0, "PE_Printf_Guest (BIOS A(3Fh) printf)", 0u, bios_printf_thunk8, 1u},
    {0x80073C5Cu, 12u, 0u, bios_puts_thunk, "BIOS B(3Fh) puts", 0u, 0, 0u},
};

/* Native game fixed-point leaves at 800370BC/CC. The original ADD traps
 * on signed overflow before its arithmetic shift. No guest memory is read. */
static int game_round_fixed(uintptr_t a0, unsigned shift)
{
    int32_t value = (int32_t)(uint32_t)a0;
    int64_t sum = (int64_t)value + 0x8000;
    if (sum > INT32_MAX) {
        PE_Port_RequestStop(PE_PORT_STOP_GUEST_TRAP);
        return 0;
    }
    /* Division with floor for negative values models MIPS arithmetic shift. */
    if (sum < 0) return (int)(-(((-sum) + ((1LL << shift) - 1)) / (1LL << shift)));
    return (int)(sum / (1LL << shift));
}
static int game_round_int(uintptr_t a, uintptr_t b, uintptr_t c, uintptr_t d)
{ (void)b; (void)c; (void)d; return game_round_fixed(a, 16u); }
static int game_round_byte(uintptr_t a, uintptr_t b, uintptr_t c, uintptr_t d)
{ (void)b; (void)c; (void)d; return game_round_fixed(a, 8u); }
static const PeGuestCodeEntry k_game_fixed_round[] = {
    {0x800370BCu, 16u, 0u, game_round_int, "game_fixed_round_int", 0u, 0, 0u},
    {0x800370CCu, 16u, 0u, game_round_byte, "game_fixed_round_byte", 0u, 0, 0u},
};

static const PeGuestCodeTable *const *pe_gc_tables;
static unsigned pe_gc_table_count;

void PE_GuestCode_SetRegistry(const PeGuestCodeTable *const *tables, unsigned count)
{
    /* Residency belongs to the installed table identities, not just their
     * names. Replacing (or removing) a registry must not inherit old marks. */
    PE_Overlay_ResetResidency();
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

static void invalidate_resident_window(uint32_t lo,uint32_t hi)
{
    for (unsigned i = 0; i < PE_MAX_RESIDENT; i++)
        if (g_resident[i] && g_resident[i]->lo < hi && lo < g_resident[i]->hi)
            g_resident[i] = NULL;
}

int PE_Overlay_SetResident(const char *ovl)
{
    const PeGuestCodeTable *t = find_table(ovl);
    if (!t || strcmp(ovl, "exe") == 0)
        return -1;
    invalidate_resident_window(t->lo,t->hi);
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
        /* A failed load verification cannot leave the previous occupant's
         * unverified mark authorizing calls in this same RAM window. A
         * matching leaf fingerprint can still establish its own identity. */
        invalidate_resident_window(t->lo,t->hi);
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
    /* Code pointers use the same first-2-MiB RAM segment aliases as data.
     * Only normalize the address; all registry/residency/fingerprint and
     * ambiguity checks below still apply to the canonical entry. */
    addr = PE_RamCanonical(addr, 4u);
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
             * Longer bodies stay ambiguous (their callees are room-local),
             * unless both tables list the same generated TU: the generator
             * emits one TU per masked-body group and names every member room
             * in it ("this TU is room_m0022i, room_m0428i's func_..."), so the
             * same symbol over the same bytes is the same native code. */
            /* Two different TUs (room twins whose matched sources differ in
             * text) are also the same native code when their retail closures
             * (this leaf plus every same-room absolute j/jal callee, transitively) are
             * byte-identical: the generator records that as `closure`. */
            int same = by_fp && by_fp->size == e->size &&
                       by_fp->fingerprint == e->fingerprint &&
                       (e->size == 8u ||
                        (by_fp->symbol && e->symbol && strcmp(by_fp->symbol, e->symbol) == 0) ||
                        (e->closure != 0u && by_fp->closure == e->closure));
            if (!same)
                fp_matches++;
            if (!by_fp) by_fp = e;
        }
    }
    if (fp_matches == 0) {
        for (unsigned i = 0; i < sizeof k_game_fixed_round / sizeof k_game_fixed_round[0]; i++)
            if (k_game_fixed_round[i].vma == addr)
                return &k_game_fixed_round[i];
        for (unsigned i = 0; i < sizeof k_bios_tty / sizeof k_bios_tty[0]; i++)
            if (k_bios_tty[i].vma == addr)
                return &k_bios_tty[i];
    }
    return fp_matches == 1 ? by_fp : NULL;
}

/* a[] holds all PE_GUESTCALL_MAX_ARGS slots (unused ones as the site left
 * them: PE_GuestCall forwards all four registers, as the retail jalr does). */
static int guest_call(const char *site, pe_addr_t fn, unsigned arity,
                      const uintptr_t a[PE_GUESTCALL_MAX_ARGS])
{
    const PeGuestCodeEntry *e;
    PE_Port_EscapePoint();      /* armed only by PE_Main_EnterCredits */
    e = fn ? PE_GuestCode_Resolve(fn) : NULL;
    record_call(fn, a);
    if (e && e->thunk)
        return e->thunk(a[0], a[1], a[2], a[3]);
    if (e && e->thunk8) {
        if (arity >= e->nargs)
            return e->thunk8(a[0], a[1], a[2], a[3], a[4], a[5], a[6], a[7]);
        fprintf(stderr, "[GUESTCALL] %s: 0x%08X (%s) takes %u arguments but the site "
                "passes %u (stack arguments unset)\n", site, (unsigned)fn,
                e->symbol ? e->symbol : "?", (unsigned)e->nargs, arity);
    }
    /* No native thunk for this guest code pointer: a boundary (STOP in the
     * native-only port binary), never interpreted. */
    return PE_Decomp_BoundaryFrom(site, site, fn, arity, a[0], a[1], a[2], a[3]);
}

int PE_GuestCallN(const char *site, pe_addr_t fn, unsigned arity, const uintptr_t *args)
{
    uintptr_t a[PE_GUESTCALL_MAX_ARGS] = {0};
    unsigned i, n = arity > PE_GUESTCALL_MAX_ARGS ? PE_GUESTCALL_MAX_ARGS : arity;
    for (i = 0; i < n; i++) a[i] = args[i];
    if (arity > PE_GUESTCALL_MAX_ARGS) {
        fprintf(stderr, "[GUESTCALL] %s: %u arguments to 0x%08X exceed the %u the "
                "port carries\n", site, arity, (unsigned)fn, PE_GUESTCALL_MAX_ARGS);
        return PE_Decomp_BoundaryFrom(site, site, fn, n, a[0], a[1], a[2], a[3]);
    }
    return guest_call(site, fn, arity, a);
}

int PE_GuestCall(const char *site, pe_addr_t fn, unsigned arity,
                 uintptr_t a0, uintptr_t a1, uintptr_t a2, uintptr_t a3)
{
    const uintptr_t a[PE_GUESTCALL_MAX_ARGS] = {a0, a1, a2, a3, 0, 0, 0, 0};
    if (arity > 4u) {
        /* This entry carries registers only. A claimed argument count
         * cannot turn its zero-filled tail into supplied O32 stack words. */
        fprintf(stderr, "[GUESTCALL] %s: %u arguments to 0x%08X exceed the four "
                "registers this entry carries; use PE_GuestCallN for stack arguments\n",
                site, arity, (unsigned)fn);
        return PE_Decomp_BoundaryFrom(site, site, fn, arity, a0, a1, a2, a3);
    }
    return guest_call(site, fn, arity, a);
}

void PE_GuestHook_Refuse(const char *slot, const char *caller, pe_addr_t fn,
                         unsigned arity, uintptr_t a0, uintptr_t a1,
                         uintptr_t a2, uintptr_t a3, const char *why)
{
    char msg[200], detail[400];
    snprintf(msg, sizeof msg,
             "CPU_BOUNDARY/REFUSED pc=0x%08X ra=%s sym=%s arity=%u "
             "a0=0x%08X a1=0x%08X a2=0x%08X a3=0x%08X",
             (unsigned)fn, caller, slot, arity,
             (unsigned)(uint32_t)a0, (unsigned)(uint32_t)a1,
             (unsigned)(uint32_t)a2, (unsigned)(uint32_t)a3);
    snprintf(detail, sizeof detail, "%s (%s)", msg, why);
    fprintf(stderr, "%s (native only: %s; not emulated)\n", msg, why);
    Bootstrap_ReturnVoid4Indirect(slot, caller, (uintptr_t)fn, a0, a1, a2, a3);
    PE_Decomp_NoteStopDetail(detail);
    PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
}

int PE_GuestHook_Call(const char *slot, const char *caller, pe_addr_t fn,
                      unsigned arity, uintptr_t a0, uintptr_t a1,
                      uintptr_t a2, uintptr_t a3, int *ret)
{
    const PeGuestCodeEntry *e = fn ? PE_GuestCode_Resolve(fn) : NULL;
    int r;

    if (!e || !e->thunk) {
        PE_GuestHook_Refuse(slot, caller, fn, arity, a0, a1, a2, a3,
                            e ? "its C takes stack arguments a hook site does not pass"
                              : "no C for this hook target");
        return 0;
    }
    r = PE_GuestCall(slot, fn, arity, a0, a1, a2, a3);
    if (ret) *ret = r;
    return 1;
}
