/* Opt-in SPU INTERPOLATION (pe_plat/enhance_host.h, platform/plat_enhance.c,
 * platform/pe_spu.c): the console's 4-point Gaussian (retail, default) or a
 * 4-point cubic through the same four samples.
 *
 * Voices are real SPU voices: ADPCM blocks in SPU RAM, played through the
 * register file and PE_Spu_Render.  Golden FNV-1a hashes pin the rendered
 * buffers.  The Gaussian one was first taken from the unmodified mixer
 * (pc_port/platform/pe_spu.c of 283d4677); since the 2026-10-29 SPU envelope
 * and ADPCM fidelity fixes both hashes are regression pins of the current
 * mixer, not an independent pre-option identity proof.  The kernel is
 * also checked against the Gaussian table itself (a ramp must interpolate the
 * same way in both) so the interval and the direction are pinned by data, not
 * by symmetry. */
#include "pe_plat/enhance_host.h"
#include "pe_spu.h"
#include "pe_spu_dma.h"

/* Two periods of a 56-sample wave (6 sin(t) + 2 sin(5t + 0.7), rounded to a signed
 * nibble) as four ADPCM blocks, filter 0 shift 1 (nibble << 11): loop start,
 * body, body, loop end + repeat.  Generated, not captured from the mixer. */
static const uint8_t enhx_wave[64] = {
    0x01, 0x04, 0x31, 0x33, 0x23, 0x22, 0x43, 0x75, 0x77, 0x77, 0x45, 0x33, 0x43, 0x55, 0x45, 0x02,
    0x01, 0x00, 0xDF, 0xDD, 0xED, 0xEE, 0xCD, 0x9B, 0x88, 0x98, 0xCB, 0xDD, 0xCD, 0xBB, 0xCB, 0x0E,
    0x01, 0x00, 0x31, 0x33, 0x23, 0x22, 0x43, 0x75, 0x77, 0x77, 0x45, 0x33, 0x43, 0x55, 0x45, 0x02,
    0x01, 0x03, 0xDF, 0xDD, 0xED, 0xEE, 0xCD, 0x9B, 0x88, 0x98, 0xCB, 0xDD, 0xCD, 0xBB, 0xCB, 0x0E};

#define ENHX_SPU_FRAMES 4096

/* Four looping voices at four pitches (1.0, 0.667, 1.5, 0.125 of the base rate),
 * rendered ENHX_SPU_FRAMES frames with SPU INTERPOLATION = interp.  Returns the
 * FNV-1a hash of the interleaved int16 output and leaves it in out[]. */
static uint64_t enhx_spu_render(int interp, int16_t *out)
{
    static const uint16_t pitch[4] = {0x1000, 0x0AAA, 0x1800, 0x0200};
    uint8_t *ram;
    ResetTestState();
    PE_Spu_SetActive(1);
    PE_Spu_Reset();
    PE_Spu_SetInterpolation(interp);
    ram = PE_SpuRam_Data();
    memcpy(ram + 0x1000, enhx_wave, sizeof enhx_wave);
    for (unsigned n = 0; n < 4; n++) {
        uint32_t r = n * 0x10u;
        PE_SpuRegister_StoreU16(r + 0x0, 0x3FFF);                 /* volume L */
        PE_SpuRegister_StoreU16(r + 0x2, 0x3FFF);                 /* volume R */
        PE_SpuRegister_StoreU16(r + 0x4, pitch[n]);
        PE_SpuRegister_StoreU16(r + 0x6, 0x1000 / 8);             /* start */
        PE_SpuRegister_StoreU16(r + 0x8, 0x000F);                 /* instant attack, sustain at the top */
        PE_SpuRegister_StoreU16(r + 0xA, 0x0000);
    }
    PE_SpuRegister_StoreU16(0x180, 0x3FFF);
    PE_SpuRegister_StoreU16(0x182, 0x3FFF);
    PE_SpuRegister_StoreU16(0x188, 0x000F);                       /* key on voices 0-3 */
    PE_Spu_Render(out, ENHX_SPU_FRAMES);
    PE_Spu_SetActive(0);
    PE_Spu_Reset();
    PE_Spu_SetInterpolation(PE_SPU_INTERP_GAUSSIAN);
    return enhx_fnv((const uint8_t *)out, (size_t)ENHX_SPU_FRAMES * 4u);
}

