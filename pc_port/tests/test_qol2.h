/* QoL lane: BRIGHTNESS (Graphics), MONO AUDIO (Audio) and the F12 screenshot
 * writer.  platform/plat_enhance.c (registry rows and accessors),
 * platform/host_qol.h (the pure LUT and mono rules), platform/host_png.c (the
 * PNG writer and the screenshot folder).
 *
 * The window and audio backends need a display or a device, so what runs
 * here is every pure piece they call, the registry and config, the dev-menu
 * rows, and a PNG round trip checked by a reader written for this test.  The
 * backends' wiring is covered by the runtime evidence (docs/port/ENHANCEMENTS.md). */
#include "plat_enhance.h"
#include "host_qol.h"
#include "host_png.h"
#include "pe_plat/cheats.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define QOL2_W 320
#define QOL2_H 240
#define QOL2_PNG_CAP (QOL2_W * QOL2_H * 3 + 4096)

static uint8_t qol2_rgb[QOL2_W * QOL2_H * 3];
static uint8_t qol2_back[QOL2_W * QOL2_H * 3];
static uint8_t qol2_png[QOL2_PNG_CAP];
static uint8_t qol2_fb[QOL2_W * QOL2_H * 3];
static char qol2_root[512];

static uint32_t qol2_be32(const uint8_t *p)
{
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) | ((uint32_t)p[2] << 8) | (uint32_t)p[3];
}

/* A reader for what HostPng_Encode writes: the signature, every chunk's CRC,
 * the IHDR fields, one zlib stream of stored blocks with a checked Adler-32,
 * filter type 0 on every row, and the IEND.  Returns 1 and the RGB bytes. */
static int qol2_decode(const uint8_t *png, size_t n, int *w_out, int *h_out,
                       uint8_t *rgb, size_t rgb_cap, int *idat_len_out)
{
    static const uint8_t sig[8] = {0x89, 'P', 'N', 'G', 0x0D, 0x0A, 0x1A, 0x0A};
    static uint8_t raw[QOL2_W * QOL2_H * 3 + QOL2_H + 64];
    size_t p = 8, raw_len = 0, zpos, idat_len = 0;
    const uint8_t *idat = NULL;
    int w = 0, h = 0, saw_ihdr = 0, saw_iend = 0;
    if (n < 8 || memcmp(png, sig, 8) != 0) return 0;
    while (p + 12 <= n) {
        uint32_t len = qol2_be32(png + p);
        const uint8_t *type = png + p + 4, *data = png + p + 8;
        if (p + 12 + (size_t)len > n) return 0;
        if (HostPng_Crc32(type, 4u + len) != qol2_be32(data + len)) return 0;
        if (memcmp(type, "IHDR", 4) == 0) {
            if (len != 13 || saw_ihdr) return 0;
            w = (int)qol2_be32(data);
            h = (int)qol2_be32(data + 4);
            if (data[8] != 8 || data[9] != 2 || data[10] != 0 || data[11] != 0 || data[12] != 0) return 0;
            saw_ihdr = 1;
        } else if (memcmp(type, "IDAT", 4) == 0) {
            if (!idat) { idat = data; idat_len = len; }   /* one IDAT chunk expected */
        } else if (memcmp(type, "IEND", 4) == 0) {
            if (len != 0 || p + 12 != n) return 0;
            saw_iend = 1;
        }
        p += 12u + len;
    }
    if (!saw_ihdr || !saw_iend || !idat || w <= 0 || h <= 0) return 0;
    if (idat_len < 6 || idat[0] != 0x78 || idat[1] != 0x01 || ((idat[0] << 8) | idat[1]) % 31 != 0) return 0;
    zpos = 2;
    for (;;) {
        uint8_t hdr;
        uint16_t blen, nlen;
        if (zpos + 5 > idat_len) return 0;
        hdr = idat[zpos];
        if ((hdr & 0x06u) != 0u) return 0;          /* BTYPE must be 00 (stored) */
        blen = (uint16_t)(idat[zpos + 1] | (idat[zpos + 2] << 8));
        nlen = (uint16_t)(idat[zpos + 3] | (idat[zpos + 4] << 8));
        if ((uint16_t)~blen != nlen) return 0;
        if (zpos + 5 + blen > idat_len || raw_len + blen > sizeof raw) return 0;
        memcpy(raw + raw_len, idat + zpos + 5, blen);
        raw_len += blen;
        zpos += 5u + blen;
        if (hdr & 1u) break;                         /* BFINAL */
    }
    if (zpos + 4 != idat_len) return 0;
    if (qol2_be32(idat + zpos) != HostPng_Adler32(raw, raw_len)) return 0;
    if (raw_len != (size_t)h * (1u + 3u * (size_t)w)) return 0;
    if ((size_t)w * (size_t)h * 3u > rgb_cap) return 0;
    for (int y = 0; y < h; y++) {
        const uint8_t *row = raw + (size_t)y * (1u + 3u * (size_t)w);
        if (row[0] != 0) return 0;                   /* filter type 0 */
        memcpy(rgb + (size_t)y * (size_t)w * 3u, row + 1, (size_t)w * 3u);
    }
    *w_out = w;
    *h_out = h;
    if (idat_len_out) *idat_len_out = (int)idat_len;
    return 1;
}

