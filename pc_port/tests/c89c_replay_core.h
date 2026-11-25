/* Guest-RAM replay checker core for the bounded/resumable C89C adapter (pe_plat_vlc_frame_host_regs) against
 *   retail_mdec_vlc_bound_cases.h  (original's bounded/resume call chains, black box) and
 *   retail_c89c_real_sequence.h    (253 calls observed in real FMV playback).
 * Framework-free: used by the standalone tools/pe_c89c_replay_check.c and by the native suite (test_mdec_vlc.h).
 * The including file provides, before this one: pe_guest_ram.h, pe_plat/vlc_frame.h, retail_mdec_vlc_cases.h,
 * retail_mdec_vlc_bound_cases.h, retail_c89c_real_sequence.h, mdec_vlc_check_core.h (MdecVlcFnv), and
 *   C89CR_RESET()       fresh guest RAM + clears a pending stop
 *   C89CR_STOPPED()     nonzero when a named boundary stop is pending
 *   C89CR_STR_SECTOR(lba, raw) callback wrapper returning 1/0.                                                     */
#include <stdio.h>
#include <string.h>

typedef int (*C89CReadRaw)(void *ctx, uint32_t lba, uint8_t raw[2352]);

/* assemble the STR video frame whose first sector is `lba`; returns its byte size (0 on failure) and the next video lba */
static uint32_t C89CAssemble(C89CReadRaw rd, void *ctx, uint32_t lba, uint8_t *out, size_t cap, uint32_t *next)
{
    uint8_t raw[2352]; size_t n = 0; unsigned total = 0, index = 0;
    for (uint32_t l = lba; l < lba + 64u; l++) {
        if (!rd(ctx, l, raw)) return 0;
        if (!(raw[18] & 8u) || raw[24] != 0x60 || raw[25] != 1 || raw[26] != 1 || raw[27] != 0x80) continue;
        if (raw[28] != index) return 0;
        total = raw[30];
        if (n + 2016u > cap) return 0;
        memcpy(out + n, raw + 56, 2016u); n += 2016u; index++;
        if (index == total) {
            uint32_t size = (uint32_t)raw[36] | ((uint32_t)raw[37] << 8) | ((uint32_t)raw[38] << 16) | ((uint32_t)raw[39] << 24);
            *next = l + 1u;
            return size <= n ? size : 0;
        }
    }
    return 0;
}

static void C89CPlace(const uint8_t *frame, size_t n, uint32_t ptr)
{
    C89CR_RESET();
    memcpy(PE_Translate(ptr, n + 64u), frame, n);          /* RAM beyond the frame stays zero */
}

#define C89CR_FAIL(...) do { snprintf(err, errsz, __VA_ARGS__); return 0; } while (0)

/* Replays every bounded/resume chain.  Steps on which the original never exits (ret 0xFFFFFFFF) must end in a named
 * stop.  `known` steps (case name, bound, step index) are counted instead of failing; their list is below. */
static const struct { const char *name; uint32_t bound; uint32_t step; } C89CR_known[] = { {"updater-scenario", 0, 1}, {"updater-scenario", 1, 1}, {"updater-scenario", 3, 0}, {"updater-scenario", 4, 0}, {NULL, 0, 0} };

static int C89CIsKnown(const char *name, uint32_t bound, uint32_t step)
{
    for (unsigned i = 0; C89CR_known[i].name; i++)
        if (!strcmp(C89CR_known[i].name, name) && C89CR_known[i].bound == bound && C89CR_known[i].step == step) return 1;
    return 0;
}

