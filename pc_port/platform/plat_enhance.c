/* Opt-in gameplay enhancements: Aya's field movement speed and "Always run".
 * See pe_plat/enhance.h.  At the defaults the hooks are pure pass-throughs. */
#include "pe_plat/enhance.h"
#include "pe_plat/enhance_host.h"
#include "pe_guest_ram.h"
#include "pe_gpu.h"
#include "pe_gte_commands.h"
#include "host_framebuffer.h"
#include "pe_plat/audio.h"
#include "pe_spu.h"
#include "plat_enhance.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

const int pe_enh_move_speed_steps[PE_ENH_MOVE_SPEED_COUNT] = {100, 125, 150, 175, 200};
static const int k_toggle[] = {0, 1};
static const int k_mult[]   = {1, 2, 3};
#ifdef _WIN32
/* This backend has no asynchronous FSR presenter. Expose only modes it runs. */
static const int k_scaler[] = {PE_ENH_SCALER_NEAREST, PE_ENH_SCALER_SHARP_BILINEAR};
static const char *const k_scaler_names[] = {"NEAREST", "SHARP BILINEAR"};
#else
static const int k_scaler[] = {PE_ENH_SCALER_NEAREST, PE_ENH_SCALER_SHARP_BILINEAR, PE_ENH_SCALER_FSR1};
static const char *const k_scaler_names[] = {"NEAREST", "SHARP BILINEAR", "FSR 1 (CPU)"};
#endif
static const int k_internal_scale[] = {1, 2, 3, 4};
static const char *const k_internal_scale_names[] = {"NATIVE", "2X", "3X", "4X"};
static const char *const k_texture_filter_names[] = {"NEAREST", "BILINEAR (POLYGONS)"};
static const int k_dither[] = {0, 1, 2};
static const char *const k_dither_names[] = {"RETAIL", "FINE (SCALED)", "OFF"};
static const int k_display_filter[] = {PE_ENH_FILTER_OFF, PE_ENH_FILTER_SCANLINES, PE_ENH_FILTER_CRT};
static const char *const k_display_filter_names[] = {"OFF", "SCANLINES", "CRT (SCANLINES + MASK)"};
static const int k_display_fit[] = {PE_ENH_FIT_KEEP, PE_ENH_FIT_INTEGER, PE_ENH_FIT_STRETCH};
static const char *const k_display_fit_names[] = {"KEEP ASPECT", "INTEGER SCALE", "STRETCH"};
static const int k_volume[] = {100, 90, 80, 70, 60, 50, 40, 30, 20, 10, 0};
static const int k_spu_interp[] = {PE_ENH_SPU_GAUSSIAN, PE_ENH_SPU_CUBIC};
static const char *const k_spu_interp_names[] = {"GAUSSIAN", "CUBIC (4-POINT)"};
/* BRIGHTNESS steps in percent; the first is the retail identity.  The menu cycles them in this order. */
static const int k_brightness[] = {100, 110, 120, 130, 90, 80};

/* The registry.  Order within a category is the menu order. */
enum { OPT_DISPLAY_SCALER, OPT_INTERNAL_SCALE, OPT_TEXTURE_FILTER, OPT_SSAA, OPT_DITHER, OPT_SUBPIXEL,
       OPT_DISPLAY_FILTER, OPT_DISPLAY_FIT, OPT_BRIGHTNESS, OPT_MOVE_SPEED, OPT_ALWAYS_RUN, OPT_PAUSE_FOCUS, OPT_ONE_HIT, OPT_GOD, OPT_EXP_MULT,
       OPT_MASTER_VOLUME, OPT_MOVIE_VOLUME, OPT_MUSIC_VOLUME, OPT_SFX_VOLUME, OPT_SPU_INTERP, OPT_MONO, OPT_ANALOG_STICK, OPT_COUNT };
