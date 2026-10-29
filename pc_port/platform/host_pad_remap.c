/* Controls remapping (host_pad_remap.h): the controls.conf parser, the
 * built-in keyboard table, and the pad / key lookups the window backends use.
 * Pure C: no X11, Win32 or SDL. */
#include "host_pad_remap.h"

#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

/* ── tables ──────────────────────────────────────────────────────────── */

typedef struct { const char *name; uint16_t bit; } ButtonName;
static const ButtonName k_buttons[] = {
    {"cross", HOST_PAD_CROSS},   {"circle", HOST_PAD_CIRCLE},
    {"square", HOST_PAD_SQUARE}, {"triangle", HOST_PAD_TRIANGLE},
    {"l1", HOST_PAD_L1},         {"r1", HOST_PAD_R1},
    {"l2", HOST_PAD_L2},         {"r2", HOST_PAD_R2},
    {"select", HOST_PAD_SELECT}, {"start", HOST_PAD_START},
    {"up", HOST_PAD_UP},         {"down", HOST_PAD_DOWN},
    {"left", HOST_PAD_LEFT},     {"right", HOST_PAD_RIGHT},
};

/* The pre-remap X11 key chain, one entry per keysym.  Letters are stored
 * lowercase: lookups fold A-Z (the old chain bound both cases of every
 * letter it used), and digits, Return, Tab and the arrows are exact. */
typedef struct { uint32_t sym; uint16_t bit; } DefaultKey;
static const DefaultKey k_default_keys[] = {
    {0xFF0D, HOST_PAD_CROSS},  {0x0020, HOST_PAD_CROSS},   /* Return, space */
    {'z', HOST_PAD_CROSS},     {'x', HOST_PAD_CROSS},
    {0xFF52, HOST_PAD_UP},     {0xFF53, HOST_PAD_RIGHT},   /* arrows */
    {0xFF54, HOST_PAD_DOWN},   {0xFF51, HOST_PAD_LEFT},
    {'c', HOST_PAD_CIRCLE},    {'v', HOST_PAD_TRIANGLE},
    {'s', HOST_PAD_SQUARE},    {'q', HOST_PAD_L1},
    {'e', HOST_PAD_R1},        {'1', HOST_PAD_L2},
    {'3', HOST_PAD_R2},        {0xFF09, HOST_PAD_SELECT},  /* Tab */
    {'p', HOST_PAD_START},
};

/* X11 keysym names beyond a single letter or digit (keysymdef.h values). */
typedef struct { const char *name; uint32_t sym; } KeyName;
static const KeyName k_keynames[] = {
    {"BackSpace", 0xFF08}, {"Tab", 0xFF09}, {"Return", 0xFF0D}, {"Escape", 0xFF1B},
    {"Delete", 0xFFFF},
    {"Home", 0xFF50}, {"Left", 0xFF51}, {"Up", 0xFF52}, {"Right", 0xFF53}, {"Down", 0xFF54},
    {"Prior", 0xFF55}, {"Page_Up", 0xFF55}, {"Next", 0xFF56}, {"Page_Down", 0xFF56},
    {"End", 0xFF57}, {"Insert", 0xFF63},
    {"KP_Enter", 0xFF8D}, {"KP_Home", 0xFF95}, {"KP_Left", 0xFF96}, {"KP_Up", 0xFF97},
    {"KP_Right", 0xFF98}, {"KP_Down", 0xFF99}, {"KP_Prior", 0xFF9A}, {"KP_Next", 0xFF9B},
    {"KP_End", 0xFF9C}, {"KP_Begin", 0xFF9D}, {"KP_Insert", 0xFF9E}, {"KP_Delete", 0xFF9F},
    {"KP_Multiply", 0xFFAA}, {"KP_Add", 0xFFAB}, {"KP_Subtract", 0xFFAD},
    {"KP_Decimal", 0xFFAE}, {"KP_Divide", 0xFFAF},
    {"KP_0", 0xFFB0}, {"KP_1", 0xFFB1}, {"KP_2", 0xFFB2}, {"KP_3", 0xFFB3}, {"KP_4", 0xFFB4},
    {"KP_5", 0xFFB5}, {"KP_6", 0xFFB6}, {"KP_7", 0xFFB7}, {"KP_8", 0xFFB8}, {"KP_9", 0xFFB9},
    {"F1", 0xFFBE}, {"F2", 0xFFBF}, {"F3", 0xFFC0}, {"F4", 0xFFC1}, {"F5", 0xFFC2},
    {"F6", 0xFFC3}, {"F7", 0xFFC4}, {"F8", 0xFFC5}, {"F9", 0xFFC6}, {"F10", 0xFFC7},
    {"F11", 0xFFC8}, {"F12", 0xFFC9},
    {"Shift_L", 0xFFE1}, {"Shift_R", 0xFFE2}, {"Control_L", 0xFFE3}, {"Control_R", 0xFFE4},
    {"Alt_L", 0xFFE9}, {"Alt_R", 0xFFEA},
    {"space", 0x20}, {"apostrophe", 0x27}, {"comma", 0x2C}, {"minus", 0x2D},
    {"period", 0x2E}, {"slash", 0x2F}, {"semicolon", 0x3B}, {"equal", 0x3D},
    {"bracketleft", 0x5B}, {"backslash", 0x5C}, {"bracketright", 0x5D}, {"grave", 0x60},
};