static void test_ENHX_interp_registry(void)
{
    char label[64];
    int o;
    TEST("ENHX_interp_registry");
    enh_isolate();
    unsetenv("PE_SPU_INTERPOLATION"); pe_enh_reset();
    o = pe_enh_option_index("spu_interpolation");
    ASSERT(o >= 0, "spu_interpolation is registered");
    {
        const PeEnhOption *op = pe_enh_option(o);
        ASSERT(op->category == PE_ENH_CAT_AUDIO && op->format == PE_ENH_FMT_NAMED && op->nvalues == 2 &&
               op->values[0] == 0 && op->names && strcmp(op->env, "PE_SPU_INTERPOLATION") == 0 &&
               op->credit && strcmp(op->credit, "the owner") == 0, "a named audio row, two values, retail first");
    }
    ASSERT(pe_enh_category_option(PE_ENH_CAT_AUDIO, 3) == pe_enh_option_index("sfx_volume") &&
           pe_enh_category_option(PE_ENH_CAT_AUDIO, 4) == o && pe_enh_category_option(PE_ENH_CAT_AUDIO, 6) == -1,
           "row 5 of the Audio page, after the four volumes");
    ASSERT((int)PE_ENH_SPU_GAUSSIAN == (int)PE_SPU_INTERP_GAUSSIAN && (int)PE_ENH_SPU_CUBIC == (int)PE_SPU_INTERP_CUBIC,
           "registry values are the mixer's enums");
    ASSERT(pe_enh_spu_interpolation() == PE_ENH_SPU_GAUSSIAN && pe_enh_option_is_retail(o) &&
           PE_Spu_Interpolation() == PE_SPU_INTERP_GAUSSIAN, "defaults to the retail Gaussian");
    ASSERT(access(enh_conf, F_OK) != 0, "queries create no config file");
    pe_enh_option_label(o, label, sizeof label);
    ASSERT(strcmp(label, "SPU INTERPOLATION: GAUSSIAN (RETAIL)") == 0, "label names the retail value");
    pe_enh_option_cycle(o);
    pe_enh_option_label(o, label, sizeof label);
    ASSERT(pe_enh_spu_interpolation() == PE_ENH_SPU_CUBIC && PE_Spu_Interpolation() == PE_SPU_INTERP_CUBIC &&
           strcmp(label, "SPU INTERPOLATION: CUBIC (4-POINT)") == 0, "A cycles to CUBIC and the mixer follows");
    pe_enh_option_cycle(o);
    ASSERT(pe_enh_spu_interpolation() == PE_ENH_SPU_GAUSSIAN && PE_Spu_Interpolation() == PE_SPU_INTERP_GAUSSIAN,
           "and wraps to GAUSSIAN");
    pe_enh_set_spu_interpolation(7);
    ASSERT(pe_enh_spu_interpolation() == PE_ENH_SPU_CUBIC, "set snaps to the largest listed value");
    pe_enh_set_spu_interpolation(-1);
    ASSERT(pe_enh_spu_interpolation() == PE_ENH_SPU_GAUSSIAN, "below the list snaps to retail");
    /* the mixer keeps the option across its own reset; the registry's reset restores retail */
    pe_enh_set_spu_interpolation(PE_ENH_SPU_CUBIC);
    PE_Spu_Reset();
    ASSERT(PE_Spu_Interpolation() == PE_SPU_INTERP_CUBIC, "PE_Spu_Reset keeps the host option");
    {
        char text[1024] = {0};
        FILE *fp = fopen(enh_conf, "r");
        size_t n_;
        ASSERT(fp != NULL, "the menu change wrote the config");
        n_ = fread(text, 1, sizeof text - 1, fp);
        text[n_] = 0; fclose(fp);
        ASSERT(strstr(text, "spu_interpolation=1\n") != NULL, "config line spu_interpolation=1");
    }
    pe_enh_reset();
    ASSERT(pe_enh_spu_interpolation() == PE_ENH_SPU_CUBIC && PE_Spu_Interpolation() == PE_SPU_INTERP_CUBIC,
           "persists across a reload and is applied to the mixer");
    setenv("PE_SPU_INTERPOLATION", "0", 1); pe_enh_reset();
    ASSERT(pe_enh_spu_interpolation() == PE_ENH_SPU_GAUSSIAN && PE_Spu_Interpolation() == PE_SPU_INTERP_GAUSSIAN,
           "the env override applies");
    unsetenv("PE_SPU_INTERPOLATION"); pe_enh_reset();
    ASSERT(pe_enh_spu_interpolation() == PE_ENH_SPU_CUBIC, "the env override is not written back");
    setenv("PE_SPU_INTERPOLATION", "smooth", 1); pe_enh_reset();
    ASSERT(pe_enh_spu_interpolation() == PE_ENH_SPU_GAUSSIAN, "an unparsable override means retail");
    unsetenv("PE_SPU_INTERPOLATION");
    unlink(enh_conf); pe_enh_reset();
    ASSERT(PE_Spu_Interpolation() == PE_SPU_INTERP_GAUSSIAN, "defaults again");
    PASS();
}