static const PeEnhOption k_options[OPT_COUNT] = {
    {"display_scaler", PE_ENH_CAT_GRAPHICS, "DISPLAY SCALER",      PE_ENH_FMT_NAMED,
        k_scaler, sizeof k_scaler / sizeof k_scaler[0], "PE_DISPLAY_SCALER", "the owner", k_scaler_names},
    {"internal_scale", PE_ENH_CAT_GRAPHICS, "INTERNAL RESOLUTION", PE_ENH_FMT_NAMED,
        k_internal_scale, 4, "PE_INTERNAL_SCALE", "the owner", k_internal_scale_names},
    {"texture_filter", PE_ENH_CAT_GRAPHICS, "TEXTURE FILTER", PE_ENH_FMT_NAMED,
        k_toggle, 2, "PE_TEXTURE_FILTER", "the owner", k_texture_filter_names},
    {"ssaa", PE_ENH_CAT_GRAPHICS, "SUPERSAMPLE AA", PE_ENH_FMT_TOGGLE,
        k_toggle, 2, "PE_SSAA", "the owner", NULL},
    {"dithering", PE_ENH_CAT_GRAPHICS, "DITHERING", PE_ENH_FMT_NAMED,
        k_dither, 3, "PE_DITHERING", "the owner", k_dither_names},
    {"subpixel_geometry", PE_ENH_CAT_GRAPHICS, "SUB-PIXEL GEOMETRY", PE_ENH_FMT_TOGGLE,
        k_toggle, 2, "PE_SUBPIXEL_GEOMETRY", "the owner", NULL},
    {"display_filter", PE_ENH_CAT_GRAPHICS, "DISPLAY FILTER", PE_ENH_FMT_NAMED,
        k_display_filter, 3, "PE_DISPLAY_FILTER", "the owner", k_display_filter_names},
    {"display_fit", PE_ENH_CAT_GRAPHICS, "WINDOW FIT", PE_ENH_FMT_NAMED,
        k_display_fit, 3, "PE_DISPLAY_FIT", "the owner", k_display_fit_names},
    {"brightness", PE_ENH_CAT_GRAPHICS, "BRIGHTNESS", PE_ENH_FMT_PERCENT,
        k_brightness, 6, "PE_BRIGHTNESS", "the owner"},
    {"move_speed",    PE_ENH_CAT_GAMEPLAY, "MOVEMENT SPEED",       PE_ENH_FMT_PERCENT,
        pe_enh_move_speed_steps, PE_ENH_MOVE_SPEED_COUNT, "PE_MOVE_SPEED",    "u/stellarola"},
    {"always_run",    PE_ENH_CAT_GAMEPLAY, "ALWAYS RUN",           PE_ENH_FMT_TOGGLE,
        k_toggle, 2, "PE_ALWAYS_RUN",    "u/stellarola"},
    {"pause_on_focus_loss", PE_ENH_CAT_GAMEPLAY, "PAUSE ON FOCUS LOSS", PE_ENH_FMT_TOGGLE,
        k_toggle, 2, "PE_PAUSE_ON_FOCUS_LOSS", "the owner", NULL},
    {"one_hit_kills", PE_ENH_CAT_ASSIST,   "1-HIT KILLS",          PE_ENH_FMT_TOGGLE,
        k_toggle, 2, "PE_ONE_HIT_KILLS", NULL},
    {"god_mode",      PE_ENH_CAT_ASSIST,   "GOD MODE (NO DAMAGE)", PE_ENH_FMT_TOGGLE,
        k_toggle, 2, "PE_GOD_MODE",      "u/SandmansSlave"},
    {"exp_mult",      PE_ENH_CAT_ASSIST,   "EXP/BP MULTIPLIER",    PE_ENH_FMT_MULT,
        k_mult, 3, "PE_EXP_MULT",       "u/SandmansSlave"},
    {"master_volume", PE_ENH_CAT_AUDIO,   "MASTER VOLUME",        PE_ENH_FMT_PERCENT,
        k_volume, 11, "PE_MASTER_VOLUME", "the owner"},
    {"movie_volume",  PE_ENH_CAT_AUDIO,   "MOVIE AUDIO VOLUME",   PE_ENH_FMT_PERCENT,
        k_volume, 11, "PE_MOVIE_VOLUME",  "the owner"},
    {"music_volume",  PE_ENH_CAT_AUDIO,   "MUSIC VOLUME",         PE_ENH_FMT_PERCENT,
        k_volume, 11, "PE_MUSIC_VOLUME",  "the owner"},
    {"sfx_volume",    PE_ENH_CAT_AUDIO,   "SFX VOLUME",           PE_ENH_FMT_PERCENT,
        k_volume, 11, "PE_SFX_VOLUME",    "the owner"},
    {"spu_interpolation", PE_ENH_CAT_AUDIO, "SPU INTERPOLATION",  PE_ENH_FMT_NAMED,
        k_spu_interp, 2, "PE_SPU_INTERPOLATION", "the owner", k_spu_interp_names},
    {"mono",          PE_ENH_CAT_AUDIO,   "MONO AUDIO",           PE_ENH_FMT_TOGGLE,
        k_toggle, 2, "PE_MONO", "the owner"},
    {"analog_stick",  PE_ENH_CAT_CONTROLS, "ANALOG STICK",        PE_ENH_FMT_TOGGLE,
        k_toggle, 2, "PE_ANALOG_STICK",  "the owner"},
};