/* ── small helpers ───────────────────────────────────────────────────── */

static int ascii_lower(int c) { return (c >= 'A' && c <= 'Z') ? c + 32 : c; }
static uint32_t fold_sym(uint32_t s) { return (s >= 'A' && s <= 'Z') ? s + 32u : s; }

/* Case-insensitive: s[0..n) equals the NUL-terminated name. */
static int name_eq(const char *s, size_t n, const char *name)
{
    size_t i;
    for (i = 0; i < n; i++)
        if (!name[i] || ascii_lower((unsigned char)s[i]) != ascii_lower((unsigned char)name[i]))
            return 0;
    return name[n] == '\0';
}

static void trim(const char **s, size_t *n)
{
    while (*n && (**s == ' ' || **s == '\t' || **s == '\r')) { (*s)++; (*n)--; }
    while (*n && ((*s)[*n - 1] == ' ' || (*s)[*n - 1] == '\t' || (*s)[*n - 1] == '\r')) (*n)--;
}

static int popcount16(unsigned v)
{
    int n = 0;
    for (; v; v &= v - 1u) n++;
    return n;
}

static int bit_index(uint16_t bit)
{
    int i;
    for (i = 0; i < 16; i++) if (bit == (uint16_t)(1u << i)) return i;
    return 0;
}

static uint16_t button_from_name(const char *s, size_t n)
{
    size_t i;
    for (i = 0; i < sizeof k_buttons / sizeof k_buttons[0]; i++)
        if (name_eq(s, n, k_buttons[i].name)) return k_buttons[i].bit;
    return 0;
}

/* 0 = ok (*out set, A-Z folded), 1 = not a keysym we know. */
static int keysym_from_name(const char *s, size_t n, uint32_t *out)
{
    size_t i;
    if (n >= 3 && s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
        uint32_t v = 0;
        if (n > 10) return 1;                               /* 0x + 8 digits at most */
        for (i = 2; i < n; i++) {
            int c = (unsigned char)s[i], d;
            if (c >= '0' && c <= '9') d = c - '0';
            else if (c >= 'a' && c <= 'f') d = c - 'a' + 10;
            else if (c >= 'A' && c <= 'F') d = c - 'A' + 10;
            else return 1;
            v = (v << 4) | (uint32_t)d;
        }
        if (v == 0) return 1;                               /* NoSymbol */
        *out = fold_sym(v);
        return 0;
    }
    if (n == 1) {
        int c = (unsigned char)s[0];
        if (c >= '0' && c <= '9') { *out = (uint32_t)c; return 0; }
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
            *out = (uint32_t)ascii_lower(c);
            return 0;
        }
        return 1;
    }
    for (i = 0; i < sizeof k_keynames / sizeof k_keynames[0]; i++)
        if (name_eq(s, n, k_keynames[i].name)) { *out = fold_sym(k_keynames[i].sym); return 0; }
    return 1;
}