static void qol2_fill_pattern(uint8_t *rgb, int w, int h)
{
    for (int i = 0; i < w * h * 3; i++) rgb[i] = (uint8_t)(i * 7 + i / 3 + (i >> 9));
}

/* Per-process scratch folder under TMPDIR (or /tmp): never the owner's dirs. */
static void qol2_setup_root(void)
{
    const char *t = getenv("TMPDIR");
    snprintf(qol2_root, sizeof qol2_root, "%s/pe_qol2_test_%d",
             t && *t ? t : "/tmp", (int)getpid());
}

static void test_QOL2_registry(void)
{
    char label[96];
    int b = pe_enh_option_index("brightness"), m = pe_enh_option_index("mono");
    const PeEnhOption *ob, *om;
    TEST("QOL2_registry");
    enh_isolate();
    unsetenv("PE_BRIGHTNESS");
    unsetenv("PE_MONO");
    pe_enh_reset();
    ASSERT(b >= 0 && m >= 0, "brightness and mono are registered");
    ob = pe_enh_option(b);
    om = pe_enh_option(m);
    ASSERT(ob->category == PE_ENH_CAT_GRAPHICS && ob->format == PE_ENH_FMT_PERCENT &&
           ob->nvalues == 6 && ob->values[0] == 100, "brightness: graphics, percent, six steps, retail 100 first");
    ASSERT(strcmp(ob->id, "brightness") == 0 && strcmp(ob->label, "BRIGHTNESS") == 0 &&
           strcmp(ob->env, "PE_BRIGHTNESS") == 0 && ob->credit && strcmp(ob->credit, "the owner") == 0,
           "brightness: config key, label, env override, credit");
    ASSERT(om->category == PE_ENH_CAT_AUDIO && om->format == PE_ENH_FMT_TOGGLE &&
           om->nvalues == 2 && om->values[0] == 0, "mono: audio, toggle, stereo (0) first");
    ASSERT(strcmp(om->id, "mono") == 0 && strcmp(om->label, "MONO AUDIO") == 0 &&
           strcmp(om->env, "PE_MONO") == 0, "mono: config key, label, env override");
    ASSERT(pe_enh_brightness_pct() == 100 && pe_enh_mono() == 0, "both default to retail (100 %, stereo)");
    ASSERT(pe_enh_option_is_retail(b) && pe_enh_option_is_retail(m), "both report retail");
    pe_enh_option_label(b, label, sizeof label);
    ASSERT(strcmp(label, "BRIGHTNESS: 100% (RETAIL)") == 0, "brightness row label at retail");
    pe_enh_option_label(m, label, sizeof label);
    ASSERT(strcmp(label, "MONO AUDIO: OFF") == 0, "mono row label when off");
    ASSERT(pe_enh_category_option(PE_ENH_CAT_GRAPHICS, pe_enh_category_count(PE_ENH_CAT_GRAPHICS) - 1) == b,
           "brightness is the last Graphics row");
    ASSERT(pe_enh_category_option(PE_ENH_CAT_AUDIO, pe_enh_category_count(PE_ENH_CAT_AUDIO) - 1) == m,
           "mono is the last Audio row");
    ASSERT(pe_enh_category_option(PE_ENH_CAT_CONTROLS, 0) == pe_enh_option_index("analog_stick"),
           "analog stick stays the controls row");
    unlink(enh_conf);
    pe_enh_reset();
    PASS();
}