/* The kernel: exact for constants, ramps and parabolas; the Gaussian table's own ramp response pins the interval. */
static void test_ENHX_interp_cubic_kernel(void)
{
    uint64_t h = 1469598103934665603ull;
    uint32_t lcg = 12345u;
    TEST("ENHX_interp_cubic_kernel");
    {   /* phase 0 is the second tap; constants stay constant */
        const int16_t t[4] = {-1200, 777, 31000, -32768};
        ASSERT(PE_Spu_InterpolateCubic(t, 0) == 777, "phase 0 returns the second tap");
        for (int v = -32768; v <= 32767; v += 4369) {
            const int16_t c[4] = {(int16_t)v, (int16_t)v, (int16_t)v, (int16_t)v};
            for (unsigned ph = 0; ph < 4096; ph += 511) ASSERT(PE_Spu_InterpolateCubic(c, ph) == v, "a constant stays constant");
        }
    }
    {   /* a ramp: out = p1 + (p2 - p1) * phase / 4096; a parabola, 100 * x^2 through x = -1..2, is exact too */
        const int16_t ramp[4] = {-1500, -500, 500, 1500};
        const int16_t par[4] = {100, 0, 100, 400};
        for (unsigned ph = 0; ph < 4096; ph += 64) {
            int want = -500 + (int)((1000 * ph + 2048) / 4096);
            int d = PE_Spu_InterpolateCubic(ramp, ph) - want;
            int pexp = (int)((100ll * ph * ph + 8388608ll) / 16777216ll);
            int pd = PE_Spu_InterpolateCubic(par, ph) - pexp;
            ASSERT(d >= -1 && d <= 1, "a ramp interpolates linearly");
            ASSERT(pd >= -1 && pd <= 1, "a parabola is reproduced");
        }
    }
    {   /* the Gaussian table's own ramp response: the same interval, the same direction.  For zero-mean ramp taps
         * both kernels put the output on the line between the second and third tap; a cubic over the wrong pair
         * would be off by a whole step.  (The table's weights sum to 0.996, so allow a few LSB.) */
#include "pe_spu_gauss.inc"
        for (int step = 200; step <= 4000; step += 600) {
            const int16_t ramp[4] = {(int16_t)(-3 * step / 2), (int16_t)(-step / 2), (int16_t)(step / 2), (int16_t)(3 * step / 2)};
            for (unsigned ph = 0; ph < 4096; ph += 16) {
                int i = (int)((ph >> 4) & 0xFFu), s;
                int c, d;
                s  = (k_gauss[0x0FF - i] * ramp[0]) >> 15;
                s += (k_gauss[0x1FF - i] * ramp[1]) >> 15;
                s += (k_gauss[0x100 + i] * ramp[2]) >> 15;
                s += (k_gauss[0x000 + i] * ramp[3]) >> 15;
                c = PE_Spu_InterpolateCubic(ramp, ph);
                d = c - s;
                ASSERT(d >= -16 && d <= 16, "cubic and the Gaussian table agree on a ramp (same interval, same direction)");
            }
        }
    }
    {   /* overshoot is clamped to int16 */
        const int16_t t[4] = {32767, 32767, -32768, -32768};
        int lo = 0, hi = 0;
        for (unsigned ph = 0; ph < 4096; ph += 8) {
            int v = PE_Spu_InterpolateCubic(t, ph);
            ASSERT(v >= -32768 && v <= 32767, "within int16");
            lo |= v == -32768; hi |= v == 32767;
        }
        ASSERT(hi || lo, "the clamp is reached by a full-scale step");
    }
    {   /* a golden over 4096 pseudo-random tap sets and phases pins the arithmetic and the rounding */
        for (int i = 0; i < 4096; i++) {
            int16_t t[4];
            int v;
            for (int k = 0; k < 4; k++) { lcg = lcg * 1664525u + 1013904223u; t[k] = (int16_t)(lcg >> 16); }
            lcg = lcg * 1664525u + 1013904223u;
            v = PE_Spu_InterpolateCubic(t, (lcg >> 16) & 0xFFFu);
            h ^= (uint64_t)(uint16_t)v; h *= 1099511628211ull;
        }
        {
            static char msg[96];
            const uint64_t want = 0xc4a64b9adf22ab1bull;
            snprintf(msg, sizeof msg, "kernel golden: got %016llx want %016llx", (unsigned long long)h, (unsigned long long)want);
            ASSERT(h == want, msg);
        }
    }
    PASS();
}