/* ── key table edits ─────────────────────────────────────────────────── */

static void key_remove_bit(HostPadRemap *r, uint16_t bit)
{
    int i, j = 0;
    for (i = 0; i < r->nkey; i++)
        if (r->key_bit[i] != bit) {
            r->key_sym[j] = r->key_sym[i];
            r->key_bit[j] = r->key_bit[i];
            j++;
        }
    r->nkey = j;
}

static void key_remove_sym(HostPadRemap *r, uint32_t sym)
{
    int i, j = 0;
    for (i = 0; i < r->nkey; i++)
        if (r->key_sym[i] != sym) {
            r->key_sym[j] = r->key_sym[i];
            r->key_bit[j] = r->key_bit[i];
            j++;
        }
    r->nkey = j;
}

static int key_add(HostPadRemap *r, uint32_t sym, uint16_t bit)
{
    if (r->nkey >= HOST_PAD_REMAP_MAX_KEYS) return 0;
    r->key_sym[r->nkey] = sym;
    r->key_bit[r->nkey] = bit;
    r->nkey++;
    return 1;
}

void HostPadRemap_Defaults(HostPadRemap *r)
{
    size_t i;
    memset(r, 0, sizeof *r);
    for (i = 0; i < 16; i++) r->pad_dest[i] = (uint16_t)(1u << i);
    for (i = 0; i < sizeof k_default_keys / sizeof k_default_keys[0]; i++)
        (void)key_add(r, k_default_keys[i].sym, k_default_keys[i].bit);
}

/* ── parser ──────────────────────────────────────────────────────────── */

static int clip(size_t n) { return (int)(n > 40 ? 40 : n); }

static void warn(HostPadRemap *r, FILE *log, const char *src, int line, const char *fmt, ...)
{
    va_list ap;
    r->warnings++;
    if (!log) return;
    fprintf(log, "[CONTROLS] %s:%d: ", src, line);
    va_start(ap, fmt);
    vfprintf(log, fmt, ap);
    va_end(ap);
    fputc('\n', log);
}

static void parse_pad(HostPadRemap *r, FILE *log, const char *src, int line,
                      const char *sn, size_t sl, const char *dn, size_t dl)
{
    uint16_t from = button_from_name(sn, sl), to = button_from_name(dn, dl);
    if (!from) {
        warn(r, log, src, line, "unknown PS1 button '%.*s' in pad.<button>, ignored", clip(sl), sn);
        return;
    }
    if (!to) {
        warn(r, log, src, line, "unknown PS1 button '%.*s' as the value of pad.%.*s, ignored",
             clip(dl), dn, clip(sl), sn);
        return;
    }
    if (r->pad_set & from)
        warn(r, log, src, line, "duplicate pad.%.*s: replaces the earlier line", clip(sl), sn);
    r->pad_dest[bit_index(from)] = to;
    r->pad_set = (uint16_t)(r->pad_set | from);
}

