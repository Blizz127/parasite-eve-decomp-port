/* pe_plat cheats: opt-in cheat console (contract in pe_plat/cheats.h).
 *
 * Guest-state facts used (each from matched src/ or a verified port):
 *   Aya battle record  *0x8009D278: +0x0C HP (u16), +0x1C max HP (u16),
 *                      +0x68 weapon record: +0x0C low 10 bits = rounds loaded
 *                      (route_ammo_trace.h; the harness HP lock uses +12/+28).
 *   Aya stats          **0x8009D254: +0x08 PE, +0x28 max PE (16.16;
 *                      src/func_800515F8.c reads rec[5] / rec[0x15]).
 *   Ammo reserves      0x800A1E6E + k*32 (u16, k = 0..2).
 *   Enemy table        0x8009E000 + i*12 actor pointers; HP = *(actor)+0x10
 *                      (s32; script tag 0x2C, src/func_8003010C.c).
 *   Add item           func_80053D2C(id) (src/func_80053D2C.c; inventory
 *                      D_8009D048[D_8009D050] u16 slots).
 *   Give-item op       field VM op 0xA7 = func_800194B0 (D_800910A0[0xA7]):
 *                      argc 2, arg0 = item id.
 *   Room transfer      field VM op 0x31 = func_80017BB4(args): latches the
 *                      room token in D_8009D280 and sets D_8009D1A0 |= 0x2000.
 *   Story word         persist[74] = 0x800A7918.
 * No retail data tables are embedded: item ids come from the loaded room
 * scripts at run time; the warp list is route checkpoint tokens.
 */
#include "pe_plat/cheats.h"
#include "pe_plat/mods.h"
#include "pe_guest_ram.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Game routines (weak: small tools linking only the platform library). */
extern int func_80053D2C(int id) __attribute__((weak));
extern int func_80017BB4(pe_addr_t a0) __attribute__((weak));

#define GA_AYA_REC     0x8009D278u
#define GA_AYA_ACTOR   0x8009D254u
#define GA_ENEMIES     0x8009E000u
#define GA_D1A0        0x8009D1A0u
#define GA_MOVIE       0x800B0DBAu   /* field movie active (func_801216C4 sets, func_80121A00 clears) */
#define GA_TOKEN       0x8009D280u
#define GA_ROOM_CUR    0x8009D1C4u   /* func_8003F3C4 loops while == D_8009D280 */
#define GA_STORY       0x800A7918u
#define GA_INV         0x8009D048u
#define GA_INV_COUNT   0x8009D050u
/* Scratch for the room-transfer argument block (unused hand window). */
#define PE_CHEAT_STACK 0x801FF460u   /* +0 args[0] -> +8 token word */

typedef struct { const char *name; uint32_t token, story; } CheatWarp;
/* Day-1 route checkpoints: first-entry room token and persist[74]
 * (build/lanes/port/rr_full30.log "[ROUTE] change" lines). */
static const CheatWarp k_warps[] = {
    {"FIELD START (CARNEGIE HALL)", 0xA8000148u, 0x08u},
    {"STAGE (M0002I)",              0xA80002C8u, 0x18u},
    {"M0011I",                      0xA80011C8u, 0x40u},
    {"EVE 1 ROOM (M0023I)",         0xA80021C8u, 0x5Bu},
    {"SEWER ENTRY (M0026I)",        0xA8002348u, 0x68u},
    {"SEWER (M0027I)",              0xA80023C8u, 0x68u},
    {"SEWER HALL (M0028I)",         0xA8002448u, 0x68u},
    {"M0031I",                      0xA80030C8u, 0x68u},
    {"M0032I",                      0xA8003148u, 0x68u},
    {"M0033I",                      0xA80031C8u, 0x68u},
    {"M34 ALLIGATOR (M0034I)",      0xA8003248u, 0x6Cu},
    {"M0359I POST-BOSS",            0xA80654C8u, 0x70u},
    {"M0036I DAY-1 EXIT",           0xA8003348u, 0x78u},
};
#define N_WARPS ((int)(sizeof(k_warps) / sizeof(k_warps[0])))

static struct {
    int inited, enabled, hook;
    int god, ammo, help, menu, menu_index, ff_index;
    int pending_win, pending_kit, pending_warp;
    unsigned wins, kits, warps;
    uint32_t ammo_gun, ammo_loaded_max, ammo_pool_max[3];
    char toast[64]; int toast_frames;
    uint64_t frames;
    struct { uint64_t frame; PeCheatKey key; } script[64];
    int nscript, iscript;
} C;