static void test_ENHX_interp_defaults_identity(void)
{
    static int16_t a[2 * ENHX_SPU_FRAMES], b[2 * ENHX_SPU_FRAMES];
    uint64_t ha, hb;
    TEST("ENHX_interp_defaults_identity");
    enh_isolate();
    unsetenv("PE_SPU_INTERPOLATION"); pe_enh_reset();
    ASSERT(pe_enh_spu_interpolation() == PE_ENH_SPU_GAUSSIAN && PE_Spu_Interpolation() == PE_SPU_INTERP_GAUSSIAN,
           "default: the Gaussian");
    ha = enhx_spu_render(pe_enh_spu_interpolation(), a);
    hb = enhx_spu_render(PE_SPU_INTERP_GAUSSIAN, b);
    ASSERT(ha == hb && memcmp(a, b, sizeof a) == 0, "the default is the Gaussian, deterministically");
    {
        int nz = 0;
        for (int i = 0; i < 2 * ENHX_SPU_FRAMES; i++) nz |= a[i] != 0;
        ASSERT(nz, "the voices sound");
    }
    {
        static char msg[96];
        /* Refreshed 2026-10-29 at 51dfb7d8e after the 10-29 SPU envelope/ADPCM
         * fidelity commits (was 0x9b6e8bfb785f06cf from 283d4677): a regression
         * pin of this tree, not an independent pre-option identity proof. */
        const uint64_t want = 0xd1192de649de71a7ull;
        snprintf(msg, sizeof msg, "retail mixer render: got %016llx want %016llx", (unsigned long long)ha, (unsigned long long)want);
        ASSERT(ha == want, msg);
    }
    ASSERT(access(enh_conf, F_OK) != 0, "no config file was created");
    PASS();
}

static void test_ENHX_interp_render_golden(void)
{
    static int16_t g[2 * ENHX_SPU_FRAMES], c[2 * ENHX_SPU_FRAMES], c2[2 * ENHX_SPU_FRAMES];
    uint64_t hg, hc, hc2;
    long long diff = 0, mag = 0;
    TEST("ENHX_interp_render_golden");
    enh_isolate();
    hg = enhx_spu_render(PE_SPU_INTERP_GAUSSIAN, g);
    hc = enhx_spu_render(PE_SPU_INTERP_CUBIC, c);
    hc2 = enhx_spu_render(PE_SPU_INTERP_CUBIC, c2);
    ASSERT(hc == hc2, "the cubic render is deterministic");
    ASSERT(hc != hg && memcmp(g, c, sizeof g) != 0, "the cubic render differs from the Gaussian");
    for (int i = 0; i < 2 * ENHX_SPU_FRAMES; i++) {
        diff += g[i] > c[i] ? g[i] - c[i] : c[i] - g[i];
        mag += g[i] < 0 ? -g[i] : g[i];
    }
    ASSERT(mag > 0 && diff * 5 < mag, "the two renderings of the same voices stay within 20 % of each other (mean absolute difference)");
    {
        static char msg[96];
        const uint64_t want = 0x515efe08c65ff91full;   /* refreshed 2026-10-29 (was 0x4b2dee5f792eb6db) */
        snprintf(msg, sizeof msg, "cubic render: got %016llx want %016llx", (unsigned long long)hc, (unsigned long long)want);
        ASSERT(hc == want, msg);
    }
    ASSERT(PE_Spu_Interpolation() == PE_SPU_INTERP_GAUSSIAN, "the helper leaves the mixer on the Gaussian");
    PASS();
}