static void parse_key(HostPadRemap *r, FILE *log, const char *src, int line,
                      const char *bn, size_t bl, const char *vp, size_t vl)
{
    uint16_t btn = button_from_name(bn, bl);
    uint32_t syms[HOST_PAD_REMAP_KEYS_PER_LINE];
    int nsyms = 0, i;
    if (!btn) {
        warn(r, log, src, line, "unknown PS1 button '%.*s' in key.<button>, ignored", clip(bl), bn);
        return;
    }
    /* Validate the whole list first: one bad name ignores the line. */
    for (;;) {
        const char *comma = memchr(vp, ',', vl);
        size_t seg = comma ? (size_t)(comma - vp) : vl;
        const char *tok = vp;
        size_t tl = seg;
        uint32_t ks;
        trim(&tok, &tl);
        if (tl == 0) {
            warn(r, log, src, line, "empty key name in key.%.*s, ignored", clip(bl), bn);
            return;
        }
        if (nsyms == HOST_PAD_REMAP_KEYS_PER_LINE) {
            warn(r, log, src, line, "too many keys for key.%.*s (at most %d), ignored",
                 clip(bl), bn, HOST_PAD_REMAP_KEYS_PER_LINE);
            return;
        }
        if (keysym_from_name(tok, tl, &ks)) {
            warn(r, log, src, line, "unknown key name '%.*s' in key.%.*s, ignored",
                 clip(tl), tok, clip(bl), bn);
            return;
        }
        if (HostPadRemap_KeyReserved(ks)) {
            warn(r, log, src, line,
                 "'%.*s' is reserved (Esc and F1-F8 stay with the menus), key.%.*s ignored",
                 clip(tl), tok, clip(bl), bn);
            return;
        }
        syms[nsyms++] = ks;
        if (!comma) break;
        vp += seg + 1;
        vl -= seg + 1;
    }
    if (r->key_set & btn)
        warn(r, log, src, line, "duplicate key.%.*s: replaces the earlier line", clip(bl), bn);
    /* The button's old keys go (its defaults, or an earlier line's), and each
     * listed key leaves whichever button had it: a key drives one button. */
    key_remove_bit(r, btn);
    for (i = 0; i < nsyms; i++) {
        key_remove_sym(r, syms[i]);
        (void)key_add(r, syms[i], btn);
    }
    r->key_set = (uint16_t)(r->key_set | btn);
}

static void parse_line(HostPadRemap *r, FILE *log, const char *src, int line,
                       const char *s, size_t n)
{
    const char *eq, *lhs, *rhs, *hash = memchr(s, '#', n);
    size_t lhs_n, rhs_n;
    if (hash) n = (size_t)(hash - s);
    trim(&s, &n);
    if (n == 0) return;
    if (n >= HOST_PAD_REMAP_LINE_MAX) {
        warn(r, log, src, line, "line longer than %d bytes, ignored", HOST_PAD_REMAP_LINE_MAX - 1);
        return;
    }
    if (memchr(s, 0, n)) {
        warn(r, log, src, line, "NUL byte in line, ignored");
        return;
    }
    eq = memchr(s, '=', n);
    if (!eq) {
        warn(r, log, src, line, "expected <setting>=<value>, ignored");
        return;
    }
    lhs = s;
    lhs_n = (size_t)(eq - s);
    rhs = eq + 1;
    rhs_n = n - lhs_n - 1;
    trim(&lhs, &lhs_n);
    trim(&rhs, &rhs_n);
    if (lhs_n > 4 && name_eq(lhs, 4, "pad."))
        parse_pad(r, log, src, line, lhs + 4, lhs_n - 4, rhs, rhs_n);
    else if (lhs_n > 4 && name_eq(lhs, 4, "key."))
        parse_key(r, log, src, line, lhs + 4, lhs_n - 4, rhs, rhs_n);
    else
        warn(r, log, src, line, "unknown setting '%.*s' (want pad.<button> or key.<button>), ignored",
             clip(lhs_n), lhs);
}

int HostPadRemap_Parse(HostPadRemap *r, const char *text, size_t len, const char *src, FILE *log)
{
    size_t pos = 0;
    int line = 0, i;
    HostPadRemap_Defaults(r);
    if (!text) len = 0;
    if (!src) src = "controls";
    if (len >= 3 && memcmp(text, "\xEF\xBB\xBF", 3) == 0) pos = 3;    /* UTF-8 BOM */
    while (pos < len) {
        const char *ls = text + pos;
        const char *nl = memchr(ls, '\n', len - pos);
        size_t ll = nl ? (size_t)(nl - ls) : len - pos;
        pos += nl ? ll + 1 : ll;
        line++;
        parse_line(r, log, src, line, ls, ll);
    }
    for (i = 0; i < 16; i++)
        if (r->pad_dest[i] != (uint16_t)(1u << i)) r->pad_active = 1;
    r->loaded = popcount16(r->pad_set) + popcount16(r->key_set);
    return r->loaded;
}