#define GA_AYA_ACTOR  0x8009D254u   /* *(u32 *) = Aya's field actor */
#define GA_GAME_FLAGS 0x8009D1A0u   /* bit 1 = battle */

static struct {
    int inited;
    int value[OPT_COUNT];   /* in effect: config, then env override */
    int file[OPT_COUNT];    /* what the config holds; save_config writes this */
} E;

static void sync_graphics(void)
{
    int scale = E.value[OPT_INTERNAL_SCALE];
    if ((E.value[OPT_SSAA] || E.value[OPT_TEXTURE_FILTER] || E.value[OPT_DITHER] || E.value[OPT_SUBPIXEL]) &&
        scale == 1) scale = 2;
    PE_GPU_SetRenderScale(scale);
    PE_GPU_SetTextureFilter(E.value[OPT_TEXTURE_FILTER]);
    PE_GPU_SetShadowDither(E.value[OPT_DITHER]);
    HostFB_SetSSAA(E.value[OPT_SSAA]);
    /* Sub-pixel geometry: the GTE records screen fractions, the shadow
     * rasterizer reads them.  Neither touches guest-visible state. */
    PE_GTE_SetSubpixelCapture(E.value[OPT_SUBPIXEL]);
    PE_GPU_SetSubpixelLookup(E.value[OPT_SUBPIXEL] ? PE_GTE_SubpixelLookup : NULL);
}

/* MASTER VOLUME is read by host_audio.c on each device submit. */
static void sync_audio(void)
{
    pe_plat_audio_set_cd_gain(E.value[OPT_MOVIE_VOLUME]);
    pe_plat_audio_set_voice_gains(E.value[OPT_MUSIC_VOLUME], E.value[OPT_SFX_VOLUME]);
    PE_Spu_SetInterpolation(E.value[OPT_SPU_INTERP]);
}

/* Snap to a listed value: the largest listed value <= v, else the smallest
 * listed one.  Lists that rise from the retail value (speed, multipliers)
 * snap below-range input to retail; the volume lists fall from it. */
static int snap(int i, int v)
{
    const PeEnhOption *o = &k_options[i];
    int best = 0, found = 0, lowest = o->values[0];
    for (int k = 0; k < o->nvalues; k++) {
        if (o->values[k] < lowest) lowest = o->values[k];
        if (o->values[k] <= v && (!found || o->values[k] > best)) { best = o->values[k]; found = 1; }
    }
    if (o->format == PE_ENH_FMT_TOGGLE) return v != 0;
    return found ? best : lowest;
}