static const int k_ff_mult[4] = {0, 2, 4, 8};

static void toast(const char *msg)
{
    snprintf(C.toast, sizeof(C.toast), "%s", msg);
    C.toast_frames = 150;
    fprintf(stderr, "[CHEATS] %s\n", msg);
}

static PeCheatKey key_from_name(const char *s, size_t n)
{
    static const struct { const char *name; PeCheatKey key; } t[] = {
        {"F1",PE_CHEAT_KEY_F1},{"F2",PE_CHEAT_KEY_F2},{"F3",PE_CHEAT_KEY_F3},
        {"F4",PE_CHEAT_KEY_F4},{"F5",PE_CHEAT_KEY_F5},{"F7",PE_CHEAT_KEY_F7},
        {"F8",PE_CHEAT_KEY_F8},{"UP",PE_CHEAT_KEY_UP},{"DOWN",PE_CHEAT_KEY_DOWN},
        {"ENTER",PE_CHEAT_KEY_ENTER}};
    for (size_t i = 0; i < sizeof(t) / sizeof(t[0]); i++)
        if (strlen(t[i].name) == n && !strncmp(s, t[i].name, n)) return t[i].key;
    return PE_CHEAT_KEY_NONE;
}

static void load_script(void)
{
    const char *s = getenv("PE_CHEAT_KEYS");
    C.nscript = C.iscript = 0;
    while (s && *s && C.nscript < 64) {
        char *end; unsigned long long f = strtoull(s, &end, 10);
        const char *k, *e;
        if (end == s || *end != ':') break;
        k = end + 1; e = k;
        while (*e && *e != ',') e++;
        C.script[C.nscript].frame = f;
        C.script[C.nscript].key = key_from_name(k, (size_t)(e - k));
        if (C.script[C.nscript].key != PE_CHEAT_KEY_NONE) C.nscript++;
        s = *e ? e + 1 : e;
    }
}

static void frame_hook(const PePlatModEventData *ev, void *user)
{
    (void)ev; (void)user;
    pe_plat_cheats_apply();
}

void pe_plat_cheats_init(void)
{
    const char *e;
    if (C.inited) return;
    C.inited = 1;
    e = getenv("PE_CHEATS");
    C.enabled = !(e && e[0] == '0');
    if (!C.enabled) return;
    load_script();
    C.hook = pe_plat_mods_subscribe(PE_PLAT_EVENT_FRAME_TICK, frame_hook, NULL);
}

int pe_plat_cheats_enabled(void) { return C.inited ? C.enabled : 1; }
int pe_plat_cheats_menu_open(void) { return C.enabled && C.menu; }
int pe_plat_cheats_ff_multiplier(void) { return k_ff_mult[C.ff_index & 3]; }
int pe_plat_cheats_warp_count(void) { return N_WARPS; }
const char *pe_plat_cheats_warp_name(int i) { return (i >= 0 && i < N_WARPS) ? k_warps[i].name : ""; }

void pe_plat_cheats_get_state(PePlatCheatState *o)
{
    o->god = C.god; o->ammo = C.ammo; o->help = C.help; o->menu = C.menu;
    o->menu_index = C.menu_index; o->ff_multiplier = pe_plat_cheats_ff_multiplier();
    o->wins = C.wins; o->kits = C.kits; o->warps = C.warps;
}

void pe_plat_cheats_reset(void)
{
    int inited = C.inited, enabled = C.enabled, hook = C.hook;
    memset(&C, 0, sizeof(C));
    C.inited = inited; C.enabled = enabled; C.hook = hook;
}