/* ── file and process-wide config ────────────────────────────────────── */

int HostPadRemap_ConfigPath(char *out, size_t cap)
{
    const char *p = getenv("PE_CONTROLS_CONFIG");
    const char *x;
    int n;
    if (!out || cap == 0) return 0;
    if (p && *p) n = snprintf(out, cap, "%s", p);
    else {
        n = -1;
#ifdef _WIN32
        x = getenv("APPDATA");
        if (x && *x) n = snprintf(out, cap, "%s\\parasite-eve-port\\controls.conf", x);
        else
#endif
        {
            x = getenv("XDG_CONFIG_HOME");
            if (x && *x) n = snprintf(out, cap, "%s/parasite-eve-port/controls.conf", x);
            else {
                x = getenv("HOME");
                if (x && *x) n = snprintf(out, cap, "%s/.config/parasite-eve-port/controls.conf", x);
            }
        }
    }
    return n >= 0 && (size_t)n < cap;
}

int HostPadRemap_LoadFile(HostPadRemap *r, const char *path, FILE *log)
{
    FILE *f;
    char *buf;
    size_t n;
    HostPadRemap_Defaults(r);
    if (!path || !*path) return 0;
    f = fopen(path, "rb");
    if (!f) {
        if (errno != ENOENT && errno != ENOTDIR) {
            r->warnings++;
            if (log) fprintf(log, "[CONTROLS] cannot open %s: %s\n", path, strerror(errno));
        }
        return 0;
    }
    buf = (char *)malloc(HOST_PAD_REMAP_FILE_MAX + 1);
    if (!buf) { fclose(f); return 0; }
    n = fread(buf, 1, HOST_PAD_REMAP_FILE_MAX + 1, f);
    if (ferror(f)) {
        r->warnings++;
        if (log) fprintf(log, "[CONTROLS] cannot read %s: %s\n", path, strerror(errno));
        free(buf);
        fclose(f);
        return 0;
    }
    fclose(f);
    if (n > HOST_PAD_REMAP_FILE_MAX) {
        r->warnings++;
        if (log) fprintf(log, "[CONTROLS] %s is larger than %d bytes, ignored\n",
                         path, HOST_PAD_REMAP_FILE_MAX);
        free(buf);
        return 0;
    }
    (void)HostPadRemap_Parse(r, buf, n, path, log);
    free(buf);
    if (log) fprintf(log, "[CONTROLS] loaded %d remaps from %s\n", r->loaded, path);
    return 1;
}

static HostPadRemap g_remap;
static int g_remap_ready;

const HostPadRemap *HostPadRemap_Global(void)
{
    if (!g_remap_ready) { HostPadRemap_Defaults(&g_remap); g_remap_ready = 1; }
    return &g_remap;
}

void HostPadRemap_Reset(void)
{
    HostPadRemap_Defaults(&g_remap);
    g_remap_ready = 1;
}

int HostPadRemap_Init(void)
{
    char path[1024];
    HostPadRemap_Reset();
    if (!HostPadRemap_ConfigPath(path, sizeof path)) return 0;
    return HostPadRemap_LoadFile(&g_remap, path, stderr);
}

/* ── lookups ─────────────────────────────────────────────────────────── */

uint16_t HostPadRemap_Pad(const HostPadRemap *r, uint16_t held)
{
    uint16_t out = 0;
    int i;
    if (!r || !r->pad_active) return held;
    for (i = 0; i < 16; i++)
        if (held & (1u << i)) out = (uint16_t)(out | r->pad_dest[i]);
    return out;
}

uint16_t HostPadRemap_PadWord(const HostPadRemap *r, uint16_t keys_held,
                              uint16_t controller_filtered)
{
    return (uint16_t)~(unsigned)(keys_held | HostPadRemap_Pad(r, controller_filtered));
}