/* A config or env value must be a whole decimal number.  Anything else
 * (empty, "loud", "50%") keeps the retail value instead of snapping. */
static int parse_value(int i, const char *s)
{
    char *end;
    long v;
    errno = 0;
    v = strtol(s, &end, 10);
    if (end == s) return k_options[i].values[0];   /* no digits (strtol leaves end at s) */
    while (*end == ' ' || *end == '\t' || *end == '\r' || *end == '\n') end++;
    if (*end || errno || v < -1000000 || v > 1000000) return k_options[i].values[0];
    return snap(i, (int)v);
}

static int config_path(char *out, size_t n)
{
    const char *p = getenv("PE_ENH_CONFIG");
    const char *x;
    if (p && *p) return snprintf(out, n, "%s", p) < (int)n;
#ifdef _WIN32
    /* A stock Windows account has no HOME; controls.conf and the memory card already
     * live under %APPDATA%.  Forward slashes keep mkdir_parent()'s '/' split working. */
    x = getenv("APPDATA");
    if (x && *x) return snprintf(out, n, "%s/parasite-eve-port/enhancements.conf", x) < (int)n;
#endif
    x = getenv("XDG_CONFIG_HOME");
    if (x && *x) return snprintf(out, n, "%s/parasite-eve-port/enhancements.conf", x) < (int)n;
    x = getenv("HOME");
    if (x && *x) return snprintf(out, n, "%s/.config/parasite-eve-port/enhancements.conf", x) < (int)n;
    return 0;
}

static void load_config(void)
{
    char path[1024], line[128];
    FILE *f;
    if (!config_path(path, sizeof path)) return;
    f = fopen(path, "r");
    if (!f) return;
    while (fgets(line, sizeof line, f)) {
        char *eq = strchr(line, '=');
        if (!eq || line[0] == '#') continue;
        *eq = 0;
        for (int i = 0; i < OPT_COUNT; i++)
            if (strcmp(line, k_options[i].id) == 0) { E.value[i] = E.file[i] = parse_value(i, eq + 1); break; }
    }
    fclose(f);
}

static void mkdir_parent(const char *path)
{
    char dir[1024];
    char *s;
    snprintf(dir, sizeof dir, "%s", path);
    s = strrchr(dir, '/');
    if (!s || s == dir) return;
    *s = 0;
    for (char *q = dir + 1; *q; q++) {
        if (*q != '/') continue;
        *q = 0; (void)mkdir(dir, 0700); *q = '/';
    }
    (void)mkdir(dir, 0700);
}

/* Written to a sibling temp file and renamed over the config, so an
 * interrupted save never leaves a truncated value behind. */
static void save_config(void)
{
    char path[1024], tmp[1100];
    FILE *f;
    int failed;
    if (!config_path(path, sizeof path)) return;
    mkdir_parent(path);
    snprintf(tmp, sizeof tmp, "%s.tmp", path);
    f = fopen(tmp, "w");
    if (!f) {
        fprintf(stderr, "[ENH] cannot write %s: %s\n", tmp, strerror(errno));
        return;
    }
    fprintf(f, "# Parasite Eve port enhancements and assists (dev menu > Options / Assists)\n");
    fprintf(f, "# Every value's default is the retail behaviour; delete this file to reset.\n");
    for (int i = 0; i < OPT_COUNT; i++)
        fprintf(f, "%s=%d\n", k_options[i].id, E.file[i]);
    failed = ferror(f) != 0;
    if (fclose(f) != 0) failed = 1;
#ifdef _WIN32
    if (!failed) (void)remove(path);   /* the CRT's rename() does not replace (pe_memcard.c pattern) */
#endif
    if (failed || rename(tmp, path) != 0) {
        fprintf(stderr, "[ENH] cannot write %s: %s\n", path, strerror(errno));
        (void)remove(tmp);
    }
}