int pe_plat_cheats_key(PeCheatKey key, int pressed)
{
    char msg[64];
    if (!C.inited) pe_plat_cheats_init();
    if (!C.enabled) return 0;
    if (C.menu && (key == PE_CHEAT_KEY_UP || key == PE_CHEAT_KEY_DOWN || key == PE_CHEAT_KEY_ENTER)) {
        if (pressed) {
            if (key == PE_CHEAT_KEY_UP) C.menu_index = (C.menu_index + N_WARPS - 1) % N_WARPS;
            else if (key == PE_CHEAT_KEY_DOWN) C.menu_index = (C.menu_index + 1) % N_WARPS;
            else { C.pending_warp = C.menu_index + 1; C.menu = 0; }
        }
        return 1;
    }
    if (key == PE_CHEAT_KEY_UP || key == PE_CHEAT_KEY_DOWN || key == PE_CHEAT_KEY_ENTER ||
        key == PE_CHEAT_KEY_NONE)
        return 0;
    if (!pressed) return 1;
    switch (key) {
    case PE_CHEAT_KEY_F1: C.help = !C.help; break;
    case PE_CHEAT_KEY_F2: C.god = !C.god; toast(C.god ? "GOD MODE ON" : "GOD MODE OFF"); break;
    case PE_CHEAT_KEY_F3:
        C.ammo = !C.ammo; C.ammo_gun = 0;
        toast(C.ammo ? "INFINITE AMMO + FULL PE ON" : "INFINITE AMMO + FULL PE OFF"); break;
    case PE_CHEAT_KEY_F4: C.pending_win = 1; toast("INSTANT WIN"); break;
    case PE_CHEAT_KEY_F5: C.pending_kit = 1; break;
    case PE_CHEAT_KEY_F7:
        C.ff_index = (C.ff_index + 1) & 3;
        if (k_ff_mult[C.ff_index]) snprintf(msg, sizeof msg, "FAST-FORWARD SPEED %dX", k_ff_mult[C.ff_index]);
        else snprintf(msg, sizeof msg, "FAST-FORWARD SPEED MAX");
        toast(msg); break;
    case PE_CHEAT_KEY_F8: C.menu = !C.menu; break;
    default: break;
    }
    return 1;
}

/* ── game-state passes ─────────────────────────────────────────────── */
static int ram(pe_addr_t a, uint32_t n) { return a && PE_RangeIsRam(a, n); }

static void god_pass(void)
{
    pe_addr_t rec = PE_LoadU32(GA_AYA_REC);
    if (!ram(rec, 0x20u)) return;
    uint16_t max = PE_LoadU16(rec + 0x1Cu);
    if (max && PE_LoadU16(rec + 0x0Cu) != max) PE_StoreU16(rec + 0x0Cu, max);
}

static void ammo_pass(void)
{
    pe_addr_t rec = PE_LoadU32(GA_AYA_REC), actor, stats, gun;
    unsigned k;
    if (ram(rec, 0x70u) && ram(gun = PE_LoadU32(rec + 0x68u), 0x10u)) {
        uint32_t w = PE_LoadU32(gun + 0x0Cu), loaded = w & 0x3FFu;
        if (gun != C.ammo_gun) { C.ammo_gun = gun; C.ammo_loaded_max = loaded; }
        if (loaded > C.ammo_loaded_max) C.ammo_loaded_max = loaded;
        if (loaded < C.ammo_loaded_max)
            PE_StoreU32(gun + 0x0Cu, (w & ~0x3FFu) | C.ammo_loaded_max);
    }
    for (k = 0; k < 3u; k++) {
        uint16_t v = PE_LoadU16(0x800A1E6Eu + k * 32u);
        if (v > C.ammo_pool_max[k]) C.ammo_pool_max[k] = v;
        if (v < C.ammo_pool_max[k]) PE_StoreU16(0x800A1E6Eu + k * 32u, (uint16_t)C.ammo_pool_max[k]);
    }
    actor = PE_LoadU32(GA_AYA_ACTOR);
    if (ram(actor, 4u) && ram(stats = PE_LoadU32(actor), 0x2Cu)) {
        uint32_t max = PE_LoadU32(stats + 0x28u);
        if ((int32_t)max > 0 && PE_LoadU32(stats + 0x08u) != max) PE_StoreU32(stats + 0x08u, max);
    }
}

static int win_pass(void)
{
    pe_addr_t aya = PE_LoadU32(GA_AYA_ACTOR);
    unsigned i, n = 0;
    if (!(PE_LoadU32(GA_D1A0) & 2u)) return -1;          /* not in a battle */
    for (i = 0; i < 45u; i++) {
        pe_addr_t p = PE_LoadU32(GA_ENEMIES + i * 12u), body;
        if (!p) break;
        if (p == aya || !ram(p, 0x10u) || !ram(body = PE_LoadU32(p), 0x14u)) continue;
        if ((int32_t)PE_LoadU32(body + 0x10u) > 0) { PE_StoreU32(body + 0x10u, 0u); n++; }
    }
    return (int)n;
}

static int owned(uint32_t id)
{
    pe_addr_t inv = PE_LoadU32(GA_INV);
    uint32_t n = PE_LoadU32(GA_INV_COUNT), i;
    if (!ram(inv, 2u) || n > 512u) return 0;
    for (i = 0; i < n; i++) if (PE_LoadU16(inv + i * 2u) == id) return 1;
    return 0;
}

