/* Full-callee differential for the generated native M0086I park callbacks.
 * Adapted from test_room_m0086i_F7.c in the integrated later-days checkout.
 * Expected values are the original retail-body oracle's synthetic fixtures;
 * all calls here go through the current generated guest-code registry into
 * the actual native callees. No interpreter is linked or called by this test.
 *
 * Compares every fixture-window RAM word, every scratchpad byte, all 64 GTE
 * registers, return values and RNG state; checks the rest of RAM for stray
 * writes. Existing masks cover only retail uninitialized packet-template
 * bytes. Retains per-case input perturbations and RAM/scratch/GTE comparer
 * negative controls. Legacy hand-port call counters are not available for
 * generated code; calls must instead resolve and finish with zero boundaries,
 * zero bootstrap invocations and no stop request.
 *
 * Fixtures are not room execution or visual/audio/timing/save acceptance.
 * The sound archive fixtures do not contain the requested sound resource,
 * so sound-driver playback is outside their coverage. Requires the owner's
 * disc cache/image; returns 77 if unavailable and fails on an EXE identity
 * mismatch. Never changes owner assets.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "pe_sdk.h"
#include "pe_guest_ram.h"
#include "pe_guestcode.h"
#include "pe_disc_cache.h"
#include "pe_disc_check.h"
#include "pe_guest_decomp.h"
#include "game_port.h"
#include "stub_registry.h"
#include "retail_park_m0086_full_cases.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* Retail whole-body identities; independent of generated host symbol names. */
enum { PE_M7_B734, PE_M7_B004, PE_M7_B574, PE_M7_B0CC, PE_M7_NBODIES };
#define PE_ROOM_M0086I_8018F734_VMA 0x8018F734u
#define PE_ROOM_M0086I_8018F004_VMA 0x8018F004u
#define PE_ROOM_M0086I_80190574_VMA 0x80190574u
#define PE_ROOM_M0086I_801900CC_VMA 0x801900CCu
#define PE_ROOM_M0086I_8018F734_SIZE 0x998u
#define PE_ROOM_M0086I_8018F004_SIZE 0x730u
#define PE_ROOM_M0086I_80190574_SIZE 0x4D0u
#define PE_ROOM_M0086I_801900CC_SIZE 0x4A8u
#define PE_ROOM_M0086I_8018F734_FNV1A 0x4064A8DCu
#define PE_ROOM_M0086I_8018F004_FNV1A 0x233180C8u
#define PE_ROOM_M0086I_80190574_FNV1A 0x446CFDE0u
#define PE_ROOM_M0086I_801900CC_FNV1A 0x1E091749u
#define RETAIL_EXE_SHA1 "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"

void Trace_Direct(const char *event) { (void)event; }

#define N(a) (sizeof(a) / sizeof((a)[0]))
#define EXE_LOAD 0x80010000u
#define EXE_HDR 0x800u
#define EXE_FILE_LEN 0x1EE800u          /* SLUS_006.62: 0x800 header + 0x1EE000 text/data */
#define SPAD 0x1F800000u
static int failures, checks;
#define CHECK(cond, ...) do { checks++; if (!(cond)) { failures++; \
    fprintf(stderr, "FAIL: " __VA_ARGS__); fputc('\n', stderr); } } while (0)

static const uint32_t s_vma[PE_M7_NBODIES] = {
    PE_ROOM_M0086I_8018F734_VMA, PE_ROOM_M0086I_8018F004_VMA, PE_ROOM_M0086I_80190574_VMA, PE_ROOM_M0086I_801900CC_VMA
};
static const uint32_t s_size[PE_M7_NBODIES] = {
    PE_ROOM_M0086I_8018F734_SIZE, PE_ROOM_M0086I_8018F004_SIZE, PE_ROOM_M0086I_80190574_SIZE, PE_ROOM_M0086I_801900CC_SIZE
};
static const uint32_t s_fnv[PE_M7_NBODIES] = {
    PE_ROOM_M0086I_8018F734_FNV1A, PE_ROOM_M0086I_8018F004_FNV1A, PE_ROOM_M0086I_80190574_FNV1A, PE_ROOM_M0086I_801900CC_FNV1A
};

static uint8_t *room_bytes, *exe_bytes;
static uint32_t *exp_words;               /* expected words of the compared windows, window order */
static uint32_t *exp_masks;               /* per word: bytes not compared (stack garbage in retail) */
static unsigned exp_count;