void pe_enh_init(void)
{
    int any = 0;
    if (E.inited) return;
    E.inited = 1;
    for (int i = 0; i < OPT_COUNT; i++) E.value[i] = E.file[i] = k_options[i].values[0];
    load_config();
    for (int i = 0; i < OPT_COUNT; i++) {
        const char *e = getenv(k_options[i].env);
        if (e && *e) E.value[i] = parse_value(i, e);
        if (E.value[i] != k_options[i].values[0]) any = 1;
    }
    sync_graphics();
    sync_audio();
    if (any) {
        fprintf(stderr, "[ENH]");
        for (int i = 0; i < OPT_COUNT; i++)
            if (E.value[i] != k_options[i].values[0]) fprintf(stderr, " %s=%d", k_options[i].id, E.value[i]);
        fprintf(stderr, "\n");
    }
}

void pe_enh_reset(void) {
    memset(&E, 0, sizeof E);
    PE_GPU_SetRenderScale(1);
    PE_GPU_SetTextureFilter(0);
    PE_GPU_SetShadowDither(0);
    HostFB_SetSSAA(0);
    PE_GTE_SetSubpixelCapture(0);
    PE_GPU_SetSubpixelLookup(NULL);
    pe_plat_audio_set_cd_gain(100);
    pe_plat_audio_set_voice_gains(100, 100);
    PE_Spu_SetInterpolation(PE_SPU_INTERP_GAUSSIAN);
}

/* ── registry access ─────────────────────────────────────────────────── */
int pe_enh_option_count(void) { return OPT_COUNT; }
const PeEnhOption *pe_enh_option(int i) { return (i >= 0 && i < OPT_COUNT) ? &k_options[i] : NULL; }
int pe_enh_option_index(const char *id)
{
    for (int i = 0; id && i < OPT_COUNT; i++) if (strcmp(id, k_options[i].id) == 0) return i;
    return -1;
}
int pe_enh_category_count(PeEnhCategory cat)
{
    int n = 0;
    for (int i = 0; i < OPT_COUNT; i++) if (k_options[i].category == cat) n++;
    return n;
}
int pe_enh_category_option(PeEnhCategory cat, int n)
{
    for (int i = 0; i < OPT_COUNT; i++) if (k_options[i].category == cat && n-- == 0) return i;
    return -1;
}
int pe_enh_option_value(int i)
{
    pe_enh_init();
    return (i >= 0 && i < OPT_COUNT) ? E.value[i] : 0;
}
int pe_enh_option_is_retail(int i)
{
    return i >= 0 && i < OPT_COUNT && pe_enh_option_value(i) == k_options[i].values[0];
}
void pe_enh_option_set(int i, int value)
{
    if (i < 0 || i >= OPT_COUNT) return;
    pe_enh_init();
    E.value[i] = E.file[i] = snap(i, value);   /* a menu choice is the player's: it persists */
    if (k_options[i].category == PE_ENH_CAT_GRAPHICS) sync_graphics();
    if (k_options[i].category == PE_ENH_CAT_AUDIO) sync_audio();
    save_config();
}
void pe_enh_option_cycle(int i)
{
    const PeEnhOption *o = pe_enh_option(i);
    int k;
    if (!o) return;
    pe_enh_init();
    for (k = 0; k < o->nvalues; k++) if (o->values[k] == E.value[i]) break;
    pe_enh_option_set(i, o->values[(k + 1) % o->nvalues]);
}
void pe_enh_option_label(int i, char *buf, size_t n)
{
    const PeEnhOption *o = pe_enh_option(i);
    int v;
    if (!o) { if (n) buf[0] = 0; return; }
    v = pe_enh_option_value(i);
    switch (o->format) {
    case PE_ENH_FMT_TOGGLE:  snprintf(buf, n, "%s: %s", o->label, v ? "ON" : "OFF"); break;
    case PE_ENH_FMT_PERCENT: snprintf(buf, n, "%s: %d%%%s", o->label, v, pe_enh_option_is_retail(i) ? " (RETAIL)" : ""); break;
    case PE_ENH_FMT_NAMED: {
        int k;
        for (k = 0; k < o->nvalues && o->values[k] != v; k++) ;
        snprintf(buf, n, "%s: %s%s", o->label, (o->names && k < o->nvalues) ? o->names[k] : "?",
                 pe_enh_option_is_retail(i) ? " (RETAIL)" : "");
        break;
    }
    default:                 snprintf(buf, n, "%s: %dX%s", o->label, v, pe_enh_option_is_retail(i) ? " (RETAIL)" : ""); break;
    }
}