int pe_plat_cheats_area_kit(uint32_t lo, uint32_t hi)
{
    uint32_t a, seen[64]; int nseen = 0, added = 0;
    if (!func_80053D2C) return 0;
    for (a = lo & ~3u; a + 12u <= hi; a += 4u) {
        uint32_t h = PE_LoadU32(a), id;
        int dup = 0;
        /* op 0xA7, argc 2, arg0 mode 0 (immediate) */
        if ((h & 0x1FFFu) != 0xA7u || ((h >> 13) & 0xFu) != 2u || ((h >> 17) & 7u) != 0u) continue;
        id = PE_LoadU32(a + 8u);
        if (id < 0x100u || id >= 0x180u) continue;
        for (int i = 0; i < nseen; i++) if (seen[i] == id) dup = 1;
        if (dup) continue;
        if (nseen < 64) seen[nseen++] = id;
        if (owned(id)) continue;
        if (func_80053D2C((int)id) == 0) {
            added++;
            fprintf(stderr, "[CHEATS] area kit: added key item 0x%03X (script op at %08X)\n", id, a);
        }
    }
    return added;
}

/* The field state a warp may act in, from the main loop's own dispatch
 * (func_8001220C, bootstrap/func_8001220C_port.c): D_8009D280 reaches the
 * field room loop func_8003F3C4 unless it is the title state 0xA9400048
 * (title overlay func_801909B4: menu and attract movies), the New Game init
 * 0xAA108448, or 0xA8000048 (func_8019234C).  func_80017BB4 is a field-VM
 * op; outside that loop it has no room loop to act on (a warp picked on the
 * title latched story 0x5B into New Game init and froze it). */
#define TOK_TITLE      0xA9400048u
#define TOK_NEWGAME    0xAA108448u
#define TOK_A8000048   0xA8000048u
#define TOK_PROLOGUE   0xA80830C8u   /* New Game prologue room (FMV002) */

static int in_field(uint32_t tok)
{
    return tok != 0u && tok != TOK_TITLE && tok != TOK_NEWGAME && tok != TOK_A8000048;
}

/* 1 = transfer requested, 0 = wait (field running but busy),
 * -1 = no room_transfer linked, -2 = not in the field (refused). */
static int warp_pass(int index)
{
    const CheatWarp *w = &k_warps[index];
    uint32_t d1a0 = PE_LoadU32(GA_D1A0);
    uint32_t tok = PE_LoadU32(GA_TOKEN);
    if (!func_80017BB4) return -1;
    if (!in_field(tok)) return -2;
    if (PE_LoadU32(GA_ROOM_CUR) != tok) return 0;        /* room change in flight */
    if (d1a0 & (2u | 0x2000u)) return 0;                 /* battle / transfer pending */
    /* A field movie (New Game prologue FMV002 in A80830C8, field op 0x35)
     * owns the display while D_800B0DBA != 0: the field tick keeps calling
     * the movie overlay (func_80122040 reads its state at 0x801223F6..
     * 0x801228FA).  A room transfer now loads the destination room over that
     * overlay, and the next tick follows a garbage pointer (black screen /
     * PE_LoadU32 fatal).  Wait for func_80121A00 to clear the flag, and let
     * the prologue hand over to its first room before warping. */
    if (PE_LoadU8(GA_MOVIE) || tok == TOK_PROLOGUE) return 0;
    PE_StoreU32(GA_STORY, w->story);
    PE_StoreU32(PE_CHEAT_STACK + 8u, w->token);
    PE_StoreU32(PE_CHEAT_STACK + 0u, PE_CHEAT_STACK + 8u);
    (void)func_80017BB4(PE_CHEAT_STACK);                 /* the field VM's room_transfer */
    return 1;
}