static void test_QOL2_persistence(void)
{
    char line[128];
    FILE *f;
    int found_b = 0, found_m = 0;
    TEST("QOL2_persistence");
    enh_isolate();
    unsetenv("PE_BRIGHTNESS");
    unsetenv("PE_MONO");
    pe_enh_reset();
    pe_enh_set_brightness_pct(120);
    pe_enh_set_mono(1);
    ASSERT(pe_enh_brightness_pct() == 120 && pe_enh_mono() == 1, "menu setters take effect");
    f = fopen(enh_conf, "r");
    ASSERT(f != NULL, "a menu change writes the config");
    while (fgets(line, sizeof line, f)) {
        if (strcmp(line, "brightness=120\n") == 0) found_b = 1;
        if (strcmp(line, "mono=1\n") == 0) found_m = 1;
    }
    fclose(f);
    ASSERT(found_b && found_m, "config holds brightness=120 and mono=1");
    pe_enh_reset();
    ASSERT(pe_enh_brightness_pct() == 120 && pe_enh_mono() == 1, "both persist across a reload");
    pe_enh_set_brightness_pct(95);
    ASSERT(pe_enh_brightness_pct() == 90, "95 snaps to the listed 90");
    pe_enh_set_brightness_pct(300);
    ASSERT(pe_enh_brightness_pct() == 130, "300 snaps to the largest step, 130");
    pe_enh_set_brightness_pct(50);
    ASSERT(pe_enh_brightness_pct() == 80, "50 snaps to the smallest step, 80");
    pe_enh_set_mono(7);
    ASSERT(pe_enh_mono() == 1, "mono is a toggle: any non-zero is ON");
    /* env overrides win for the run and are not written back */
    pe_enh_set_brightness_pct(80);
    setenv("PE_BRIGHTNESS", "110", 1);
    setenv("PE_MONO", "0", 1);
    pe_enh_reset();
    ASSERT(pe_enh_brightness_pct() == 110 && pe_enh_mono() == 0, "PE_BRIGHTNESS and PE_MONO override the file");
    pe_enh_set_mono(1);   /* a menu save writes the file's own brightness, not the env value */
    f = fopen(enh_conf, "r");
    found_b = found_m = 0;
    if (f) {
        while (fgets(line, sizeof line, f)) {
            if (strcmp(line, "brightness=80\n") == 0) found_b = 1;
            if (strcmp(line, "brightness=110\n") == 0) found_b = -1;
            if (strcmp(line, "mono=1\n") == 0) found_m = 1;
        }
        fclose(f);
    }
    ASSERT(found_b == 1 && found_m == 1, "the save writes the file's brightness (80), never the env value");
    unsetenv("PE_BRIGHTNESS");
    unsetenv("PE_MONO");
    pe_enh_reset();
    ASSERT(pe_enh_brightness_pct() == 80 && pe_enh_mono() == 1, "without the env the file values apply");
    setenv("PE_BRIGHTNESS", "loud", 1);
    pe_enh_reset();
    ASSERT(pe_enh_brightness_pct() == 100, "a non-numeric override falls back to retail (100), as documented");
    unsetenv("PE_BRIGHTNESS");
    unlink(enh_conf);
    pe_enh_reset();
    ASSERT(pe_enh_brightness_pct() == 100 && pe_enh_mono() == 0, "no config and no env is retail again");
    PASS();
}