static int load_disc(void)
{
    char err[256], sha1[41];
    unsigned w;
    room_bytes = malloc(M7_ROOM_LEN);
    exe_bytes = malloc(EXE_FILE_LEN);
    for (w = 0; w < N(M7_compare); w++) exp_count += (M7_compare[w][1] - M7_compare[w][0]) / 4u;
    exp_words = malloc(exp_count * 4u);
    exp_masks = malloc(exp_count * 4u);
    if (!room_bytes || !exe_bytes || !exp_words || !exp_masks) return -2;
    if (PE_DiscCache_FileRead(PE_DISC_SRC_PEIMG, M7_ROOM_OFF, room_bytes, M7_ROOM_LEN, err, sizeof err) != 0) {
        fprintf(stderr, "PE.IMG unavailable (%s)\n", err);
        return -1;
    }
    if (PE_DiscCache_FileRead(PE_DISC_SRC_EXE, 0, exe_bytes, EXE_FILE_LEN, err, sizeof err) != 0) {
        fprintf(stderr, "EXE unavailable (%s)\n", err);
        return -1;
    }
    PE_DiscCheck_Sha1Hex(exe_bytes, EXE_FILE_LEN, sha1);
    if (strcmp(sha1, RETAIL_EXE_SHA1) != 0) {
        fprintf(stderr, "EXE identity mismatch: %s, expected %s\n", sha1, RETAIL_EXE_SHA1);
        return -2;
    }
    return 0;
}

static uint8_t zone_byte(uint32_t a) { return (uint8_t)((a - M7_ZONE_LO) * 17u + 7u); }
static uint8_t scratch_byte(uint32_t i) { return (uint8_t)(i * 37u + 1u); }
static int in_chunk(uint32_t a) { return a >= M7_VB && a < M7_VB + M7_ROOM_LEN; }
static int in_exe(uint32_t a) { return a >= EXE_LOAD && a < EXE_LOAD + (EXE_FILE_LEN - EXE_HDR); }
static int in_gframe(uint32_t a) { return a >= PE_GUEST_FRAME_LO && a < PE_GUEST_FRAME_HI; }

/* window index of a word address, or -1 */
static long win_index(uint32_t a)
{
    unsigned w;
    long base = 0;
    for (w = 0; w < N(M7_compare); w++) {
        if (a >= M7_compare[w][0] && a < M7_compare[w][1]) return base + (long)((a - M7_compare[w][0]) / 4u);
        base += (long)((M7_compare[w][1] - M7_compare[w][0]) / 4u);
    }
    return -1;
}

/* The starting word at `a` outside the case seeds: the EXE image, the room
 * chunk over it, the zone pattern over the zone (the oracle's layering). */
static uint32_t background_word(uint32_t a)
{
    uint32_t v = 0, k;
    if (in_exe(a)) memcpy(&v, exe_bytes + EXE_HDR + (a - EXE_LOAD), 4);
    if (in_chunk(a)) memcpy(&v, room_bytes + (a - M7_VB), 4);
    if (a >= M7_ZONE_LO && a < M7_ZONE_HI) {
        v = 0;
        for (k = 0; k < 4; k++) v |= (uint32_t)zone_byte(a + k) << (8u * k);
    }
    return v;
}

static uint32_t s_tamper_addr, s_tamper_xor;           /* comparer negative control: RAM word */
static unsigned s_tamper_scratch = 0xFFFFFFFFu;        /* scratch offset */
static unsigned s_tamper_gte = 0xFFFFFFFFu;            /* GTE register index 0..63 */

/* expected image of the windows: background, then the seed rows, then the oracle's changed words */
static void build_expected(const M7Case *c)
{
    unsigned w, k, i = 0;
    uint32_t a, n;
    for (w = 0; w < N(M7_compare); w++)
        for (a = M7_compare[w][0]; a < M7_compare[w][1]; a += 4u) { exp_words[i] = background_word(a); exp_masks[i] = 0; i++; }
    for (k = 0; k < N(M7_base_seed); k++)
        for (n = 0; n < M7_base_seed[k][2]; n++) {
            long x = win_index(M7_base_seed[k][0] + 4u * n);
            if (x >= 0) exp_words[x] = M7_base_seed[k][1];
        }
    for (k = c->seed_first; k < c->seed_end; k++)
        for (n = 0; n < M7_seed[k][2]; n++) {
            long x = win_index(M7_seed[k][0] + 4u * n);
            if (x >= 0) exp_words[x] = M7_seed[k][1];
        }
    for (k = c->diff_first; k < c->diff_end; k++)
        for (n = 0; n < M7_diff[k][2]; n++) {
            long x = win_index(M7_diff[k][0] + 4u * n);
            if (x >= 0) exp_words[x] = M7_diff[k][1];
        }
    for (k = c->mask_first; k < c->mask_end; k++) {
        long x = win_index(M7_mask[k][0]);
        if (x >= 0) exp_masks[x] = M7_mask[k][1];
    }
}