/* ── named accessors (thin wrappers over the registry) ───────────────── */
int  pe_enh_display_scaler(void)          { return pe_enh_option_value(OPT_DISPLAY_SCALER); }
void pe_enh_set_display_scaler(int mode)  { pe_enh_option_set(OPT_DISPLAY_SCALER, mode); }
int  pe_enh_internal_scale(void)          { return pe_enh_option_value(OPT_INTERNAL_SCALE); }
void pe_enh_set_internal_scale(int scale) { pe_enh_option_set(OPT_INTERNAL_SCALE, scale); }
int  pe_enh_display_filter(void)          { return pe_enh_option_value(OPT_DISPLAY_FILTER); }
void pe_enh_set_display_filter(int mode)  { pe_enh_option_set(OPT_DISPLAY_FILTER, mode); }
int  pe_enh_display_fit(void)             { return pe_enh_option_value(OPT_DISPLAY_FIT); }
void pe_enh_set_display_fit(int mode)     { pe_enh_option_set(OPT_DISPLAY_FIT, mode); }
int  pe_enh_spu_interpolation(void)       { return pe_enh_option_value(OPT_SPU_INTERP); }
void pe_enh_set_spu_interpolation(int m)  { pe_enh_option_set(OPT_SPU_INTERP, m); }
int  pe_enh_pause_on_focus_loss(void)     { return pe_enh_option_value(OPT_PAUSE_FOCUS); }
void pe_enh_set_pause_on_focus_loss(int on) { pe_enh_option_set(OPT_PAUSE_FOCUS, on); }
int  pe_enh_master_volume(void)           { return pe_enh_option_value(OPT_MASTER_VOLUME); }
int  pe_enh_movie_volume(void)            { return pe_enh_option_value(OPT_MOVIE_VOLUME); }
void pe_enh_set_master_volume(int pct)    { pe_enh_option_set(OPT_MASTER_VOLUME, pct); }
void pe_enh_set_movie_volume(int pct)     { pe_enh_option_set(OPT_MOVIE_VOLUME, pct); }
int  pe_enh_music_volume(void)            { return pe_enh_option_value(OPT_MUSIC_VOLUME); }
int  pe_enh_sfx_volume(void)              { return pe_enh_option_value(OPT_SFX_VOLUME); }
void pe_enh_set_music_volume(int pct)     { pe_enh_option_set(OPT_MUSIC_VOLUME, pct); }
void pe_enh_set_sfx_volume(int pct)       { pe_enh_option_set(OPT_SFX_VOLUME, pct); }
int  pe_enh_move_speed_pct(void)          { return pe_enh_option_value(OPT_MOVE_SPEED); }
int  pe_enh_always_run(void)              { return pe_enh_option_value(OPT_ALWAYS_RUN); }
void pe_enh_set_move_speed_pct(int pct)   { pe_enh_option_set(OPT_MOVE_SPEED, pct); }
void pe_enh_cycle_move_speed(void)        { pe_enh_option_cycle(OPT_MOVE_SPEED); }
void pe_enh_set_always_run(int on)        { pe_enh_option_set(OPT_ALWAYS_RUN, on); }
int  pe_enh_one_hit_kills(void)           { return pe_enh_option_value(OPT_ONE_HIT); }
int  pe_enh_god_mode(void)                { return pe_enh_option_value(OPT_GOD); }
int  pe_enh_exp_mult(void)                { return pe_enh_option_value(OPT_EXP_MULT); }
void pe_enh_set_one_hit_kills(int on)     { pe_enh_option_set(OPT_ONE_HIT, on); }
void pe_enh_set_god_mode(int on)          { pe_enh_option_set(OPT_GOD, on); }
void pe_enh_set_exp_mult(int mult)        { pe_enh_option_set(OPT_EXP_MULT, mult); }
void pe_enh_cycle_exp_mult(void)          { pe_enh_option_cycle(OPT_EXP_MULT); }
int  pe_enh_analog_stick(void)            { return pe_enh_option_value(OPT_ANALOG_STICK); }
void pe_enh_set_analog_stick(int on)      { pe_enh_option_set(OPT_ANALOG_STICK, on); }
int  pe_enh_brightness_pct(void)          { return pe_enh_option_value(OPT_BRIGHTNESS); }
void pe_enh_set_brightness_pct(int pct)   { pe_enh_option_set(OPT_BRIGHTNESS, pct); }
int  pe_enh_mono(void)                    { return pe_enh_option_value(OPT_MONO); }
void pe_enh_set_mono(int on)              { pe_enh_option_set(OPT_MONO, on); }