static void test_QOL2_brightness_lut(void)
{
    uint8_t lut[256], img[3 * 2 * 4];
    TEST("QOL2_brightness_lut");
    ASSERT(HostQol_BrightnessLut(lut, 100) == 0, "retail 100 builds no table (no pass)");
    ASSERT(HostQol_BrightnessLut(lut, 0) == 0 && HostQol_BrightnessLut(lut, -5) == 0,
           "a non-positive percentage builds no table");
    ASSERT(HostQol_BrightnessLut(lut, 110) == 1, "110 builds a table");
    ASSERT(lut[0] == 0 && lut[1] == 1 && lut[5] == 6 && lut[100] == 110 && lut[255] == 255,
           "110: round half up (5.5 -> 6), black stays black, clips at 255");
    ASSERT(HostQol_BrightnessLut(lut, 90) == 1 && lut[5] == 5 && lut[100] == 90 && lut[255] == 230,
           "90: 4.5 -> 5, 229.5 -> 230");
    ASSERT(HostQol_BrightnessLut(lut, 80) == 1 && lut[255] == 204 && lut[100] == 80,
           "80: 255 x 0.8 = 204");
    ASSERT(HostQol_BrightnessLut(lut, 130) == 1 && lut[100] == 130 && lut[255] == 255,
           "130: the table clips above 196");
    {
        int ok = 1;
        static const int steps[] = {110, 120, 130, 90, 80};
        for (int s = 0; s < 5; s++) {
            (void)HostQol_BrightnessLut(lut, steps[s]);
            if (lut[0] != 0) ok = 0;
            for (int i = 0; i < 255; i++) if (lut[i] > lut[i + 1]) ok = 0;
        }
        ASSERT(ok, "every step keeps black at 0 and is non-decreasing");
    }
    /* the registry's own steps are the ones the table accepts */
    {
        int b = pe_enh_option_index("brightness"), ok = 1;
        for (int k = 0; k < pe_enh_option(b)->nvalues; k++)
            if (HostQol_BrightnessLut(lut, pe_enh_option(b)->values[k]) != (pe_enh_option(b)->values[k] != 100)) ok = 0;
        ASSERT(ok, "each registry step builds a table, retail builds none");
    }
    /* LutRect touches only its rectangle; alpha bytes are left alone */
    (void)HostQol_BrightnessLut(lut, 110);
    for (int i = 0; i < 3 * 2; i++) {
        img[i * 4 + 0] = img[i * 4 + 1] = img[i * 4 + 2] = 100;
        img[i * 4 + 3] = 0x7F;
    }
    HostQol_LutRect(img, 3, 4, 1, 0, 2, 2, lut);
    ASSERT(img[0] == 100 && img[1] == 100 && img[2] == 100 && img[3] == 0x7F,
           "the column left of the rectangle is untouched");
    ASSERT(img[4] == 110 && img[5] == 110 && img[6] == 110 && img[7] == 0x7F &&
           img[(1 * 3 + 2) * 4] == 110 && img[(1 * 3 + 2) * 4 + 3] == 0x7F,
           "the rectangle is brightened, alpha stays 0x7F");
    {
        uint8_t px[4] = {0, 0, 0, 9};
        HostQol_LutPixels(px, 1, 4, lut);
        ASSERT(px[0] == 0 && px[1] == 0 && px[2] == 0 && px[3] == 9, "black and alpha survive the table");
    }
    PASS();
}