/* OPTIONS > AUDIO row 5 in the dev menu. */
static void test_ENHX_interp_dev_menu(void)
{
    static uint8_t fb[320 * 240 * 3];
    char label[96];
    TEST("ENHX_interp_dev_menu");
    enh_isolate();
    unsetenv("PE_SPU_INTERPOLATION"); pe_enh_reset();
    setenv("PE_DEV_MENU", "1", 1);
    pe_plat_cheats_reset();
    (void)pe_plat_cheats_key(PE_CHEAT_KEY_DEV_MENU, 1);
    for (int i = 0; i < 6; i++) (void)pe_plat_cheats_key(PE_CHEAT_KEY_DOWN, 1);   /* OPTIONS (row 7) */
    (void)pe_plat_cheats_key(PE_CHEAT_KEY_ENTER, 1);                             /* -> OPTIONS */
    (void)pe_plat_cheats_key(PE_CHEAT_KEY_DOWN, 1);
    (void)pe_plat_cheats_key(PE_CHEAT_KEY_DOWN, 1);                              /* AUDIO */
    (void)pe_plat_cheats_key(PE_CHEAT_KEY_ENTER, 1);                             /* -> AUDIO */
    for (int i = 0; i < 4; i++) (void)pe_plat_cheats_key(PE_CHEAT_KEY_DOWN, 1);   /* row 5: SPU INTERPOLATION */
    ASSERT(pe_plat_cheats_overlay(fb, 320, 240) != fb, "audio page draws");
    (void)pe_plat_cheats_key(PE_CHEAT_KEY_ENTER, 1);
    ASSERT(pe_enh_spu_interpolation() == PE_ENH_SPU_CUBIC && PE_Spu_Interpolation() == PE_SPU_INTERP_CUBIC &&
           pe_enh_master_volume() == 100, "row 5 cycles the interpolation and nothing else");
    (void)pe_plat_cheats_key(PE_CHEAT_KEY_ENTER, 1);
    ASSERT(pe_enh_spu_interpolation() == PE_ENH_SPU_GAUSSIAN, "and back to the Gaussian");
    (void)pe_plat_cheats_key(PE_CHEAT_KEY_DOWN, 1);  /* row 5 is SPU; row 6 is MONO AUDIO */
    (void)pe_plat_cheats_key(PE_CHEAT_KEY_DOWN, 1);                              /* wraps to row 1: MASTER VOLUME */
    (void)pe_plat_cheats_key(PE_CHEAT_KEY_ENTER, 1);
    ASSERT(pe_enh_master_volume() == 90, "the page wraps from row 6 to row 1");
    pe_enh_set_master_volume(100);
    for (int n = 0; n < pe_enh_category_count(PE_ENH_CAT_AUDIO); n++) {
        pe_enh_option_label(pe_enh_category_option(PE_ENH_CAT_AUDIO, n), label, sizeof label);
        ASSERT(strlen(label) + 2 <= 50, "every audio row fits the 320-pixel overlay (6 px per character)");
    }
    (void)pe_plat_cheats_key(PE_CHEAT_KEY_DEV_MENU, 1);
    unsetenv("PE_DEV_MENU");
    pe_plat_cheats_reset();
    unlink(enh_conf); pe_enh_reset();
    PASS();
}

static void test_ENHX_audio_all(void)
{
    test_ENHX_interp_registry();
    test_ENHX_interp_cubic_kernel();
    test_ENHX_interp_defaults_identity();
    test_ENHX_interp_render_golden();
    test_ENHX_interp_dev_menu();
}
