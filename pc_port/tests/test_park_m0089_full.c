/* Full-callee retail differential for the generated native M0089I effect.
 * Adapted from test_room_m0089i_F9hand.c in the preserved later-days checkout.
 * All 77 retained synthetic fixtures now dispatch 80193618 through the current
 * generated registry and execute actual native callees. No CPU interpreter or
 * legacy hand implementation is linked or called by this adapter.
 *
 * Retains compared RAM windows, stray-write checks over the rest of RAM,
 * scratchpad, all 64 GTE registers, return values, RNG state, per-case input
 * perturbations and RAM/scratch/GTE comparer negative controls. The existing
 * per-case masks cover only uninitialized retail fan-packet bytes. Legacy
 * hand-port hit counters are unavailable for generated code; every call must
 * instead resolve with zero unresolved boundaries, bootstrap calls or stops.
 *
 * The pinned retail EXE SHA-1 and whole-body fingerprint establish fixture
 * input identity. Fixtures do not prove room reachability, visual appearance,
 * audio playback, timing or save fidelity. Missing owner assets return 77;
 * mismatched assets fail. No owner assets are modified.
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
#include "retail_park_m0089_full_cases.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PE_ROOM_M0089I_80193618_VMA 0x80193618u
#define PE_ROOM_M0089I_80193618_SIZE 0x9D8u
#define PE_ROOM_M0089I_80193618_FNV1A 0xBBA18A69u
#define PE_ROOM_M0089I_DATA_8018F1CC 0x8018F1CCu
#define PE_ROOM_M0089I_POOL_CALLBACK_80193200 0x80193200u
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

static uint8_t *room_bytes, *exe_bytes;
static uint32_t *exp_words;               /* expected words of the compared windows, window order */
static uint32_t *exp_masks;               /* per word: bytes not compared (stack garbage in retail) */
static unsigned exp_count;