static int C89CReplayBound(C89CReadRaw rd, void *ctx, char *err, size_t errsz, unsigned *nsteps, unsigned *nknown, unsigned *ncases)
{
    static uint8_t frame[64u * 2016u], zero[0x4E];
    static const struct { uint32_t idx, first; } realmap[] = { {4u, 101735u}, {351u, 106710u}, {269u, 153248u} };
    unsigned stepbase = 0;
    *nsteps = *nknown = *ncases = 0;
    for (unsigned c = 0; c < sizeof(MDECVLCB_case) / sizeof(MDECVLCB_case[0]); c++) {
        const char *name = MDECVLCB_case[c].name;
        uint32_t ptr = MDECVLCB_case[c].frame_ptr, size = MDECVLCB_case[c].size, a1 = 0x80160000u, prev_end = 0;
        const uint8_t *data = NULL;
        if (!strncmp(name, "real-frame-", 11)) {
            uint32_t want = (uint32_t)strtoul(name + 11, NULL, 10), first = 0, next, got;
            for (unsigned k = 0; k < 3u; k++) if (want == (k == 0 ? 3u : k == 1 ? 500u : 5000u)) first = realmap[k].first;
            got = C89CAssemble(rd, ctx, first, frame, sizeof(frame), &next);
            if (!first || got != size) C89CR_FAIL("%s: frame assembly", name);
            data = frame;
        } else if (!strncmp(name, "syn-", 4)) {
            for (unsigned k = 0; k < sizeof(MDECVLC_syn) / sizeof(MDECVLC_syn[0]); k++)
                if (!strcmp(MDECVLC_syn[k].name, name + 4)) { data = MDECVLC_syn[k].in; size = MDECVLC_syn[k].insize; }
            if (!data) C89CR_FAIL("%s: synthetic input missing", name);
        } else if (!strcmp(name, "updater-scenario")) {
            memset(zero, 0, sizeof(zero)); zero[0xC] = 0x34; zero[0xD] = 0x12;   /* word 0x1234 at +0xC, rest zero */
            data = zero; size = sizeof(zero);
        } else C89CR_FAIL("%s: unknown case", name);
        if (MdecVlcFnv(data, size) != MDECVLCB_case[c].in_hash)
            C89CR_FAIL("%s: input hash differs", name);
        C89CPlace(data, size, ptr);
        memset(PE_Translate(0x80160000u, 0x40000u), 0xCD, 0x40000u);       /* canary arenas (the chains use arena 1 and a second one) */
        memset(PE_Translate(0x80180000u, 0x20000u), 0xCD, 0x20000u);
        PE_StoreU32(0x8011EB8Cu, MDECVLCB_case[c].bound);
        for (unsigned j = 0; j < MDECVLCB_case[c].nsteps; j++) {
            const __typeof__(MDECVLCB_step[0]) *s = &MDECVLCB_step[stepbase + j];
            uint32_t a0r = 0, a1r = 0, a0 = s->resume ? 0u : ptr;
            int ret;
            PE_StoreU32(0x8011EB8Cu, MDECVLCB_case[c].bound);
            ret = pe_plat_vlc_frame_host_regs(a0, s->resume ? 0x80180000u : a1, 0x80130000u, 0u, &a0r, &a1r);
            if (s->ret == 0xFFFFFFFFu) {                       /* the original never exits: the adapter must stop, not return */
                if (!C89CR_STOPPED()) { if (C89CIsKnown(name, MDECVLCB_case[c].bound, j)) { (*nknown)++; break; } C89CR_FAIL("%s bound %u step %u: original never exits, adapter returned %d", name, (unsigned)MDECVLCB_case[c].bound, j, ret); }
                (*nsteps)++; break;
            }
            if (C89CR_STOPPED()) { if (C89CIsKnown(name, MDECVLCB_case[c].bound, j)) { (*nknown)++; break; } C89CR_FAIL("%s bound %u step %u: adapter stopped (original returned %u)", name, (unsigned)MDECVLCB_case[c].bound, j, (unsigned)s->ret); }
            if ((uint32_t)ret != s->ret || a0r - ptr != s->a0_rel || a1r - a1 != s->a1_rel ||
                PE_LoadU32(0x8011EBB4u) - a1 != s->end_word_rel ||
                s->added != s->a1_rel - prev_end ||
                MdecVlcFnv(PE_TranslateConst(a1 + prev_end, s->added), s->added) != s->hash) {
                if (C89CIsKnown(name, MDECVLCB_case[c].bound, j)) { (*nknown)++; break; }
                C89CR_FAIL("%s bound %u step %u: ret %d/%u a0' %u/%u a1' %u/%u end %u/%u", name, (unsigned)MDECVLCB_case[c].bound, j, ret, (unsigned)s->ret,
                           (unsigned)(a0r - ptr), (unsigned)s->a0_rel, (unsigned)(a1r - a1), (unsigned)s->a1_rel, (unsigned)(PE_LoadU32(0x8011EBB4u) - a1), (unsigned)s->end_word_rel);
            }
            prev_end = s->a1_rel; (*nsteps)++;
        }
        stepbase += MDECVLCB_case[c].nsteps; (*ncases)++;
    }
    return 1;
}

/* The 253 calls of real playback: frames 1.. of the first movie (LBA 189742), each called once with its recorded
 * arguments (a0 = ring pointer, a1 = arena) and the bound word at its shipped value. */
static int C89CReplayReal(C89CReadRaw rd, void *ctx, char *err, size_t errsz, unsigned *ncalls)
{
    static uint8_t frame[64u * 2016u];
    uint32_t lba = 189742u, next;
    *ncalls = 0;
    for (unsigned i = 0; i < sizeof(C89CREAL_seq) / sizeof(C89CREAL_seq[0]); i++) {
        uint32_t size = C89CAssemble(rd, ctx, lba, frame, sizeof(frame), &next), a0r = 0, a1r = 0;
        int ret;
        if (!size) C89CR_FAIL("real call %u: frame assembly at lba %u", i, (unsigned)lba);
        lba = next;
        C89CPlace(frame, size, C89CREAL_seq[i].a0);
        PE_StoreU32(0x8011EB8Cu, C89CREAL_seq[i].bound_in);
        memset(PE_Translate(C89CREAL_seq[i].a1, 0xFA00u), 0xCD, 0xFA00u);
        ret = pe_plat_vlc_frame_host_regs(C89CREAL_seq[i].a0, C89CREAL_seq[i].a1, C89CREAL_seq[i].a2, C89CREAL_seq[i].a3, &a0r, &a1r);
        if (C89CR_STOPPED() || (uint32_t)ret != C89CREAL_seq[i].ret) C89CR_FAIL("real call %u: stop or ret %d", i, ret);
        if (PE_LoadU32(0x8011EB8Cu) != C89CREAL_seq[i].bound_out) C89CR_FAIL("real call %u: bound word changed", i);
        if (PE_LoadU32(0x8011EBB4u) != C89CREAL_seq[i].end_word) C89CR_FAIL("real call %u: end word %08X != %08X", i, (unsigned)PE_LoadU32(0x8011EBB4u), (unsigned)C89CREAL_seq[i].end_word);
        if (PE_LoadU32(C89CREAL_seq[i].a0) != C89CREAL_seq[i].header) C89CR_FAIL("real call %u: frame header word differs from the recorded run", i);
        if (a1r - C89CREAL_seq[i].a1 > 0xFA00u) C89CR_FAIL("real call %u: output overruns the 64000-byte arena spacing", i);
        (*ncalls)++;
    }
    return 1;
}