static void test_QOL2_mono_rule(void)
{
    int16_t s[8] = {1, 0, -1, 0, 3, 4, -3, -4};
    int16_t d[2] = {0, 0};
    int16_t src[2] = {10, 21};
    TEST("QOL2_mono_rule");
    ASSERT(HostQol_MonoSample(1, 0) == 1, "(1, 0) -> 1: the half rounds away from zero");
    ASSERT(HostQol_MonoSample(-1, 0) == -1, "(-1, 0) -> -1");
    ASSERT(HostQol_MonoSample(3, 4) == 4 && HostQol_MonoSample(-3, -4) == -4, "(3, 4) -> 4, (-3, -4) -> -4");
    ASSERT(HostQol_MonoSample(1, 2) == 2 && HostQol_MonoSample(-1, -2) == -2, "(1, 2) -> 2 (1.5 rounds up in magnitude)");
    ASSERT(HostQol_MonoSample(32767, 32767) == 32767 && HostQol_MonoSample(-32768, -32768) == -32768,
           "the extremes stay in int16");
    ASSERT(HostQol_MonoSample(32767, -32768) == -1 && HostQol_MonoSample(100, -101) == -1,
           "opposite signs average toward zero");
    HostQol_Downmix(s, s, 4);   /* in place, as the backends call it */
    ASSERT(s[0] == 1 && s[1] == 1 && s[2] == -1 && s[3] == -1 && s[4] == 4 && s[5] == 4 &&
           s[6] == -4 && s[7] == -4, "in place: both channels get the mono sample");
    HostQol_Downmix(d, src, 1);
    ASSERT(d[0] == 16 && d[1] == 16 && src[0] == 10 && src[1] == 21, "out of place: the source is unchanged");
    PASS();
}

static void test_QOL2_png_checksums(void)
{
    TEST("QOL2_png_checksums");
    ASSERT(HostPng_Crc32((const uint8_t *)"123456789", 9) == 0xCBF43926u, "CRC-32 of 123456789 is CBF43926");
    ASSERT(HostPng_Crc32((const uint8_t *)"", 0) == 0u, "CRC-32 of nothing is 0");
    ASSERT(HostPng_Adler32((const uint8_t *)"Wikipedia", 9) == 0x11E60398u, "Adler-32 of Wikipedia is 11E60398");
    ASSERT(HostPng_Adler32((const uint8_t *)"", 0) == 1u, "Adler-32 of nothing is 1");
    ASSERT(HostPng_EncodedSize(0, 240) == 0 && HostPng_EncodedSize(320, 0) == 0 && HostPng_EncodedSize(16385, 1) == 0,
           "a size outside 1..16384 has no encoding");
    ASSERT(HostPng_EncodedSize(320, 240) == 230723u,
           "320x240: 8 signature + 25 IHDR + (12 + 230666) IDAT + 12 IEND");
    PASS();
}

static void test_QOL2_png_round_trip(void)
{
    int w = 0, h = 0, idat = 0;
    size_t n;
    TEST("QOL2_png_round_trip");
    qol2_fill_pattern(qol2_rgb, QOL2_W, QOL2_H);
    n = HostPng_Encode(qol2_png, sizeof qol2_png, qol2_rgb, QOL2_W, QOL2_H);
    ASSERT(n == HostPng_EncodedSize(QOL2_W, QOL2_H) && n == 230723u, "320x240 encodes to the computed size");
    ASSERT(HostPng_Encode(qol2_png, 1000, qol2_rgb, QOL2_W, QOL2_H) == 0, "a short buffer is refused");
    ASSERT(qol2_decode(qol2_png, n, &w, &h, qol2_back, sizeof qol2_back, &idat),
           "structure: signature, CRCs, IHDR, zlib header, stored blocks, Adler-32, filter 0, IEND");
    ASSERT(w == QOL2_W && h == QOL2_H, "IHDR width and height");
    ASSERT(idat == 230666, "the IDAT length field is the zlib stream length");
    ASSERT(memcmp(qol2_back, qol2_rgb, sizeof qol2_rgb) == 0, "decoded pixels equal the frame, byte for byte");
    /* a second shape: one pixel, and a width that is not a multiple of anything */
    {
        uint8_t one[3] = {0x12, 0x34, 0x56}, back[3] = {0, 0, 0};
        size_t k = HostPng_Encode(qol2_png, sizeof qol2_png, one, 1, 1);
        ASSERT(k == HostPng_EncodedSize(1, 1) && qol2_decode(qol2_png, k, &w, &h, back, sizeof back, NULL) &&
               memcmp(back, one, 3) == 0 && w == 1 && h == 1, "a 1x1 frame round-trips");
    }
    {
        uint8_t odd[7 * 5 * 3], back[7 * 5 * 3];
        size_t k;
        for (int i = 0; i < 7 * 5 * 3; i++) odd[i] = (uint8_t)(255 - i);
        k = HostPng_Encode(qol2_png, sizeof qol2_png, odd, 7, 5);
        ASSERT(k == HostPng_EncodedSize(7, 5) && qol2_decode(qol2_png, k, &w, &h, back, sizeof back, NULL) &&
               memcmp(back, odd, sizeof odd) == 0 && w == 7 && h == 5, "a 7x5 frame round-trips");
    }
    /* a corrupted byte fails the CRC check, not silently */
    n = HostPng_Encode(qol2_png, sizeof qol2_png, qol2_rgb, QOL2_W, QOL2_H);
    qol2_png[100] ^= 0x01;
    ASSERT(!qol2_decode(qol2_png, n, &w, &h, qol2_back, sizeof qol2_back, NULL), "a flipped bit is caught by the chunk CRC");
    PASS();
}