void pe_plat_cheats_apply(void)
{
    char msg[64];
    if (!C.enabled) return;
    if (C.god) god_pass();
    if (C.ammo) ammo_pass();
    if (C.pending_win) {
        int n = win_pass();
        if (n < 0) toast("INSTANT WIN: NOT IN A BATTLE");
        else { C.wins++; snprintf(msg, sizeof msg, "INSTANT WIN: %d ENEMIES", n); toast(msg); }
        C.pending_win = 0;
    }
    if (C.pending_kit) {
        int n = pe_plat_cheats_area_kit(0x80180000u, 0x801F0000u);
        C.kits++;
        snprintf(msg, sizeof msg, "AREA KIT: %d KEY ITEMS ADDED", n);
        toast(msg);
        C.pending_kit = 0;
    }
    if (C.pending_warp) {
        int r = warp_pass(C.pending_warp - 1);
        if (r > 0) {
            C.warps++;
            snprintf(msg, sizeof msg, "WARP: %s", k_warps[C.pending_warp - 1].name);
            toast(msg);
            C.pending_warp = 0;
        } else if (r == -2) {
            /* Never carried across a mode change: the pick is dropped. */
            toast("WARP: ONLY IN THE FIELD");
            fprintf(stderr, "[CHEATS] warp refused: not in the field (token %08X)\n",
                    PE_LoadU32(GA_TOKEN));
            C.pending_warp = 0;
        } else if (r < 0) { toast("WARP UNAVAILABLE"); C.pending_warp = 0; }
        /* r == 0: wait (battle, room change or field movie in progress) */
    }
}

void pe_plat_cheats_frame(void)
{
    if (!C.inited) pe_plat_cheats_init();
    if (!C.enabled) return;
    C.frames++;
    {   /* Harness only: PE_WARP_TRACE=N logs the room loop every N presents
         * (D_8009CDA4 = field ticks, D_8009CE00 = field VM cursor). */
        static int every = -1;
        if (every < 0) every = getenv("PE_WARP_TRACE") ? atoi(getenv("PE_WARP_TRACE")) : 0;
        if (every > 0 && C.frames % every == 0)
            fprintf(stderr, "[CHEATS] trace present=%d token=%08X cur=%08X story=%08X d1a0=%08X ticks=%u vm=%08X\n",
                    C.frames, (unsigned)PE_LoadU32(GA_TOKEN), (unsigned)PE_LoadU32(GA_ROOM_CUR),
                    (unsigned)PE_LoadU32(GA_STORY), (unsigned)PE_LoadU32(GA_D1A0),
                    (unsigned)PE_LoadU32(0x8009CDA4u), (unsigned)PE_LoadU32(0x8009CE00u));
    }
    while (C.iscript < C.nscript && C.script[C.iscript].frame <= C.frames) {
        pe_plat_cheats_key(C.script[C.iscript].key, 1);
        pe_plat_cheats_key(C.script[C.iscript].key, 0);
        C.iscript++;
    }
    (void)pe_plat_mods_emit(PE_PLAT_EVENT_FRAME_TICK, (uint32_t)C.frames, 0u);
    if (C.toast_frames) C.toast_frames--;
}

/* ── overlay ───────────────────────────────────────────────────────── */
/* 5x7 font, ASCII 0x20..0x5F, one byte per column, bit 0 = top row. */
static const uint8_t k_font[64][5] = {
    {0,0,0,0,0},{0,0,0x5F,0,0},{0,7,0,7,0},{0x14,0x7F,0x14,0x7F,0x14},
    {0x24,0x2A,0x7F,0x2A,0x12},{0x23,0x13,8,0x64,0x62},{0x36,0x49,0x56,0x20,0x50},{0,8,7,3,0},
    {0,0x1C,0x22,0x41,0},{0,0x41,0x22,0x1C,0},{0x2A,0x1C,0x7F,0x1C,0x2A},{8,8,0x3E,8,8},
    {0,0x80,0x70,0x30,0},{8,8,8,8,8},{0,0,0x60,0x60,0},{0x20,0x10,8,4,2},
    {0x3E,0x51,0x49,0x45,0x3E},{0,0x42,0x7F,0x40,0},{0x72,0x49,0x49,0x49,0x46},{0x21,0x41,0x49,0x4D,0x33},
    {0x18,0x14,0x12,0x7F,0x10},{0x27,0x45,0x45,0x45,0x39},{0x3C,0x4A,0x49,0x49,0x31},{0x41,0x21,0x11,9,7},
    {0x36,0x49,0x49,0x49,0x36},{0x46,0x49,0x49,0x29,0x1E},{0,0,0x14,0,0},{0,0x40,0x34,0,0},
    {0,8,0x14,0x22,0x41},{0x14,0x14,0x14,0x14,0x14},{0,0x41,0x22,0x14,8},{2,1,0x59,9,6},
    {0x3E,0x41,0x5D,0x59,0x4E},{0x7C,0x12,0x11,0x12,0x7C},{0x7F,0x49,0x49,0x49,0x36},{0x3E,0x41,0x41,0x41,0x22},
    {0x7F,0x41,0x41,0x41,0x3E},{0x7F,0x49,0x49,0x49,0x41},{0x7F,9,9,9,1},{0x3E,0x41,0x41,0x51,0x73},
    {0x7F,8,8,8,0x7F},{0,0x41,0x7F,0x41,0},{0x20,0x40,0x41,0x3F,1},{0x7F,8,0x14,0x22,0x41},
    {0x7F,0x40,0x40,0x40,0x40},{0x7F,2,0x1C,2,0x7F},{0x7F,4,8,0x10,0x7F},{0x3E,0x41,0x41,0x41,0x3E},
    {0x7F,9,9,9,6},{0x3E,0x41,0x51,0x21,0x5E},{0x7F,9,0x19,0x29,0x46},{0x26,0x49,0x49,0x49,0x32},
    {3,1,0x7F,1,3},{0x3F,0x40,0x40,0x40,0x3F},{0x1F,0x20,0x40,0x20,0x1F},{0x3F,0x40,0x38,0x40,0x3F},
    {0x63,0x14,8,0x14,0x63},{3,4,0x78,4,3},{0x61,0x59,0x49,0x4D,0x43},{0,0x7F,0x41,0x41,0x41},
    {2,4,8,0x10,0x20},{0,0x41,0x41,0x41,0x7F},{4,2,1,2,4},{0x40,0x40,0x40,0x40,0x40},
};