int HostPadRemap_KeyReserved(unsigned long keysym)
{
    return keysym == 0xFF1Bul || (keysym >= 0xFFBEul && keysym <= 0xFFC5ul);
}

uint16_t HostPadRemap_Key(const HostPadRemap *r, unsigned long keysym)
{
    uint32_t ks;
    int i;
    if (!r || keysym == 0 || (uint64_t)keysym > 0xFFFFFFFFu || HostPadRemap_KeyReserved(keysym))
        return 0;
    ks = fold_sym((uint32_t)keysym);
    for (i = 0; i < r->nkey; i++)
        if (r->key_sym[i] == ks) return r->key_bit[i];
    return 0;
}

/* An explicit table: Windows VK codes overlap ASCII (VK_F1 == 'p', VK_NUMPAD1
 * == 'a', VK_INSERT == '-'), so a code is never passed through as a keysym.
 * Modifier and lock keys are not bindable on Windows. */
unsigned long HostPadRemap_KeysymFromWin32Vk(unsigned vk)
{
    if (vk >= 0x41u && vk <= 0x5Au) return vk + 32u;              /* 'A'..'Z' -> a..z */
    if (vk >= 0x30u && vk <= 0x39u) return vk;                    /* '0'..'9' */
    if (vk >= 0x60u && vk <= 0x69u) return 0xFFB0ul + (vk - 0x60u); /* VK_NUMPAD0.. -> KP_0.. */
    if (vk >= 0x70u && vk <= 0x7Bu) return 0xFFBEul + (vk - 0x70u); /* VK_F1..VK_F12 */
    switch (vk) {
    case 0x08: return 0xFF08;   /* VK_BACK */
    case 0x09: return 0xFF09;   /* VK_TAB */
    case 0x0D: return 0xFF0D;   /* VK_RETURN */
    case 0x1B: return 0xFF1B;   /* VK_ESCAPE */
    case 0x20: return 0x0020;   /* VK_SPACE */
    case 0x21: return 0xFF55;   /* VK_PRIOR */
    case 0x22: return 0xFF56;   /* VK_NEXT */
    case 0x23: return 0xFF57;   /* VK_END */
    case 0x24: return 0xFF50;   /* VK_HOME */
    case 0x25: return 0xFF51;   /* VK_LEFT */
    case 0x26: return 0xFF52;   /* VK_UP */
    case 0x27: return 0xFF53;   /* VK_RIGHT */
    case 0x28: return 0xFF54;   /* VK_DOWN */
    case 0x2D: return 0xFF63;   /* VK_INSERT */
    case 0x2E: return 0xFFFF;   /* VK_DELETE */
    case 0x6A: return 0xFFAA;   /* VK_MULTIPLY */
    case 0x6B: return 0xFFAB;   /* VK_ADD */
    case 0x6D: return 0xFFAD;   /* VK_SUBTRACT */
    case 0x6E: return 0xFFAE;   /* VK_DECIMAL */
    case 0x6F: return 0xFFAF;   /* VK_DIVIDE */
    case 0xBA: return 0x3B;     /* VK_OEM_1      ; */
    case 0xBB: return 0x3D;     /* VK_OEM_PLUS   = */
    case 0xBC: return 0x2C;     /* VK_OEM_COMMA  , */
    case 0xBD: return 0x2D;     /* VK_OEM_MINUS  - */
    case 0xBE: return 0x2E;     /* VK_OEM_PERIOD . */
    case 0xBF: return 0x2F;     /* VK_OEM_2      / */
    case 0xC0: return 0x60;     /* VK_OEM_3      ` */
    case 0xDB: return 0x5B;     /* VK_OEM_4      [ */
    case 0xDC: return 0x5C;     /* VK_OEM_5      \ */
    case 0xDD: return 0x5D;     /* VK_OEM_6      ] */
    case 0xDE: return 0x27;     /* VK_OEM_7      ' */
    default:   return 0;
    }
}