/* The screenshot test changes XDG_DATA_HOME and HOME.  Both are restored on
 * every exit, a failed assertion included, so no later test runs with them;
 * a variable that was unset before is unset again. */
static void qol2_shot_restore(int had_xdg, const char *old_xdg, int had_home, const char *old_home)
{
    if (had_xdg) setenv("XDG_DATA_HOME", old_xdg, 1); else unsetenv("XDG_DATA_HOME");
    if (had_home) setenv("HOME", old_home, 1); else unsetenv("HOME");
}
#define QOL2_SHOT_ASSERT(cond, msg) do { if (!(cond)) { \
    qol2_shot_restore(had_xdg, old_xdg, had_home, old_home); FAIL(msg); return; } } while (0)

static void test_QOL2_screenshot_files(void)
{
    char base[600], dir[700], path[800], path2[800], check[800];
    char old_xdg[512], old_home[512];
    int had_xdg = getenv("XDG_DATA_HOME") != NULL, had_home = getenv("HOME") != NULL;
    FILE *f;
    int w = 0, h = 0;
    long sz;
    size_t got;
    TEST("QOL2_screenshot_files");
    qol2_setup_root();
    snprintf(old_xdg, sizeof old_xdg, "%s", had_xdg ? getenv("XDG_DATA_HOME") : "");
    snprintf(old_home, sizeof old_home, "%s", had_home ? getenv("HOME") : "");
    snprintf(base, sizeof base, "%s/data", qol2_root);
    setenv("XDG_DATA_HOME", base, 1);
    QOL2_SHOT_ASSERT(HostShot_Dir(dir, sizeof dir) == 0, "a folder under XDG_DATA_HOME");
    snprintf(check, sizeof check, "%s/parasite-eve-port/screenshots", base);
    QOL2_SHOT_ASSERT(strcmp(dir, check) == 0, "the folder is XDG_DATA_HOME/parasite-eve-port/screenshots");
    qol2_fill_pattern(qol2_rgb, QOL2_W, QOL2_H);
    QOL2_SHOT_ASSERT(HostShot_Save(NULL, QOL2_W, QOL2_H, path, sizeof path) == -1, "no presented frame, no file");
    QOL2_SHOT_ASSERT(HostShot_Save(qol2_rgb, 0, 0, path, sizeof path) == -1, "a zero-size frame is refused");
    QOL2_SHOT_ASSERT(HostShot_Save(qol2_rgb, QOL2_W, QOL2_H, path, sizeof path) == 0, "the first shot is saved");
    QOL2_SHOT_ASSERT(strncmp(strrchr(path, '/') + 1, "pe-", 3) == 0 && strstr(path, ".png") != NULL &&
                     strncmp(path, check, strlen(check)) == 0, "saved under the screenshot folder as pe-<UTC>-<n>.png");
    {
        const char *leaf = strrchr(path, '/') + 1;
        /* pe-YYYYMMDDTHHMMSSZ-<n>.png: 3 + 16 + 1 digit-or-more + 4 */
        QOL2_SHOT_ASSERT(strlen(leaf) >= 3 + 16 + 2 + 4 && leaf[3 + 8] == 'T' && leaf[3 + 15] == 'Z',
                         "the name carries a basic-format UTC stamp");
    }
    QOL2_SHOT_ASSERT(HostShot_Save(qol2_rgb, QOL2_W, QOL2_H, path2, sizeof path2) == 0 && strcmp(path, path2) != 0,
                     "the next shot takes a new name and never overwrites");
    f = fopen(path, "rb");
    QOL2_SHOT_ASSERT(f != NULL, "the saved file exists");
    fseek(f, 0, SEEK_END);
    sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    got = fread(qol2_png, 1, sizeof qol2_png, f);
    fclose(f);
    QOL2_SHOT_ASSERT(sz == (long)HostPng_EncodedSize(QOL2_W, QOL2_H) && got == (size_t)sz,
                     "the file is exactly the encoded size");
    QOL2_SHOT_ASSERT(qol2_decode(qol2_png, got, &w, &h, qol2_back, sizeof qol2_back, NULL) &&
                     memcmp(qol2_back, qol2_rgb, sizeof qol2_rgb) == 0, "the saved file decodes to the presented frame");
    /* without XDG_DATA_HOME the folder comes from HOME; only the path is checked here */
    unsetenv("XDG_DATA_HOME");
    setenv("HOME", "/qol2-home", 1);
    QOL2_SHOT_ASSERT(HostShot_Dir(dir, sizeof dir) == 0 &&
                     strcmp(dir, "/qol2-home/.local/share/parasite-eve-port/screenshots") == 0,
                     "without XDG_DATA_HOME the folder is HOME/.local/share/...");
    unlink(path);
    unlink(path2);
    snprintf(check, sizeof check, "%s/parasite-eve-port/screenshots", base);
    (void)rmdir(check);
    snprintf(check, sizeof check, "%s/parasite-eve-port", base);
    (void)rmdir(check);
    (void)rmdir(base);
    (void)rmdir(qol2_root);
    qol2_shot_restore(had_xdg, old_xdg, had_home, old_home);
    PASS();
}
#undef QOL2_SHOT_ASSERT