static int aya_in_field(uint32_t actor)
{
    return actor != 0u && actor == PE_LoadU32(GA_AYA_ACTOR)
        && (PE_LoadU32(GA_GAME_FLAGS) & 2u) == 0u;
}

int32_t pe_enh_field_speed(uint32_t actor, int32_t speed)
{
    int pct = pe_enh_move_speed_pct();
    if (pct == 100 || !aya_in_field(actor)) return speed;   /* retail path */
    return (int32_t)(((int64_t)speed * pct) / 100);
}

int pe_enh_field_always_run(uint32_t actor)
{
    return pe_enh_always_run() && aya_in_field(actor);
}

/* ── Assist hooks ────────────────────────────────────────────────────── */
int32_t pe_enh_enemy_damage(uint32_t body, int32_t damage)
{
    int32_t hp;
    if (!pe_enh_one_hit_kills() || body == 0u) return damage;    /* retail path */
    /* Story-scripted battles (game flag 0x800: the Eve stage fight on Day 1) are ended by
     * their script, not by the enemy's HP; killing the enemy there leaves the script
     * waiting forever (observed on the Day-1 autopilot).  There the assist only applies
     * the script-phase marker rule below. */
    hp = (int32_t)PE_LoadU32(body + 0x10u);
    /* Above the +1000000 script-phase marker, one hit takes the enemy down to
     * exactly 1000000 and the room script's own exit runs (also in story battles,
     * flag 0x800: the Day-5 Statue of Liberty Eve, m0273i, three parts at
     * 1000000 + 1589/1750/1900). */
    if (hp > 1000000) return damage > 0 ? hp - 1000000 : damage;
    if (PE_LoadU32(GA_GAME_FLAGS) & 0x800u) return damage;
    /* Scripts also make an enemy invulnerable for a scripted phase by adding 1000000
     * to its HP (rooms test `HP <= 1000000`, e.g. room_m0429i func_801926B4).  Killing
     * it there skips the script's own exit: the Day-4 m0156i roof fight then sat in
     * op_C3 forever with "NO TARGET".  Retail damage while the marker is set, and
     * just below it: the script's exit runs once HP drops to <= 1000000 (999999 on
     * that roof), and a second bullet in that window must not kill it either.  No
     * ordinary enemy comes near 900000 HP. */
    if (hp > 900000) return damage;
    return (hp > 0 && damage < hp) ? hp : damage;
}

int pe_enh_player_shielded(void) { return pe_enh_god_mode(); }

uint32_t pe_enh_exp_award(uint32_t exp)
{
    int m = pe_enh_exp_mult();
    return m == 1 ? exp : exp * (uint32_t)m;
}

uint32_t pe_enh_bp_award(uint32_t bp)
{
    int m = pe_enh_exp_mult();
    return m == 1 ? bp : bp * (uint32_t)m;
}