static int load_disc(void)
{
    char err[256], sha1[41];
    unsigned w;
    room_bytes = malloc(M9_ROOM_LEN);
    exe_bytes = malloc(EXE_FILE_LEN);
    for (w = 0; w < N(M9_compare); w++) exp_count += (M9_compare[w][1] - M9_compare[w][0]) / 4u;
    exp_words = malloc(exp_count * 4u);
    exp_masks = malloc(exp_count * 4u);
    if (!room_bytes || !exe_bytes || !exp_words || !exp_masks) return -2;
    if (PE_DiscCache_FileRead(PE_DISC_SRC_PEIMG, M9_ROOM_OFF, room_bytes, M9_ROOM_LEN, err, sizeof err) != 0) {
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

static uint8_t zone_byte(uint32_t a) { return (uint8_t)((a - M9_ZONE_LO) * 17u + 7u); }
static uint8_t scratch_byte(uint32_t i) { return (uint8_t)(i * 37u + 1u); }
static int in_chunk(uint32_t a) { return a >= M9_VB && a < M9_VB + M9_ROOM_LEN; }
static int in_exe(uint32_t a) { return a >= EXE_LOAD && a < EXE_LOAD + (EXE_FILE_LEN - EXE_HDR); }
static int in_gframe(uint32_t a) { return a >= PE_GUEST_FRAME_LO && a < PE_GUEST_FRAME_HI; }

/* window index of a word address, or -1 */
static long win_index(uint32_t a)
{
    unsigned w;
    long base = 0;
    for (w = 0; w < N(M9_compare); w++) {
        if (a >= M9_compare[w][0] && a < M9_compare[w][1]) return base + (long)((a - M9_compare[w][0]) / 4u);
        base += (long)((M9_compare[w][1] - M9_compare[w][0]) / 4u);
    }
    return -1;
}

/* The starting word at `a` outside the case seeds: the EXE image, the room
 * chunk over it, the zone pattern over the zone (the oracle's layering). */
static uint32_t background_word(uint32_t a)
{
    uint32_t v = 0, k;
    if (in_exe(a)) memcpy(&v, exe_bytes + EXE_HDR + (a - EXE_LOAD), 4);
    if (in_chunk(a)) memcpy(&v, room_bytes + (a - M9_VB), 4);
    if (a >= M9_ZONE_LO && a < M9_ZONE_HI) {
        v = 0;
        for (k = 0; k < 4; k++) v |= (uint32_t)zone_byte(a + k) << (8u * k);
    }
    return v;
}

static uint32_t s_tamper_addr, s_tamper_xor;           /* comparer negative control: RAM word */
static unsigned s_tamper_scratch = 0xFFFFFFFFu;        /* scratch offset */
static unsigned s_tamper_gte = 0xFFFFFFFFu;            /* GTE register index 0..63 */

/* expected image of the windows: background, then the seed rows, then the oracle's changed words */
static void build_expected(const M9Case *c)
{
    unsigned w, k, i = 0;
    uint32_t a, n;
    for (w = 0; w < N(M9_compare); w++)
        for (a = M9_compare[w][0]; a < M9_compare[w][1]; a += 4u) { exp_words[i] = background_word(a); exp_masks[i] = 0; i++; }
    for (k = c->seed_first; k < c->seed_end; k++)
        for (n = 0; n < M9_seed[k][2]; n++) {
            long x = win_index(M9_seed[k][0] + 4u * n);
            if (x >= 0) exp_words[x] = M9_seed[k][1];
        }
    for (k = c->diff_first; k < c->diff_end; k++) {
        long x = win_index(M9_diff[k][0]);
        if (x >= 0) exp_words[x] = M9_diff[k][1];
    }
    for (k = c->mask_first; k < c->mask_end; k++) {
        long x = win_index(M9_mask[k][0]);
        if (x >= 0) exp_masks[x] = M9_mask[k][1];
    }
}

static void seed_case(const M9Case *c)
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
    memcpy(PE_Translate(M9_VB, M9_ROOM_LEN), room_bytes, M9_ROOM_LEN);
    for (a = M9_ZONE_LO; a < M9_ZONE_HI; a++) PE_StoreU8(a, zone_byte(a));
    for (k = c->seed_first; k < c->seed_end; k++)
        for (n = 0; n < M9_seed[k][2]; n++) PE_StoreU32(M9_seed[k][0] + 4u * n, M9_seed[k][1]);
    for (k = 0; k < 0x400u; k++) PE_StoreU8(SPAD + k, scratch_byte(k));
    for (k = 0; k < 32; k++) PE_GTE_WriteCtrl(k, M9_ctrl_init[k]);
    for (k = 0; k < 32; k++)
        if (k != 15 && k != 28 && k != 29 && k != 31) PE_GTE_WriteData(k, M9_data_init[k]);
    func_80071A64(c->rand_seed);                        /* BIOS srand state retail started from */
}

/* Words / scratchpad bytes / GTE registers that differ from the oracle (the word at `skip`, if any, is ignored). */
static unsigned compare_all(const M9Case *c, int report, uint32_t skip)
{
    unsigned bad = 0, w, n, s = c->sd_first, i = 0;
    uint32_t a;
    for (w = 0; w < N(M9_compare); w++)
        for (a = M9_compare[w][0]; a < M9_compare[w][1]; a += 4u, i++) {
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
        while (s < c->sd_end && n >= (unsigned)M9_sdiff[s][0] + M9_sdiff[s][1]) s++;   /* sorted runs */
        if (s < c->sd_end && n >= M9_sdiff[s][0]) want = (uint8_t)M9_sdiff[s][2];
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
static unsigned stray_writes(const M9Case *c)
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

static uint32_t call_body(const M9Case *c, const uint32_t *args)
{
    const PeGuestCodeEntry *entry = PE_GuestCode_Resolve(PE_ROOM_M0089I_80193618_VMA);
    unsigned epoch = PE_Port_StopEpoch();
    uint32_t result;
    CHECK(entry && entry->thunk, "%s: 80193618 has a native registry thunk", c->name);
    if (!entry || !entry->thunk) return 0;
    result = (uint32_t)PE_GuestCall("@park-m0089-full", PE_ROOM_M0089I_80193618_VMA, 3u,
                                  args[0], args[1], args[2], 0x5555u);
    CHECK(PE_Decomp_BoundaryCount() == 0, "%s: reached %u unresolved boundaries",
          c->name, PE_Decomp_BoundaryCount());
    CHECK(g_stub_order_count == 0 && g_stub_bootstrap_invocations == 0,
          "%s: reached bootstrap/stub code", c->name);
    CHECK(PE_Port_StopEpoch() == epoch && !PE_Port_ShouldStop(), "%s: requested a stop", c->name);
    return result;
}

static void test_cases(void)
{
    unsigned i;
    unsigned long pert_controls = 0;
    for (i = 0; i < N(M9_cases); i++) {
        const M9Case *c = &M9_cases[i];
        unsigned bad, stray;
        uint32_t ret, rnext;
        build_expected(c);
        seed_case(c);
        ret = call_body(c, c->args);
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
            pbad = (unsigned)(call_body(c, args) != c->v0);
            pbad += compare_all(c, 0, skip);
            CHECK(pbad > 0, "%s: perturbation (kind %u key %X xor %X) not detected", c->name, c->pert_kind,
                  c->pert_key, c->pert_xor);
            pert_controls++;
        }
        printf("  %-30s mismatches=%u stray=%u%s\n", c->name,
               bad, stray, c->pert_kind ? " perturbation checked" : "");
    }
    CHECK(pert_controls >= N(M9_cases) / 2u, "too few perturbation controls ran (%lu)", pert_controls);
    printf("park m0089 full: %lu input-perturbation controls\n", pert_controls);
}

static const M9Case *find_case(const char *name)
{
    unsigned i;
    for (i = 0; i < N(M9_cases); i++) if (!strcmp(M9_cases[i].name, name)) return &M9_cases[i];
    fprintf(stderr, "FAIL: case %s missing from the header\n", name);
    exit(1);
}

/* Comparer negative controls: a deliberately wrong word / scratch byte / GTE register must be detected. */
static void test_negative_control(void)
{
    const M9Case *c = find_case("init_flag_byte1");
    const M9Case *d = find_case("draw_t10");
    unsigned bad;
    build_expected(c);
    seed_case(c);
    (void)call_body(c, c->args);
    CHECK(compare_all(c, 0, 0) == 0, "negative control baseline");
    s_tamper_addr = M9_O + 0x8u; s_tamper_xor = 0x80u;
    bad = compare_all(c, 0, 0);
    s_tamper_addr = 0; s_tamper_xor = 0;
    CHECK(bad == 1, "negative control: tampered word not detected (%u)", bad);
    build_expected(d);
    seed_case(d);
    (void)call_body(d, d->args);
    CHECK(compare_all(d, 0, 0) == 0, "draw negative control baseline");
    s_tamper_scratch = 0x30u;                            /* 800C6FA0 scratch word */
    bad = compare_all(d, 0, 0);
    s_tamper_scratch = 0xFFFFFFFFu;
    CHECK(bad == 1, "negative control: tampered scratch byte not detected (%u)", bad);
    s_tamper_gte = 25u;                                  /* MAC1 */
    bad = compare_all(d, 0, 0);
    s_tamper_gte = 0xFFFFFFFFu;
    CHECK(bad == 1, "negative control: tampered GTE register not detected (%u)", bad);
}

/* The pool callback is stored as a guest address, and the meshes as the archive guest addresses. */
static void test_guest_addresses(void)
{
    const M9Case *c = find_case("init_flag_byte1");
    build_expected(c);
    seed_case(c);
    (void)call_body(c, c->args);
    CHECK(PE_LoadU32(PE_LoadU32(PE_LoadU32(0x800F33E0u) + 8u) + 8u) == PE_ROOM_M0089I_POOL_CALLBACK_80193200,
          "cmd 0 stores guest address 80193200 as the pool callback");
    CHECK(PE_LoadU32(0x8019416Cu) == PE_LoadU32(0x800B0E64u) + 0x200u, "cmd 0 stores the first archive mesh");
    CHECK(PE_LoadU32(0x80194168u) == PE_LoadU32(0x800B0E64u) + 0x800u, "cmd 0 stores the second archive mesh");
}

/* Identity of the loaded room code against the census values; 8018F1CC is data. */
static void test_identity(void)
{
    const M9Case *c = find_case("init_flag_byte1");
    seed_case(c);
    CHECK(PE_GuestCode_Fnv1a(PE_ROOM_M0089I_80193618_VMA, PE_ROOM_M0089I_80193618_SIZE) ==
          PE_ROOM_M0089I_80193618_FNV1A, "80193618 fnv");
    CHECK(PE_GuestCode_Fnv1a(PE_ROOM_M0089I_DATA_8018F1CC, 0x18u) == 0xFD4EEC9Eu, "8018F1CC census row fnv");
    CHECK((PE_LoadU32(PE_ROOM_M0089I_DATA_8018F1CC + 4u) >> 26) == 0u &&
          (PE_LoadU32(PE_ROOM_M0089I_DATA_8018F1CC + 4u) & 63u) == 0x32u,
          "8018F1D0 is a reserved R3000 SPECIAL function (the row is data, not code)");
    CHECK(PE_GuestCode_Fnv1a(0x8018F1DCu, 8u) == 0xE5787985u, "8018F1DC return-0 stub fnv");
}

/* Registry-only coverage of all four newly native M0089I entries. These
 * independent constants are FNV-1a over the owner's extracted retail chunk,
 * room_m0089i_c2.bin at base 8018EFE8; they do not come from the registry.
 * Only 80193618 has full-callee fixture execution in this adapter. */
static void test_registry(void)
{
    static const struct {
        uint32_t vma, size, fingerprint;
    } bodies[] = {
        {0x801924F8u, 0x538u, 0xC9AC3D84u},
        {0x80192CD4u, 0x52Cu, 0xA7C1DF7Fu},
        {0x80193200u, 0x418u, 0xEB610AEEu},
        {PE_ROOM_M0089I_80193618_VMA, PE_ROOM_M0089I_80193618_SIZE,
         PE_ROOM_M0089I_80193618_FNV1A}
    };
    const M9Case *c = find_case("init_flag_byte1");
    unsigned i;
    for (i = 0; i < N(bodies); i++) {
        const PeGuestCodeEntry *entry;
        uint32_t at = bodies[i].vma + bodies[i].size - 8u;
        uint8_t saved;
        seed_case(c);
        CHECK(PE_GuestCode_Fnv1a(bodies[i].vma, bodies[i].size) == bodies[i].fingerprint,
              "%08X retail body fingerprint", bodies[i].vma);
        entry = PE_GuestCode_Resolve(bodies[i].vma);
        CHECK(entry && entry->thunk, "%08X registry accepts the exact native body", bodies[i].vma);
        if (entry) {
            CHECK(entry->vma == bodies[i].vma && entry->size == bodies[i].size,
                  "%08X registry whole-body bounds", bodies[i].vma);
            CHECK(entry->fingerprint == bodies[i].fingerprint,
                  "%08X registry body fingerprint", bodies[i].vma);
        }
        CHECK(PE_GuestCode_Resolve(bodies[i].vma + 4u) == NULL,
              "%08X registry refuses an interior address", bodies[i].vma);
        saved = PE_LoadU8(at);
        PE_StoreU8(at, saved ^ 1u);
        CHECK(PE_GuestCode_Resolve(bodies[i].vma) == NULL,
              "%08X registry refuses a changed body", bodies[i].vma);
        PE_StoreU8(at, saved);
        CHECK(PE_GuestCode_Resolve(bodies[i].vma) != NULL,
              "%08X registry accepts the restored body", bodies[i].vma);
    }
    CHECK(PE_GuestCode_Resolve(PE_ROOM_M0089I_DATA_8018F1CC) == NULL, "registry refuses the data row");
}

/* AVSZ3 is the fan's last GTE command. It must replace RTPT's FLAG result,
 * including when the new result is zero. Pin both helpers' register edges:
 * MAC0 overflow bits 16/15, OTZ saturation bit 18, and error-summary bit 31.
 * These synthetic values follow the retail-oracle AVSZ command contract
 * (pe_battle_hud_oracle.py), independently of the implementation helper. */
static void test_avsz_flags(void)
{
    static const struct {
        const char *name;
        unsigned quad;
        int scale;
        uint32_t z[4], mac0, otz, flag;
    } cases[] = {
        {"avsz3_clear", 0, 341, {100, 200, 300, 0}, 0x00031F38u, 49, 0},
        {"avsz3_mac_positive", 0, 32767, {65535, 65535, 65535, 0}, 0x7FFB8003u, 65535, 0x80050000u},
        {"avsz3_mac_negative", 0, -32768, {65535, 65535, 65535, 0}, 0x80018000u, 0, 0x80048000u},
        {"avsz3_otz_high", 0, 4096, {65535, 1, 0, 0}, 0x10000000u, 65535, 0x80040000u},
        {"avsz3_otz_low", 0, -1, {1, 0, 0, 0}, 0xFFFFFFFFu, 0, 0x80040000u},
        {"avsz3_otz_limit", 0, 4096, {65535, 0, 0, 0}, 0x0FFFF000u, 65535, 0},
        {"avsz4_clear", 1, 256, {100, 200, 300, 400}, 0x0003E800u, 62, 0},
        {"avsz4_mac_positive", 1, 32767, {65535, 65535, 65535, 65535}, 0xFFFA0004u, 65535, 0x80050000u},
        {"avsz4_mac_negative", 1, -32768, {65535, 65535, 65535, 65535}, 0x00020000u, 0, 0x80048000u},
        {"avsz4_otz_high", 1, 4096, {65535, 0, 0, 1}, 0x10000000u, 65535, 0x80040000u},
        {"avsz4_otz_low", 1, -1, {0, 0, 0, 1}, 0xFFFFFFFFu, 0, 0x80040000u},
        {"avsz4_otz_limit", 1, 4096, {0, 65535, 0, 0}, 0x0FFFF000u, 65535, 0}
    };
    unsigned i;
    for (i = 0; i < N(cases); i++) {
        PE_GTE_WriteCtrl(cases[i].quad ? 30u : 29u, (uint32_t)cases[i].scale);
        PE_GTE_WriteCtrl(31, 0xFFFFFFFFu);
        if (cases[i].quad) PE_GTE_AVSZ4(cases[i].z);
        else PE_GTE_AVSZ3(cases[i].z);
        CHECK(PE_GTE_ReadData(24) == cases[i].mac0, "%s: MAC0", cases[i].name);
        CHECK(PE_GTE_ReadData(7) == cases[i].otz, "%s: OTZ", cases[i].name);
        CHECK(PE_GTE_ReadCtrl(31) == cases[i].flag, "%s: FLAG got %08X expected %08X",
              cases[i].name, PE_GTE_ReadCtrl(31), cases[i].flag);
    }
}

int main(void)
{
    int loaded;
    PE_RamInit();
    PE_GuestCode_InstallGenerated();
    loaded = load_disc();
    if (loaded != 0) {
        fprintf(stderr, "park m0089 full: %s\n", loaded == -1 ? "SKIP (no owner disc/cache)" : "FAIL (fixture identity/allocation)");
        free(room_bytes); free(exe_bytes); free(exp_words); free(exp_masks);
        return loaded == -1 ? 77 : 1;
    }
    printf("park m0089 full: %u retail cases (fixtures, not room execution)\n", (unsigned)N(M9_cases));
    test_avsz_flags();
    test_identity();
    test_registry();
    test_cases();
    test_negative_control();
    test_guest_addresses();
    printf("park m0089 full: %d checks, %d failures\n", checks, failures);
    free(room_bytes); free(exe_bytes); free(exp_words); free(exp_masks);
    return failures ? 1 : 0;
}