/* BRIGHTNESS and MONO AUDIO on the dev menu, as the player reaches them:
 * OPTIONS > GRAPHICS (row 9) and OPTIONS > AUDIO (row 6).  The rows change
 * only themselves, and every row fits the overlay. */
static void test_QOL2_dev_menu(void)
{
    char label[96];
    int b = pe_enh_option_index("brightness"), m = pe_enh_option_index("mono");
    TEST("QOL2_dev_menu");
    enh_isolate();
    unsetenv("PE_BRIGHTNESS");
    unsetenv("PE_MONO");
    pe_enh_reset();
    setenv("PE_DEV_MENU", "1", 1);
    pe_plat_cheats_reset();
    (void)pe_plat_cheats_key(PE_CHEAT_KEY_DEV_MENU, 1);
    for (int i = 0; i < 6; i++) (void)pe_plat_cheats_key(PE_CHEAT_KEY_DOWN, 1);   /* OPTIONS */
    (void)pe_plat_cheats_key(PE_CHEAT_KEY_ENTER, 1);
    (void)pe_plat_cheats_key(PE_CHEAT_KEY_DOWN, 1);                               /* GRAPHICS */
    (void)pe_plat_cheats_key(PE_CHEAT_KEY_ENTER, 1);
    for (int i = 0; i < 8; i++) (void)pe_plat_cheats_key(PE_CHEAT_KEY_DOWN, 1);   /* row 9: BRIGHTNESS */
    ASSERT(pe_plat_cheats_overlay(qol2_fb, QOL2_W, QOL2_H) != qol2_fb, "graphics page draws");
    (void)pe_plat_cheats_key(PE_CHEAT_KEY_ENTER, 1);
    ASSERT(pe_enh_brightness_pct() == 110 && pe_enh_mono() == 0, "row 9 steps brightness 100 -> 110 and nothing else");
    (void)pe_plat_cheats_key(PE_CHEAT_KEY_ENTER, 1);
    (void)pe_plat_cheats_key(PE_CHEAT_KEY_ENTER, 1);
    (void)pe_plat_cheats_key(PE_CHEAT_KEY_ENTER, 1);
    ASSERT(pe_enh_brightness_pct() == 90, "the brightness steps cycle: 110, 120, 130, then 90");
    pe_enh_set_brightness_pct(100);
    for (int n = 0; n < pe_enh_category_count(PE_ENH_CAT_GRAPHICS); n++) {
        pe_enh_option_label(pe_enh_category_option(PE_ENH_CAT_GRAPHICS, n), label, sizeof label);
        ASSERT(strlen(label) + 2 <= 50, "every graphics row fits the 320-pixel overlay (6 px per character)");
    }
    /* close the menu, then open it again on the AUDIO page */
    (void)pe_plat_cheats_key(PE_CHEAT_KEY_DEV_MENU, 1);
    pe_plat_cheats_reset();
    setenv("PE_DEV_MENU", "1", 1);
    (void)pe_plat_cheats_key(PE_CHEAT_KEY_DEV_MENU, 1);
    for (int i = 0; i < 6; i++) (void)pe_plat_cheats_key(PE_CHEAT_KEY_DOWN, 1);   /* OPTIONS */
    (void)pe_plat_cheats_key(PE_CHEAT_KEY_ENTER, 1);
    (void)pe_plat_cheats_key(PE_CHEAT_KEY_DOWN, 1);
    (void)pe_plat_cheats_key(PE_CHEAT_KEY_DOWN, 1);                               /* AUDIO */
    (void)pe_plat_cheats_key(PE_CHEAT_KEY_ENTER, 1);
    for (int i = 0; i < 5; i++) (void)pe_plat_cheats_key(PE_CHEAT_KEY_DOWN, 1);   /* row 6: MONO AUDIO */
    ASSERT(pe_plat_cheats_overlay(qol2_fb, QOL2_W, QOL2_H) != qol2_fb, "audio page draws");
    (void)pe_plat_cheats_key(PE_CHEAT_KEY_ENTER, 1);
    ASSERT(pe_enh_mono() == 1 && pe_enh_brightness_pct() == 100, "row 6 turns mono on and nothing else");
    (void)pe_plat_cheats_key(PE_CHEAT_KEY_ENTER, 1);
    ASSERT(pe_enh_mono() == 0, "and back to stereo");
    for (int n = 0; n < pe_enh_category_count(PE_ENH_CAT_AUDIO); n++) {
        pe_enh_option_label(pe_enh_category_option(PE_ENH_CAT_AUDIO, n), label, sizeof label);
        ASSERT(strlen(label) + 2 <= 50, "every audio row fits the overlay");
    }
    (void)pe_plat_cheats_key(PE_CHEAT_KEY_DEV_MENU, 1);
    unsetenv("PE_DEV_MENU");
    pe_plat_cheats_reset();
    unlink(enh_conf);
    pe_enh_reset();
    ASSERT(b >= 0 && m >= 0, "both rows exist");
    PASS();
}

static void test_QOL2_all(void)
{
    qol2_setup_root();
    test_QOL2_registry();
    test_QOL2_persistence();
    test_QOL2_brightness_lut();
    test_QOL2_mono_rule();
    test_QOL2_png_checksums();
    test_QOL2_png_round_trip();
    test_QOL2_screenshot_files();
    test_QOL2_dev_menu();
    enh_isolate();                                  /* leave defaults for later tests */
    enh_hermetic();
}