static void seed_case(const M7Case *c)
{
    uint32_t a, k, n;
    PE_RamReset();
    PE_Sdk_ResetState();
    PE_Overlay_ResetResidency();
    PE_Port_RunControlReset();
    PE_Decomp_ResetBoundaries();
    PE_Decomp_SetBoundaryStop(1);
    Stub_ResetOrderLog();
    g_stub_bootstrap_invocations = 0;
    g_bootstrap_disc = 0;
    g_strict_stubs = 0;
    memcpy(PE_Translate(EXE_LOAD, EXE_FILE_LEN - EXE_HDR), exe_bytes + EXE_HDR, EXE_FILE_LEN - EXE_HDR);
    memcpy(PE_Translate(M7_VB, M7_ROOM_LEN), room_bytes, M7_ROOM_LEN);
    for (a = M7_ZONE_LO; a < M7_ZONE_HI; a++) PE_StoreU8(a, zone_byte(a));
    for (k = 0; k < N(M7_base_seed); k++)
        for (n = 0; n < M7_base_seed[k][2]; n++) PE_StoreU32(M7_base_seed[k][0] + 4u * n, M7_base_seed[k][1]);
    for (k = c->seed_first; k < c->seed_end; k++)
        for (n = 0; n < M7_seed[k][2]; n++) PE_StoreU32(M7_seed[k][0] + 4u * n, M7_seed[k][1]);
    for (k = 0; k < 0x400u; k++) PE_StoreU8(SPAD + k, scratch_byte(k));
    for (k = 0; k < 32; k++) PE_GTE_WriteCtrl(k, M7_ctrl_init[k]);
    for (k = 0; k < 32; k++)
        if (k != 15 && k != 28 && k != 29 && k != 31) PE_GTE_WriteData(k, M7_data_init[k]);
    func_80071A64(c->rand_seed);                        /* BIOS srand state retail started from */
}

/* Words / scratchpad bytes / GTE registers that differ from the oracle (the word at `skip`, if any, is ignored). */
static unsigned compare_all(const M7Case *c, int report, uint32_t skip)
{
    unsigned bad = 0, w, n, s = c->sd_first, i = 0;
    uint32_t a;
    for (w = 0; w < N(M7_compare); w++)
        for (a = M7_compare[w][0]; a < M7_compare[w][1]; a += 4u, i++) {
            uint32_t want = exp_words[i], got = PE_LoadU32(a), keep = ~exp_masks[i];
            if (a == skip) continue;
            if (a == s_tamper_addr) want ^= s_tamper_xor;
            if ((got & keep) != (want & keep)) {
                if (report && bad < 6)
                    fprintf(stderr, "  %s: %08X got %08X want %08X (mask %08X)\n", c->name, a, got, want, exp_masks[i]);
                bad++;
            }
        }
    for (n = 0; n < 0x400u; n++) {
        uint8_t want = scratch_byte(n), got = PE_LoadU8(SPAD + n);
        while (s < c->sd_end && n >= (unsigned)M7_sdiff[s][0] + M7_sdiff[s][1]) s++;   /* sorted runs */
        if (s < c->sd_end && n >= M7_sdiff[s][0]) want = (uint8_t)M7_sdiff[s][2];
        if (n == s_tamper_scratch) want ^= 0x01u;
        if (got != want) {
            if (report && bad < 6) fprintf(stderr, "  %s: scratch+%03X native %02X retail %02X\n", c->name, n, got, want);
            bad++;
        }
    }
    for (n = 0; n < 64; n++) {
        uint32_t got = n < 32 ? PE_GTE_ReadData(n) : PE_GTE_ReadCtrl(n - 32u);
        uint32_t want = c->gte[n] ^ (n == s_tamper_gte ? 1u : 0u);
        if (got != want) {
            if (report && bad < 6) fprintf(stderr, "  %s: GTE %s%u native %08X retail %08X\n", c->name,
                                           n < 32 ? "data" : "ctrl", n & 31u, got, want);
            bad++;
        }
    }
    return bad;
}