static uint8_t *g_ov;
static int g_ov_w, g_ov_h;

static void put_px(int x, int y, uint8_t r, uint8_t g, uint8_t b)
{
    if (x < 0 || y < 0 || x >= g_ov_w || y >= g_ov_h) return;
    uint8_t *p = g_ov + ((size_t)y * (size_t)g_ov_w + (size_t)x) * 3u;
    p[0] = r; p[1] = g; p[2] = b;
}

static void draw_text(int x, int y, const char *s, uint8_t r, uint8_t g, uint8_t b)
{
    for (; *s; s++, x += 6) {
        int c = (unsigned char)*s;
        if (c >= 'a' && c <= 'z') c -= 32;
        if (c < 0x20 || c > 0x5F) c = '?';
        for (int col = 0; col < 5; col++) {
            uint8_t bits = k_font[c - 0x20][col];
            for (int row = 0; row < 8; row++) {
                if (bits & (1u << row)) put_px(x + col, y + row, r, g, b);
                else if (row < 7) put_px(x + col, y + row, 0, 0, 0);   /* legibility */
            }
        }
        for (int row = 0; row < 7; row++) put_px(x + 5, y + row, 0, 0, 0);
    }
}

const uint8_t *pe_plat_cheats_overlay(const uint8_t *rgb, int w, int h)
{
    int any_active, visible, y;
    static const char *const help[] = {
        "CHEATS (OPT-IN)", "F1 THIS HELP", "F2 GOD MODE", "F3 INF AMMO + FULL PE",
        "F4 INSTANT WIN", "F5 AREA KEY ITEMS", "F6 FAST-FORWARD", "F7 FF SPEED 2/4/8/MAX",
        "F8 WARP MENU", NULL};
    if (!C.enabled || !rgb || w <= 0 || h <= 0) return rgb;
    any_active = C.god || C.ammo || C.wins || C.kits || C.warps || C.ff_index;
    visible = any_active || C.help || C.menu || C.toast_frames;
    if (!visible) return rgb;
    if (!g_ov || g_ov_w != w || g_ov_h != h) {
        free(g_ov);
        g_ov = (uint8_t *)malloc((size_t)w * (size_t)h * 3u);
        if (!g_ov) return rgb;
        g_ov_w = w; g_ov_h = h;
    }
    memcpy(g_ov, rgb, (size_t)w * (size_t)h * 3u);
    if (any_active) draw_text(w - 6 * 6 - 2, 2, "CHEATS", 255, 220, 0);
    y = 2;
    if (C.help) {
        for (int i = 0; help[i]; i++, y += 9) draw_text(4, y, help[i], 255, 255, 255);
        y += 4;
    }
    if (C.menu) {
        draw_text(4, y, "WARP (UP/DOWN, ENTER, F8 CLOSES)", 120, 220, 255); y += 10;
        for (int i = 0; i < N_WARPS; i++, y += 9) {
            int sel = i == C.menu_index;
            if (sel) draw_text(4, y, ">", 255, 255, 0);
            draw_text(12, y, k_warps[i].name, sel ? 255 : 170, sel ? 255 : 170, sel ? 0 : 170);
        }
    }
    if (C.toast_frames) draw_text(4, h - 12, C.toast, 255, 255, 0);
    return g_ov;
}