/* Nothing outside the compared windows (and the port guest frame window) may change. */
static unsigned stray_writes(const M7Case *c)
{
    unsigned bad = 0;
    uint32_t a;
    for (a = 0x80000000u; a < 0x80200000u; a += 4u) {
        if (win_index(a) >= 0 || in_gframe(a)) continue;
        if (PE_LoadU32(a) != background_word(a)) {
            if (bad < 4) fprintf(stderr, "  %s: stray write at %08X = %08X\n", c->name, a, PE_LoadU32(a));
            bad++;
        }
    }
    return bad;
}

static uint32_t call_body(unsigned body, const uint32_t *args)
{
    const PeGuestCodeEntry *entry;
    uint32_t result;
    unsigned epoch = PE_Port_StopEpoch();
    CHECK(body < PE_M7_NBODIES, "invalid body index %u", body);
    if (body >= PE_M7_NBODIES) return 0;
    entry = PE_GuestCode_Resolve(s_vma[body]);
    CHECK(entry && entry->thunk, "%08X has a native registry thunk", s_vma[body]);
    if (!entry || !entry->thunk) return 0;
    result = (uint32_t)PE_GuestCall("@park-m0086-full", s_vma[body], 3u,
                                  args[0], args[1], args[2], 0x5555u);
    CHECK(PE_Decomp_BoundaryCount() == 0, "%08X reached %u unresolved boundaries",
          s_vma[body], PE_Decomp_BoundaryCount());
    CHECK(g_stub_order_count == 0 && g_stub_bootstrap_invocations == 0,
          "%08X reached bootstrap/stub code", s_vma[body]);
    CHECK(PE_Port_StopEpoch() == epoch && !PE_Port_ShouldStop(),
          "%08X requested a stop", s_vma[body]);
    return result;
}

static void test_cases(void)
{
    unsigned i;
    unsigned long pert_controls = 0;
    for (i = 0; i < N(M7_cases); i++) {
        const M7Case *c = &M7_cases[i];
        unsigned bad, stray;
        uint32_t ret, rnext;
        build_expected(c);
        seed_case(c);
        ret = call_body(c->body, c->args);
        rnext = func_80071A54();
        bad = compare_all(c, 1, 0);
        stray = stray_writes(c);
        CHECK(bad == 0, "%s: %u mismatching words / scratch bytes / GTE registers", c->name, bad);
        CHECK(stray == 0, "%s: %u stray writes outside the compared windows", c->name, stray);
        CHECK(ret == c->v0, "%s: return %X, retail v0 %X", c->name, ret, c->v0);
        CHECK(rnext == c->rand_next, "%s: next rand %X, retail %X (rand call count differs)", c->name, rnext,
              c->rand_next);

        /* input-perturbation negative control: the retail outcome changes, so must the native one */
        if (c->pert_kind) {
            uint32_t args[3], skip = 0;
            unsigned pbad;
            memcpy(args, c->args, sizeof args);
            seed_case(c);
            if (c->pert_kind == 1u) {
                PE_StoreU32(c->pert_key, PE_LoadU32(c->pert_key) ^ c->pert_xor);
                skip = c->pert_key;
            } else {
                args[c->pert_key] ^= c->pert_xor;
            }
            pbad = (unsigned)(call_body(c->body, args) != c->v0);
            pbad += compare_all(c, 0, skip);
            CHECK(pbad > 0, "%s: perturbation (kind %u key %X xor %X) not detected", c->name, c->pert_kind,
                  c->pert_key, c->pert_xor);
            pert_controls++;
        }
        printf("  %-34s mismatches=%u stray=%u%s\n", c->name,
               bad, stray, c->pert_kind ? " perturbation checked" : "");
    }
    CHECK(pert_controls >= N(M7_cases) / 2u, "too few perturbation controls ran (%lu)", pert_controls);
    printf("park m0086 full: %lu input-perturbation controls\n", pert_controls);
}

static const M7Case *find_case(const char *name)
{
    unsigned i;
    for (i = 0; i < N(M7_cases); i++) if (!strcmp(M7_cases[i].name, name)) return &M7_cases[i];
    fprintf(stderr, "FAIL: case %s missing from the header\n", name);
    exit(1);
}

/* Comparer negative controls: a deliberately wrong word / scratch byte / GTE register must be detected. */
static void test_negative_control(void)
{
    const M7Case *c = find_case("b734_init");
    const M7Case *d = find_case("b734_draw_st0");
    unsigned bad;
    build_expected(c);
    seed_case(c);
    (void)call_body(c->body, c->args);
    CHECK(compare_all(c, 0, 0) == 0, "negative control baseline");
    s_tamper_addr = M7_O + 0x8u; s_tamper_xor = 0x80u;
    bad = compare_all(c, 0, 0);
    s_tamper_addr = 0; s_tamper_xor = 0;
    CHECK(bad == 1, "negative control: tampered word not detected (%u)", bad);
    build_expected(d);
    seed_case(d);
    (void)call_body(d->body, d->args);
    CHECK(compare_all(d, 0, 0) == 0, "draw negative control baseline");
    s_tamper_scratch = 0x30u;
    bad = compare_all(d, 0, 0);
    s_tamper_scratch = 0xFFFFFFFFu;
    CHECK(bad == 1, "negative control: tampered scratch byte not detected (%u)", bad);
    s_tamper_gte = 25u;                                  /* MAC1 */
    bad = compare_all(d, 0, 0);
    s_tamper_gte = 0xFFFFFFFFu;
    CHECK(bad == 1, "negative control: tampered GTE register not detected (%u)", bad);
}

/* The particle callbacks are stored as guest addresses, the meshes as the archive guest addresses. */
static void test_guest_addresses(void)
{
    const M7Case *c = find_case("b734_init");
    const M7Case *e = find_case("b574_init");
    seed_case(c);
    (void)call_body(c->body, c->args);
    CHECK(PE_LoadU32(M7_POOL + 8u) == PE_ROOM_M0086I_8018F004_VMA, "8018F734 cmd 0 stores guest address 8018F004");
    CHECK(PE_LoadU32(0x80190B88u) == PE_LoadU32(0x800B0E64u) + 0x200u, "cmd 0 stores the first archive mesh");
    CHECK(PE_LoadU32(0x80190B84u) == PE_LoadU32(0x800B0E64u) + 0x800u, "cmd 0 stores the second archive mesh");
    seed_case(e);
    (void)call_body(e->body, e->args);
    CHECK(PE_LoadU32(M7_POOL + 8u) == PE_ROOM_M0086I_801900CC_VMA, "80190574 cmd 0 stores guest address 801900CC");
}

/* Identity of the loaded room code against the census values. */
static void test_identity(void)
{
    const M7Case *c = find_case("b734_init");
    unsigned k;
    seed_case(c);
    for (k = 0; k < PE_M7_NBODIES; k++)
        CHECK(PE_GuestCode_Fnv1a(s_vma[k], s_size[k]) == s_fnv[k], "%08X fnv", s_vma[k]);
}

/* Registry identity rejects interior addresses and changed retail code. */
static void test_registry(void)
{
    const M7Case *c = find_case("b734_init");
    unsigned k;
    seed_case(c);
    for (k = 0; k < PE_M7_NBODIES; k++) {
        const PeGuestCodeEntry *entry = PE_GuestCode_Resolve(s_vma[k]);
        CHECK(entry && entry->thunk, "%08X registry entry", s_vma[k]);
        if (entry) {
            CHECK(entry->vma == s_vma[k] && entry->size == s_size[k],
                  "%08X registry whole-body bounds", s_vma[k]);
            CHECK(entry->fingerprint == s_fnv[k], "%08X registry fingerprint", s_vma[k]);
        }
    }
    CHECK(PE_GuestCode_Resolve(0x8018F738u) == NULL, "registry rejects interior address");
    for (k = 0; k < PE_M7_NBODIES; k++) {
        uint32_t at = s_vma[k] + s_size[k] - 8u;
        uint8_t old;
        seed_case(c);
        old = PE_LoadU8(at);
        PE_StoreU8(at, old ^ 1u);
        CHECK(PE_GuestCode_Resolve(s_vma[k]) == NULL,
              "registry rejects changed %08X", s_vma[k]);
        PE_StoreU8(at, old);
        CHECK(PE_GuestCode_Resolve(s_vma[k]) != NULL,
              "registry accepts restored %08X", s_vma[k]);
    }
}

int main(void)
{
    int loaded;
    PE_RamInit();
    PE_GuestCode_InstallGenerated();
    loaded = load_disc();
    if (loaded != 0) {
        fprintf(stderr, "park m0086 full: %s\n", loaded == -1 ? "SKIP (no owner disc/cache)" : "FAIL (fixture identity/allocation)");
        free(room_bytes); free(exe_bytes); free(exp_words); free(exp_masks);
        return loaded == -1 ? 77 : 1;
    }
    printf("park m0086 full: %u retail cases (fixtures, not room execution)\n", (unsigned)N(M7_cases));
    test_identity();
    test_registry();
    test_cases();
    test_negative_control();
    test_guest_addresses();
    printf("park m0086 full: %d checks, %d failures\n", checks, failures);
    free(room_bytes); free(exe_bytes); free(exp_words); free(exp_masks);
    return failures ? 1 : 0;
}
