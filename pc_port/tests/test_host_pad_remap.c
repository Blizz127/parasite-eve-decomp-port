/* Controls remapping (platform/host_pad_remap.c): the controls.conf parser
 * (valid, invalid and duplicate lines), identity with no file over all 65536
 * pad words, a confirm/cancel swap, the built-in key table against a verbatim
 * copy of the old X11 if-chain, key overrides, keys that can never be bound
 * (Esc, F1-F8), the Win32 virtual-key table against the old switch, and the
 * composition with the real dev-menu combo.  Pure: no window, no hardware. */
#define _GNU_SOURCE
#include "host_pad_remap.h"
#include "host_pad_combo.h"
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static int g_fail;
#define CHECK(c) do { if (!(c)) { fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #c); g_fail = 1; } } while (0)

#define ALL_BUTTONS (HOST_PAD_SELECT | HOST_PAD_START | HOST_PAD_UP | HOST_PAD_DOWN | \
                     HOST_PAD_LEFT | HOST_PAD_RIGHT | HOST_PAD_L1 | HOST_PAD_R1 | \
                     HOST_PAD_L2 | HOST_PAD_R2 | HOST_PAD_TRIANGLE | HOST_PAD_CIRCLE | \
                     HOST_PAD_CROSS | HOST_PAD_SQUARE)
#define SWEEP_KEYS 0x30000ul   /* keysyms 0 .. SWEEP_KEYS-1 are swept (covers 0xFFxx too) */

/* ── a captured warning log ──────────────────────────────────────────── */

typedef struct { FILE *f; char *buf; size_t len; } Log;
static void log_open(Log *l)
{
    l->buf = NULL; l->len = 0;
    l->f = open_memstream(&l->buf, &l->len);
    CHECK(l->f != NULL);
}
static int log_close(Log *l)            /* lines written; l->buf is the text */
{
    int n = 0;
    size_t i;
    fclose(l->f);
    for (i = 0; i < l->len; i++) if (l->buf[i] == '\n') n++;
    return n;
}
static void log_free(Log *l) { free(l->buf); l->buf = NULL; }

/* Parse text (len bytes) with a captured log; returns the log's line count. */
static int parse_n(HostPadRemap *r, const char *text, size_t len, Log *l)
{
    log_open(l);
    (void)HostPadRemap_Parse(r, text, len, "test", l->f);
    return log_close(l);
}
static int parse(HostPadRemap *r, const char *text, Log *l) { return parse_n(r, text, strlen(text), l); }

/* ── references ──────────────────────────────────────────────────────── */

/* Verbatim copy of the X11 if-chain in host_window.c before the table (the
 * Escape/F-key branches before it never reached the chain). */
static uint16_t old_chain(unsigned long ks)
{
    uint16_t bit = 0;
    if (ks == 0xFF0D || ks == 0x0020 || ks == 0x007A || ks == 0x0078 || ks == 0x005A || ks == 0x0058)
        bit = 0x4000u;
    else if (ks == 0xFF52) bit = 0x0010u;
    else if (ks == 0xFF53) bit = 0x0020u;
    else if (ks == 0xFF54) bit = 0x0040u;
    else if (ks == 0xFF51) bit = 0x0080u;
    else if (ks == 'c' || ks == 'C') bit = 0x2000u;
    else if (ks == 'v' || ks == 'V') bit = 0x1000u;
    else if (ks == 's' || ks == 'S') bit = 0x8000u;
    else if (ks == 'q' || ks == 'Q') bit = 0x0400u;
    else if (ks == 'e' || ks == 'E') bit = 0x0800u;
    else if (ks == '1') bit = 0x0100u;
    else if (ks == '3') bit = 0x0200u;
    else if (ks == 0xFF09u) bit = 0x0001u;
    else if (ks == 'p' || ks == 'P') bit = 0x0008u;
    return bit;
}

/* Verbatim copy of host_window_win32.c's vk_to_pad_bit before the table, with
 * the numeric VK_* values. */
static uint16_t old_vk_to_pad_bit(unsigned vk)
{
    switch (vk) {
    case 0x0D: case 0x20: case 'Z': case 'X':
        return 0x4000u;                       /* VK_RETURN, VK_SPACE */
    case 0x26: return 0x0010u;                /* VK_UP */
    case 0x27: return 0x0020u;                /* VK_RIGHT */
    case 0x28: return 0x0040u;                /* VK_DOWN */
    case 0x25: return 0x0080u;                /* VK_LEFT */
    case 'C':  return 0x2000u;
    case 'V':  return 0x1000u;
    case 'S':  return 0x8000u;
    case 'Q':  return 0x0400u;
    case 'E':  return 0x0800u;
    case '1':  return 0x0100u;
    case '3':  return 0x0200u;
    case 0x09: return 0x0001u;                /* VK_TAB */
    case 'P':  return 0x0008u;
    default:   return 0u;
    }
}

/* ── invariants every parse result must keep ─────────────────────────── */

static int single_button(uint16_t b) { return b && !(b & (b - 1u)) && (b & ALL_BUTTONS); }

static int invariants_ok(const HostPadRemap *r)
{
    int i, j;
    if (r->nkey < 0 || r->nkey > HOST_PAD_REMAP_MAX_KEYS) return 0;
    for (i = 0; i < r->nkey; i++) {
        if (!single_button(r->key_bit[i])) return 0;
        if (r->key_sym[i] == 0 || HostPadRemap_KeyReserved(r->key_sym[i])) return 0;
        if (r->key_sym[i] >= 'A' && r->key_sym[i] <= 'Z') return 0;     /* folded */
        for (j = 0; j < i; j++) if (r->key_sym[j] == r->key_sym[i]) return 0;
    }
    for (i = 0; i < 16; i++) {
        uint16_t bit = (uint16_t)(1u << i);
        if (bit & ALL_BUTTONS) { if (!single_button(r->pad_dest[i])) return 0; }
        else if (r->pad_dest[i] != bit) return 0;   /* unnamed bits always identity */
    }
    return 1;
}

static int same_as_defaults(const HostPadRemap *r)
{
    HostPadRemap d;
    HostPadRemap_Defaults(&d);
    return r->nkey == d.nkey && r->pad_active == d.pad_active && r->pad_set == d.pad_set &&
           r->key_set == d.key_set && r->loaded == d.loaded &&
           memcmp(r->pad_dest, d.pad_dest, sizeof d.pad_dest) == 0 &&
           memcmp(r->key_sym, d.key_sym, sizeof(uint32_t) * (size_t)d.nkey) == 0 &&
           memcmp(r->key_bit, d.key_bit, sizeof(uint16_t) * (size_t)d.nkey) == 0;
}

/* ── temp files ──────────────────────────────────────────────────────── */

static void make_tmpdir(char *dir, size_t cap)
{
    const char *t = getenv("TMPDIR");
    snprintf(dir, cap, "%s/pe-remap-XXXXXX", t && *t ? t : "/tmp");
    if (!mkdtemp(dir)) { perror("mkdtemp"); exit(2); }
}
static void write_file(const char *path, const char *data, size_t len)
{
    FILE *f = fopen(path, "wb");
    if (!f) { perror(path); exit(2); }
    if (len && fwrite(data, 1, len, f) != len) { perror("fwrite"); exit(2); }
    fclose(f);
}
static void write_str(const char *path, const char *s) { write_file(path, s, strlen(s)); }

/* ── parser ──────────────────────────────────────────────────────────── */

static void test_parser_valid(void)
{
    HostPadRemap r;
    Log l;
    static const char text[] =
        "# controls\n"
        "\n"
        "   \t \n"
        "pad.cross = circle\n"
        "PAD.Circle=Cross   # trailing comment\n"
        "  pad.l1=r1\n"
        "key.start=Return , space\n"
        "key.select=0x61,F9\n"
        "key.up=w\r\n"
        "key.R2=Shift_L,KP_Left\n";                         /* no final blank line needed */
    CHECK(parse(&r, text, &l) == 0);
    CHECK(r.warnings == 0 && l.len == 0);
    log_free(&l);
    CHECK(r.pad_set == (HOST_PAD_CROSS | HOST_PAD_CIRCLE | HOST_PAD_L1));
    CHECK(r.key_set == (HOST_PAD_START | HOST_PAD_SELECT | HOST_PAD_UP | HOST_PAD_R2));
    CHECK(r.loaded == 7 && r.pad_active);
    CHECK(HostPadRemap_Pad(&r, HOST_PAD_CROSS) == HOST_PAD_CIRCLE);
    CHECK(HostPadRemap_Pad(&r, HOST_PAD_CIRCLE) == HOST_PAD_CROSS);
    CHECK(HostPadRemap_Pad(&r, HOST_PAD_L1) == HOST_PAD_R1);
    CHECK(HostPadRemap_Pad(&r, HOST_PAD_R1) == HOST_PAD_R1);          /* only l1 was set */
    CHECK(HostPadRemap_Key(&r, 0xFF0D) == HOST_PAD_START);            /* taken from Cross */
    CHECK(HostPadRemap_Key(&r, ' ') == HOST_PAD_START);
    CHECK(HostPadRemap_Key(&r, 'z') == HOST_PAD_CROSS);               /* Cross keeps the rest */
    CHECK(HostPadRemap_Key(&r, 'x') == HOST_PAD_CROSS);
    CHECK(HostPadRemap_Key(&r, 0x61) == HOST_PAD_SELECT);
    CHECK(HostPadRemap_Key(&r, 0xFFC6) == HOST_PAD_SELECT);           /* F9 */
    CHECK(HostPadRemap_Key(&r, 'w') == HOST_PAD_UP && HostPadRemap_Key(&r, 'W') == HOST_PAD_UP);
    CHECK(HostPadRemap_Key(&r, 0xFF52) == 0);                         /* the arrow lost Up */
    CHECK(HostPadRemap_Key(&r, 0xFFE1) == HOST_PAD_R2);               /* Shift_L */
    CHECK(HostPadRemap_Key(&r, 0xFF96) == HOST_PAD_R2);               /* KP_Left */
    CHECK(HostPadRemap_Key(&r, '3') == 0);                            /* R2's default key replaced */
    CHECK(HostPadRemap_Key(&r, 0xFF09) == 0);                         /* Select's default Tab replaced */
    CHECK(invariants_ok(&r));

    /* A UTF-8 BOM and CRLF line ends are fine; NULL/empty text is the defaults. */
    CHECK(parse(&r, "\xEF\xBB\xBFpad.cross=circle\r\npad.circle=cross\r\n", &l) == 0);
    CHECK(r.loaded == 2 && r.warnings == 0 && HostPadRemap_Pad(&r, HOST_PAD_CROSS) == HOST_PAD_CIRCLE);
    log_free(&l);
    HostPadRemap_Parse(&r, NULL, 0, NULL, NULL);
    CHECK(same_as_defaults(&r) && r.loaded == 0);
    HostPadRemap_Parse(&r, "", 0, "x", NULL);
    CHECK(same_as_defaults(&r));
}

static void test_parser_invalid(void)
{
    static const char *const bad[] = {
        "pad.cross",                  /* no '=' */
        "pad.cross=",                 /* empty value */
        "pad.cross=banana",
        "pad.banana=cross",
        "pad.=cross",
        "key.cross",
        "key.cross=",
        "key.cross=z,",               /* trailing comma */
        "key.cross=,z",
        "key.cross=z,,x",
        "key.cross=NoSuchKey",
        "key.cross=z,NoSuchKey",      /* one bad name ignores the whole line */
        "key.cross=Escape",           /* reserved, by name */
        "key.cross=F1", "key.cross=F3", "key.cross=f6", "key.cross=F8",
        "key.cross=0xFF1B",           /* reserved, by hex */
        "key.cross=0xffc3",
        "key.cross=z,F1",
        "key.cross=0x0",              /* NoSymbol */
        "key.cross=0x",
        "key.cross=0xZZ",
        "key.cross=0x123456789",      /* more than 8 hex digits */
        "key.cross=;",                /* punctuation has names (semicolon) */
        "key.banana=z",
        "volume=11",
        "=cross",
        "cross=circle",
        "key.cross=a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q",    /* 17 keys */
    };
    HostPadRemap r;
    Log l;
    char longline[700];
    char nul[] = "pad.cross=cir\0cle\n";
    size_t i;
    for (i = 0; i < sizeof bad / sizeof bad[0]; i++) {
        char text[256];
        snprintf(text, sizeof text, "%s\n", bad[i]);
        CHECK(parse(&r, text, &l) == 1);                          /* exactly one warning line */
        CHECK(r.warnings == 1 && r.loaded == 0 && same_as_defaults(&r) && invariants_ok(&r));
        CHECK(strncmp(l.buf, "[CONTROLS] test:1: ", 19) == 0);
        if (strncmp(l.buf, "[CONTROLS] test:1: ", 19) != 0) fprintf(stderr, "  line: %s\n", bad[i]);
        log_free(&l);
    }
    memcpy(longline, "key.cross=", 10);
    memset(longline + 10, 'a', 600);                                /* 610-byte line */
    longline[610] = '\n';
    CHECK(parse_n(&r, longline, 611, &l) == 1 && r.warnings == 1 && same_as_defaults(&r));
    log_free(&l);
    CHECK(parse_n(&r, nul, sizeof nul - 1, &l) == 1 && r.warnings == 1 && same_as_defaults(&r));
    log_free(&l);
    /* A long comment is not a long line. */
    memset(longline, 'x', sizeof longline);
    longline[0] = '#';
    longline[699] = '\n';
    CHECK(parse_n(&r, longline, 700, &l) == 0 && r.warnings == 0 && same_as_defaults(&r));
    log_free(&l);

    /* Good lines around bad ones still apply; the warning names the line. */
    CHECK(parse(&r, "pad.cross=circle\nbanana\nkey.l1=a\npad.zzz=cross\n", &l) == 2);
    CHECK(r.loaded == 2 && r.warnings == 2 && HostPadRemap_Pad(&r, HOST_PAD_CROSS) == HOST_PAD_CIRCLE);
    CHECK(HostPadRemap_Key(&r, 'a') == HOST_PAD_L1);
    CHECK(strstr(l.buf, "test:2: ") && strstr(l.buf, "test:4: ") && !strstr(l.buf, "test:1: ") &&
          !strstr(l.buf, "test:3: "));
    log_free(&l);

    /* The warning for an unknown name quotes it. */
    parse(&r, "key.cross=Hyper_Z\n", &l);
    CHECK(strstr(l.buf, "'Hyper_Z'") != NULL);
    log_free(&l);
}

static void test_parser_duplicates(void)
{
    HostPadRemap r;
    Log l;
    unsigned long ks;

    /* a repeated pad.<x> line replaces the earlier one and warns once */
    CHECK(parse(&r, "pad.cross=circle\npad.cross=square\n", &l) == 1);
    CHECK(strstr(l.buf, "test:2: duplicate pad.cross") != NULL);
    CHECK(r.warnings == 1 && r.loaded == 1);
    CHECK(HostPadRemap_Pad(&r, HOST_PAD_CROSS) == HOST_PAD_SQUARE);
    log_free(&l);

    /* a repeated key.<x> line replaces the earlier one entirely */
    CHECK(parse(&r, "key.cross=a\nkey.cross=b\n", &l) == 1);
    CHECK(strstr(l.buf, "test:2: duplicate key.cross") != NULL && r.loaded == 1);
    CHECK(HostPadRemap_Key(&r, 'a') == 0 && HostPadRemap_Key(&r, 'b') == HOST_PAD_CROSS);
    CHECK(HostPadRemap_Key(&r, 0xFF0D) == 0 && HostPadRemap_Key(&r, 'z') == 0);   /* defaults stay gone */
    CHECK(invariants_ok(&r));
    log_free(&l);

    /* case and spacing do not hide a duplicate */
    CHECK(parse(&r, "pad.Cross=circle\n PAD.CROSS = square\n", &l) == 1 && r.loaded == 1);
    log_free(&l);

    /* the same key in a list twice binds once */
    CHECK(parse(&r, "key.cross=z,Z,z\n", &l) == 0 && r.loaded == 1 && invariants_ok(&r));
    CHECK(HostPadRemap_Key(&r, 'z') == HOST_PAD_CROSS && r.nkey == 17 - 4 + 1);
    log_free(&l);

    /* a key drives one button: an explicit binding takes it from another
     * button's default (circle loses 'c'), whatever the line order */
    CHECK(parse(&r, "key.cross=c\n", &l) == 0);
    CHECK(HostPadRemap_Key(&r, 'c') == HOST_PAD_CROSS && HostPadRemap_Key(&r, 'C') == HOST_PAD_CROSS);
    for (ks = 0; ks < SWEEP_KEYS; ks++) {
        uint16_t want = (ks == 'c' || ks == 'C') ? HOST_PAD_CROSS : old_chain(ks);
        if (old_chain(ks) == HOST_PAD_CROSS && !(ks == 'c' || ks == 'C')) want = 0;   /* Cross's defaults replaced */
        if (want != HostPadRemap_Key(&r, ks)) { CHECK(!"key.cross=c table"); break; }
    }
    log_free(&l);

    /* between two explicit lines the later one wins the key; the earlier
     * button is left with whatever it still has (here nothing) */
    CHECK(parse(&r, "key.cross=z\nkey.circle=z\n", &l) == 0);
    CHECK(HostPadRemap_Key(&r, 'z') == HOST_PAD_CIRCLE && r.loaded == 2);
    for (ks = 0; ks < SWEEP_KEYS; ks++)
        if (HostPadRemap_Key(&r, ks) == HOST_PAD_CROSS) { CHECK(!"cross keeps a key"); break; }
    log_free(&l);

    /* line order between explicit lines on different keys does not matter */
    {
        HostPadRemap a, b;
        Log la, lb;
        parse(&a, "key.cross=c\nkey.circle=v\n", &la);
        parse(&b, "key.circle=v\nkey.cross=c\n", &lb);
        for (ks = 0; ks < SWEEP_KEYS; ks++)
            if (HostPadRemap_Key(&a, ks) != HostPadRemap_Key(&b, ks)) { CHECK(!"order independent"); break; }
        CHECK(HostPadRemap_Key(&a, 'v') == HOST_PAD_CIRCLE);          /* taken from Triangle's default */
        log_free(&la); log_free(&lb);
    }
}

/* ── identity ────────────────────────────────────────────────────────── */

static void sweep_identity(const HostPadRemap *r)
{
    unsigned w;
    for (w = 0; w < 0x10000u; w++) {
        if (HostPadRemap_Pad(r, (uint16_t)w) != (uint16_t)w) { CHECK(!"Pad identity"); return; }
        if (HostPadRemap_PadWord(r, 0, (uint16_t)w) != (uint16_t)~w) { CHECK(!"PadWord identity"); return; }
        if (HostPadRemap_PadWord(r, (uint16_t)w, 0) != (uint16_t)~w) { CHECK(!"PadWord keys identity"); return; }
        if (HostPadRemap_PadWord(r, (uint16_t)(w * 7u), (uint16_t)w) !=
            (uint16_t)~((w * 7u) | w)) { CHECK(!"PadWord merge identity"); return; }
    }
}

static void sweep_default_keys(const HostPadRemap *r)
{
    static const unsigned long extra[] = { 0x1000000ul, 0x1000041ul, 0x100FFFFul, 0x110FFFFul,
                                           0xFFFFFFFFul,
#if ULONG_MAX > 0xFFFFFFFFul
                                           0xFFFFFFFFFul, 0x100000041ul,   /* wider than a 32-bit keysym */
#endif
    };
    unsigned long ks;
    size_t i;
    for (ks = 0; ks < SWEEP_KEYS; ks++)
        if (HostPadRemap_Key(r, ks) != old_chain(ks)) {
            fprintf(stderr, "key 0x%lX: table %04X old chain %04X\n", ks, HostPadRemap_Key(r, ks), old_chain(ks));
            CHECK(!"key table equals the old chain");
            return;
        }
    for (i = 0; i < sizeof extra / sizeof extra[0]; i++) CHECK(HostPadRemap_Key(r, extra[i]) == 0);
}

static void test_identity_no_file(void)
{
    char dir[512], sub[700], path[1024];
    HostPadRemap d;
    const HostPadRemap *g;
    struct stat st;

    /* before any init the process-wide config is identity (nothing read) */
    g = HostPadRemap_Global();
    CHECK(g != NULL && same_as_defaults(g) && g->loaded == 0);
    sweep_identity(g);

    /* the real load path with a missing file: identity, nothing created */
    make_tmpdir(dir, sizeof dir);
    snprintf(sub, sizeof sub, "%s/config-dir", dir);
    snprintf(path, sizeof path, "%s/controls.conf", sub);
    setenv("PE_CONTROLS_CONFIG", path, 1);
    CHECK(HostPadRemap_Init() == 0);
    g = HostPadRemap_Global();
    CHECK(same_as_defaults(g) && g->loaded == 0 && g->warnings == 0 && !g->pad_active);
    sweep_identity(g);
    sweep_default_keys(g);
    CHECK(stat(path, &st) != 0 && errno == ENOENT);                  /* queries never create the file ... */
    CHECK(stat(sub, &st) != 0 && errno == ENOENT);                   /* ... or its directory */
    unsetenv("PE_CONTROLS_CONFIG");

    /* the XDG route (a config dir that does not exist yet) is the same */
    setenv("XDG_CONFIG_HOME", sub, 1);
    CHECK(HostPadRemap_Init() == 0 && same_as_defaults(HostPadRemap_Global()));
    CHECK(stat(sub, &st) != 0 && errno == ENOENT);
    unsetenv("XDG_CONFIG_HOME");

    /* the defaults struct and a comment-only file are identity too */
    HostPadRemap_Defaults(&d);
    sweep_identity(&d);
    sweep_default_keys(&d);
    snprintf(path, sizeof path, "%s/controls.conf", dir);
    write_str(path, "# nothing\n\n");
    setenv("PE_CONTROLS_CONFIG", path, 1);
    CHECK(HostPadRemap_Init() == 1);
    g = HostPadRemap_Global();
    CHECK(g->loaded == 0 && g->warnings == 0 && same_as_defaults(g));
    sweep_identity(g);
    sweep_default_keys(g);
    unsetenv("PE_CONTROLS_CONFIG");
    unlink(path);
    rmdir(dir);
    HostPadRemap_Reset();
}

/* ── confirm/cancel swap ─────────────────────────────────────────────── */

static void test_swap(void)
{
    HostPadRemap r;
    Log l;
    unsigned w;
    uint16_t keys;

    CHECK(parse(&r, "pad.cross=circle\npad.circle=cross\n", &l) == 0 && r.loaded == 2 && r.pad_active);
    log_free(&l);
    for (w = 0; w < 0x10000u; w++) {
        uint16_t want = (uint16_t)(w & ~(HOST_PAD_CROSS | HOST_PAD_CIRCLE));
        if (w & HOST_PAD_CROSS) want |= HOST_PAD_CIRCLE;
        if (w & HOST_PAD_CIRCLE) want |= HOST_PAD_CROSS;
        if (HostPadRemap_Pad(&r, (uint16_t)w) != want) { CHECK(!"swap sweep"); break; }
    }
    CHECK(HostPadRemap_Pad(&r, HOST_PAD_CROSS) == HOST_PAD_CIRCLE);
    CHECK(HostPadRemap_Pad(&r, HOST_PAD_CROSS | HOST_PAD_UP) == (HOST_PAD_CIRCLE | HOST_PAD_UP));
    CHECK(HostPadRemap_Pad(&r, 0x0006) == 0x0006);                    /* bits 1, 2 are not buttons */

    /* the raw word: the keyboard is never touched by pad.* */
    CHECK(HostPadRemap_PadWord(&r, 0, HOST_PAD_CROSS) == (uint16_t)~HOST_PAD_CIRCLE);
    keys = HOST_PAD_CROSS;
    CHECK(HostPadRemap_PadWord(&r, keys, 0) == (uint16_t)~HOST_PAD_CROSS);
    CHECK(HostPadRemap_PadWord(&r, keys, HOST_PAD_CROSS) == (uint16_t)~(HOST_PAD_CROSS | HOST_PAD_CIRCLE));
    CHECK(HostPadRemap_PadWord(&r, 0, 0) == 0xFFFFu);

    /* one-way: the destination keeps its own source, the vacated button is gone */
    parse(&r, "pad.cross=circle\n", &l);
    log_free(&l);
    CHECK(HostPadRemap_Pad(&r, HOST_PAD_CROSS) == HOST_PAD_CIRCLE);
    CHECK(HostPadRemap_Pad(&r, HOST_PAD_CIRCLE) == HOST_PAD_CIRCLE);
    CHECK(HostPadRemap_Pad(&r, HOST_PAD_CROSS | HOST_PAD_CIRCLE) == HOST_PAD_CIRCLE);

    /* a rotation is per physical press, never chained */
    parse(&r, "pad.cross=circle\npad.circle=square\npad.square=cross\n", &l);
    log_free(&l);
    CHECK(HostPadRemap_Pad(&r, HOST_PAD_CROSS) == HOST_PAD_CIRCLE);
    CHECK(HostPadRemap_Pad(&r, HOST_PAD_CIRCLE) == HOST_PAD_SQUARE);
    CHECK(HostPadRemap_Pad(&r, HOST_PAD_SQUARE) == HOST_PAD_CROSS);
    CHECK(HostPadRemap_Pad(&r, HOST_PAD_CROSS | HOST_PAD_CIRCLE | HOST_PAD_SQUARE) ==
          (HOST_PAD_CROSS | HOST_PAD_CIRCLE | HOST_PAD_SQUARE));

    /* every one of the 14 buttons can be a source and a destination */
    {
        static const char *const n[14] = {"select", "start", "up", "right", "down", "left", "l2", "r2",
                                          "l1", "r1", "triangle", "circle", "cross", "square"};
        static const uint16_t b[14] = {HOST_PAD_SELECT, HOST_PAD_START, HOST_PAD_UP, HOST_PAD_RIGHT,
                                       HOST_PAD_DOWN, HOST_PAD_LEFT, HOST_PAD_L2, HOST_PAD_R2, HOST_PAD_L1,
                                       HOST_PAD_R1, HOST_PAD_TRIANGLE, HOST_PAD_CIRCLE, HOST_PAD_CROSS,
                                       HOST_PAD_SQUARE};
        char text[1024];
        int i, len = 0;
        for (i = 0; i < 14; i++)
            len += snprintf(text + len, sizeof text - (size_t)len, "pad.%s=%s\n", n[i], n[(i + 1) % 14]);
        CHECK(parse(&r, text, &l) == 0 && r.loaded == 14 && r.warnings == 0);
        log_free(&l);
        for (i = 0; i < 14; i++) CHECK(HostPadRemap_Pad(&r, b[i]) == b[(i + 1) % 14]);
        CHECK(HostPadRemap_Pad(&r, ALL_BUTTONS) == ALL_BUTTONS);
        CHECK(invariants_ok(&r));
    }
}

/* ── keyboard table ──────────────────────────────────────────────────── */

static void test_key_defaults(void)
{
    static const struct { unsigned long ks; uint16_t bit; } k[] = {
        {0xFF0D, HOST_PAD_CROSS}, {0x20, HOST_PAD_CROSS}, {'z', HOST_PAD_CROSS}, {'x', HOST_PAD_CROSS},
        {'Z', HOST_PAD_CROSS}, {'X', HOST_PAD_CROSS},
        {'c', HOST_PAD_CIRCLE}, {'C', HOST_PAD_CIRCLE}, {'v', HOST_PAD_TRIANGLE}, {'V', HOST_PAD_TRIANGLE},
        {'s', HOST_PAD_SQUARE}, {'S', HOST_PAD_SQUARE}, {'q', HOST_PAD_L1}, {'Q', HOST_PAD_L1},
        {'e', HOST_PAD_R1}, {'E', HOST_PAD_R1}, {'1', HOST_PAD_L2}, {'3', HOST_PAD_R2},
        {0xFF09, HOST_PAD_SELECT}, {'p', HOST_PAD_START}, {'P', HOST_PAD_START},
        {0xFF52, HOST_PAD_UP}, {0xFF53, HOST_PAD_RIGHT}, {0xFF54, HOST_PAD_DOWN}, {0xFF51, HOST_PAD_LEFT},
    };
    HostPadRemap r;
    size_t i;
    HostPadRemap_Defaults(&r);
    CHECK(r.nkey == 17 && invariants_ok(&r));
    for (i = 0; i < sizeof k / sizeof k[0]; i++) {
        CHECK(HostPadRemap_Key(&r, k[i].ks) == k[i].bit);
        CHECK(old_chain(k[i].ks) == k[i].bit);
    }
    CHECK(HostPadRemap_Key(&r, '2') == 0 && HostPadRemap_Key(&r, 'a') == 0 && HostPadRemap_Key(&r, 0) == 0);
    sweep_default_keys(&r);                                          /* every keysym equals the old chain */
}

static void test_key_override(void)
{
    HostPadRemap r;
    Log l;
    unsigned long ks;

    CHECK(parse(&r, "key.cross = a, B\n", &l) == 0);
    log_free(&l);
    CHECK(HostPadRemap_Key(&r, 'a') == HOST_PAD_CROSS && HostPadRemap_Key(&r, 'A') == HOST_PAD_CROSS);
    CHECK(HostPadRemap_Key(&r, 'b') == HOST_PAD_CROSS && HostPadRemap_Key(&r, 'B') == HOST_PAD_CROSS);
    for (ks = 0; ks < SWEEP_KEYS; ks++) {
        uint16_t want = old_chain(ks);
        if (ks == 'a' || ks == 'A' || ks == 'b' || ks == 'B') want = HOST_PAD_CROSS;
        else if (want == HOST_PAD_CROSS) want = 0;                   /* the line replaced Cross's keys */
        if (HostPadRemap_Key(&r, ks) != want) { fprintf(stderr, "key 0x%lX\n", ks); CHECK(!"key.cross override"); break; }
    }

    /* an explicit key moves a default from another button: Return -> Circle */
    parse(&r, "key.circle=Return\n", &l);
    log_free(&l);
    CHECK(HostPadRemap_Key(&r, 0xFF0D) == HOST_PAD_CIRCLE && HostPadRemap_Key(&r, ' ') == HOST_PAD_CROSS);
    CHECK(HostPadRemap_Key(&r, 'c') == 0 && HostPadRemap_Key(&r, 'C') == 0);   /* Circle's old keys are gone */

    /* names are case-insensitive, hex is exact, digits and punctuation have names */
    parse(&r, "key.l1=RETURN\nkey.r1=0x58\nkey.l2=semicolon,0xff09\nkey.r2=Page_Up\nkey.start=7\n", &l);
    CHECK(l.len == 0);
    log_free(&l);
    CHECK(HostPadRemap_Key(&r, 0xFF0D) == HOST_PAD_L1);
    CHECK(HostPadRemap_Key(&r, 'x') == HOST_PAD_R1 && HostPadRemap_Key(&r, 'X') == HOST_PAD_R1);
    CHECK(HostPadRemap_Key(&r, ';') == HOST_PAD_L2 && HostPadRemap_Key(&r, 0xFF09) == HOST_PAD_L2);
    CHECK(HostPadRemap_Key(&r, 0xFF55) == HOST_PAD_R2);               /* Page_Up == Prior */
    CHECK(HostPadRemap_Key(&r, '7') == HOST_PAD_START);
    CHECK(invariants_ok(&r));

    /* keys never come from the neighbours: Tab no longer reaches Select once L2 has it */
    CHECK(HostPadRemap_Key(&r, 0xFF09) != HOST_PAD_SELECT);

    /* the whole name table resolves; spot-check values against keysymdef.h */
    {
        static const struct { const char *name; unsigned long ks; } names[] = {
            {"BackSpace", 0xFF08}, {"Tab", 0xFF09}, {"Return", 0xFF0D}, {"Delete", 0xFFFF},
            {"Home", 0xFF50}, {"Left", 0xFF51}, {"Up", 0xFF52}, {"Right", 0xFF53}, {"Down", 0xFF54},
            {"Prior", 0xFF55}, {"Next", 0xFF56}, {"Page_Down", 0xFF56}, {"End", 0xFF57}, {"Insert", 0xFF63},
            {"KP_Enter", 0xFF8D}, {"KP_Up", 0xFF97}, {"KP_End", 0xFF9C}, {"KP_0", 0xFFB0}, {"KP_9", 0xFFB9},
            {"KP_Add", 0xFFAB}, {"F9", 0xFFC6}, {"F12", 0xFFC9}, {"Shift_R", 0xFFE2}, {"Control_L", 0xFFE3},
            {"Alt_R", 0xFFEA}, {"space", 0x20}, {"comma", 0x2C}, {"period", 0x2E}, {"slash", 0x2F},
            {"minus", 0x2D}, {"equal", 0x3D}, {"grave", 0x60}, {"bracketleft", 0x5B}, {"backslash", 0x5C},
            {"bracketright", 0x5D}, {"apostrophe", 0x27}, {"semicolon", 0x3B},
        };
        size_t i;
        for (i = 0; i < sizeof names / sizeof names[0]; i++) {
            char text[128];
            snprintf(text, sizeof text, "key.square=%s\n", names[i].name);
            parse(&r, text, &l);
            CHECK(l.len == 0);
            if (HostPadRemap_Key(&r, names[i].ks) != HOST_PAD_SQUARE) fprintf(stderr, "name %s\n", names[i].name);
            CHECK(HostPadRemap_Key(&r, names[i].ks) == HOST_PAD_SQUARE);
            log_free(&l);
        }
    }
}

static void test_reserved_keys(void)
{
    static const char *const reserved_names[] = {"Escape", "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8",
                                                 "f3", "ESCAPE"};
    static const char *const reserved_hex[] = {"0xFF1B", "0xff1b", "0xFFBE", "0xFFBF", "0xFFC0", "0xFFC1",
                                               "0xFFC2", "0xFFC3", "0xFFC4", "0xFFC5"};
    static const char *const buttons[] = {"cross", "circle", "square", "triangle", "l1", "r1", "l2", "r2",
                                          "select", "start", "up", "down", "left", "right"};
    HostPadRemap r;
    Log l;
    unsigned long ks;
    size_t i, b;

    HostPadRemap_Defaults(&r);
    CHECK(HostPadRemap_KeyReserved(0xFF1B));
    for (ks = 0xFFBE; ks <= 0xFFC5; ks++) CHECK(HostPadRemap_KeyReserved(ks) && HostPadRemap_Key(&r, ks) == 0);
    CHECK(!HostPadRemap_KeyReserved(0xFFBD) && !HostPadRemap_KeyReserved(0xFFC6));   /* F9 is free */
    CHECK(!HostPadRemap_KeyReserved(0xFF0D) && !HostPadRemap_KeyReserved(0xFF52));

    /* parse time: by name and by hex, on every button, each line one warning */
    for (b = 0; b < sizeof buttons / sizeof buttons[0]; b++) {
        for (i = 0; i < sizeof reserved_names / sizeof reserved_names[0]; i++) {
            char text[128];
            snprintf(text, sizeof text, "key.%s=%s\n", buttons[b], reserved_names[i]);
            CHECK(parse(&r, text, &l) == 1 && r.warnings == 1 && same_as_defaults(&r));
            CHECK(strstr(l.buf, "reserved") != NULL);
            log_free(&l);
        }
        for (i = 0; i < sizeof reserved_hex / sizeof reserved_hex[0]; i++) {
            char text[128];
            snprintf(text, sizeof text, "key.%s=%s\n", buttons[b], reserved_hex[i]);
            CHECK(parse(&r, text, &l) == 1 && r.warnings == 1 && same_as_defaults(&r));
            log_free(&l);
        }
    }

    /* lookup time: even a table that somehow holds a reserved key never returns it
     * (F-keys reach the table when the cheat console does not take them) */
    HostPadRemap_Defaults(&r);
    r.key_sym[r.nkey] = 0xFFC3; r.key_bit[r.nkey++] = HOST_PAD_CROSS;
    r.key_sym[r.nkey] = 0xFF1B; r.key_bit[r.nkey++] = HOST_PAD_START;
    CHECK(HostPadRemap_Key(&r, 0xFFC3) == 0 && HostPadRemap_Key(&r, 0xFF1B) == 0);

    /* the neighbours stay bindable */
    CHECK(parse(&r, "key.start=F9\nkey.select=F12\n", &l) == 0);
    log_free(&l);
    CHECK(HostPadRemap_Key(&r, 0xFFC6) == HOST_PAD_START && HostPadRemap_Key(&r, 0xFFC9) == HOST_PAD_SELECT);

    /* a hostile file (every button on Esc/F-keys) leaves the defaults and every
     * reserved key unbound */
    {
        char text[4096];
        int len = 0;
        for (b = 0; b < sizeof buttons / sizeof buttons[0]; b++)
            len += snprintf(text + len, sizeof text - (size_t)len, "key.%s=F1,Escape,F8\n", buttons[b]);
        CHECK(parse(&r, text, &l) == 14 && r.warnings == 14 && same_as_defaults(&r));
        log_free(&l);
    }
}

/* ── Windows virtual keys ────────────────────────────────────────────── */

static void test_win32_vk(void)
{
    HostPadRemap r;
    Log l;
    unsigned vk;

    HostPadRemap_Defaults(&r);
    /* with no config, the shared table gives exactly the old vk_to_pad_bit for
     * every virtual-key code (VK_F1 == 'p', VK_NUMPAD1 == 'a', VK_INSERT == '-'
     * must not leak through as ASCII) */
    for (vk = 0; vk < 0x200u; vk++) {
        uint16_t got = HostPadRemap_Key(&r, HostPadRemap_KeysymFromWin32Vk(vk));
        if (got != old_vk_to_pad_bit(vk)) {
            fprintf(stderr, "vk 0x%X: table %04X old %04X\n", vk, got, old_vk_to_pad_bit(vk));
            CHECK(!"vk table equals the old switch");
        }
    }
    CHECK(HostPadRemap_KeysymFromWin32Vk('A') == 'a' && HostPadRemap_KeysymFromWin32Vk('1') == '1');
    CHECK(HostPadRemap_KeysymFromWin32Vk(0x25) == 0xFF51 && HostPadRemap_KeysymFromWin32Vk(0x78) == 0xFFC6);
    CHECK(HostPadRemap_KeysymFromWin32Vk(0x70) == 0xFFBE && HostPadRemap_KeysymFromWin32Vk(0x1B) == 0xFF1B);
    CHECK(HostPadRemap_KeysymFromWin32Vk(0x10) == 0 && HostPadRemap_KeysymFromWin32Vk(0xA0) == 0);

    /* reserved Windows keys (Esc, F1-F8) are never bound, config or not */
    parse(&r, "key.cross=0x61\nkey.circle=F9\nkey.start=KP_0\nkey.select=comma\n", &l);
    log_free(&l);
    CHECK(HostPadRemap_Key(&r, HostPadRemap_KeysymFromWin32Vk('A')) == HOST_PAD_CROSS);
    CHECK(HostPadRemap_Key(&r, HostPadRemap_KeysymFromWin32Vk(0x78)) == HOST_PAD_CIRCLE);   /* VK_F9 */
    CHECK(HostPadRemap_Key(&r, HostPadRemap_KeysymFromWin32Vk(0x60)) == HOST_PAD_START);    /* VK_NUMPAD0 */
    CHECK(HostPadRemap_Key(&r, HostPadRemap_KeysymFromWin32Vk(0xBC)) == HOST_PAD_SELECT);   /* VK_OEM_COMMA */
    CHECK(HostPadRemap_Key(&r, HostPadRemap_KeysymFromWin32Vk(0x1B)) == 0);                  /* VK_ESCAPE */
    for (vk = 0x70; vk <= 0x77; vk++)                                                        /* VK_F1..VK_F8 */
        CHECK(HostPadRemap_Key(&r, HostPadRemap_KeysymFromWin32Vk(vk)) == 0);
}

/* ── file and path handling ──────────────────────────────────────────── */

static void test_config_path(void)
{
    char out[256], home[1024];
    int had_home = getenv("HOME") != NULL;

    snprintf(home, sizeof home, "%s", had_home ? getenv("HOME") : "");
    unsetenv("PE_CONTROLS_CONFIG"); unsetenv("XDG_CONFIG_HOME");
    setenv("HOME", "/home/u", 1);
    CHECK(HostPadRemap_ConfigPath(out, sizeof out) && !strcmp(out, "/home/u/.config/parasite-eve-port/controls.conf"));
    setenv("XDG_CONFIG_HOME", "/xdg", 1);
    CHECK(HostPadRemap_ConfigPath(out, sizeof out) && !strcmp(out, "/xdg/parasite-eve-port/controls.conf"));
    setenv("PE_CONTROLS_CONFIG", "/x/y.conf", 1);
    CHECK(HostPadRemap_ConfigPath(out, sizeof out) && !strcmp(out, "/x/y.conf"));
    /* empty variables count as unset (as plat_enhance.c's config_path) */
    setenv("PE_CONTROLS_CONFIG", "", 1);
    CHECK(HostPadRemap_ConfigPath(out, sizeof out) && !strcmp(out, "/xdg/parasite-eve-port/controls.conf"));
    setenv("XDG_CONFIG_HOME", "", 1);
    CHECK(HostPadRemap_ConfigPath(out, sizeof out) && !strcmp(out, "/home/u/.config/parasite-eve-port/controls.conf"));
    /* too small a buffer, and nothing to build a path from */
    CHECK(!HostPadRemap_ConfigPath(out, 10));
    setenv("HOME", "", 1);
    CHECK(!HostPadRemap_ConfigPath(out, sizeof out));
    unsetenv("HOME");
    CHECK(!HostPadRemap_ConfigPath(out, sizeof out));
    unsetenv("PE_CONTROLS_CONFIG"); unsetenv("XDG_CONFIG_HOME");
    if (had_home) setenv("HOME", home, 1);
}

static void test_load_file(void)
{
    char dir[512], path[700], file2[700];
    HostPadRemap r;
    Log l;
    int rc;
    char *big;

    make_tmpdir(dir, sizeof dir);

    /* missing: silent, defaults */
    snprintf(path, sizeof path, "%s/none.conf", dir);
    log_open(&l);
    rc = HostPadRemap_LoadFile(&r, path, l.f);
    CHECK(log_close(&l) == 0 && rc == 0 && r.warnings == 0 && same_as_defaults(&r));
    log_free(&l);

    /* a path through a regular file (ENOTDIR) is silent too */
    snprintf(file2, sizeof file2, "%s/plain", dir);
    write_file(file2, "x", 1);
    snprintf(path, sizeof path, "%s/plain/controls.conf", dir);
    log_open(&l);
    rc = HostPadRemap_LoadFile(&r, path, l.f);
    CHECK(log_close(&l) == 0 && rc == 0 && same_as_defaults(&r));
    log_free(&l);
    unlink(file2);

    /* a file: warnings per bad line, then the single "loaded" line */
    snprintf(path, sizeof path, "%s/controls.conf", dir);
    write_str(path, "pad.cross=circle\npad.circle=cross\nbogus\n");
    log_open(&l);
    rc = HostPadRemap_LoadFile(&r, path, l.f);
    CHECK(log_close(&l) == 2 && rc == 1 && r.loaded == 2 && r.warnings == 1);
    CHECK(strstr(l.buf, "controls.conf:3: expected") != NULL);
    {
        char want[800];
        snprintf(want, sizeof want, "[CONTROLS] loaded 2 remaps from %s\n", path);
        CHECK(strstr(l.buf, want) != NULL);
    }
    log_free(&l);

    /* a directory (fread fails) warns once and leaves the defaults */
    log_open(&l);
    rc = HostPadRemap_LoadFile(&r, dir, l.f);
    CHECK(log_close(&l) == 1 && rc == 0 && r.warnings == 1 && same_as_defaults(&r));
    CHECK(strstr(l.buf, "cannot ") != NULL);
    log_free(&l);

    /* an oversized file is refused as a whole */
    big = (char *)malloc(HOST_PAD_REMAP_FILE_MAX + 100);
    CHECK(big != NULL);
    if (big) {
        memset(big, '#', HOST_PAD_REMAP_FILE_MAX + 100);
        write_file(path, big, HOST_PAD_REMAP_FILE_MAX + 100);
        log_open(&l);
        rc = HostPadRemap_LoadFile(&r, path, l.f);
        CHECK(log_close(&l) == 1 && rc == 0 && same_as_defaults(&r) && strstr(l.buf, "larger than") != NULL);
        log_free(&l);
        /* exactly at the limit is accepted */
        write_file(path, big, HOST_PAD_REMAP_FILE_MAX);
        log_open(&l);
        rc = HostPadRemap_LoadFile(&r, path, l.f);
        CHECK(log_close(&l) == 1 && rc == 1 && r.loaded == 0);
        log_free(&l);
        free(big);
    }

    /* Init: a real file via PE_CONTROLS_CONFIG applies, Reset drops it */
    write_str(path, "pad.start=select\nkey.start=F10\n");
    setenv("PE_CONTROLS_CONFIG", path, 1);
    CHECK(HostPadRemap_Init() == 1);
    CHECK(HostPadRemap_Global()->loaded == 2);
    CHECK(HostPadRemap_Pad(HostPadRemap_Global(), HOST_PAD_START) == HOST_PAD_SELECT);
    CHECK(HostPadRemap_Key(HostPadRemap_Global(), 0xFFC7) == HOST_PAD_START);
    HostPadRemap_Reset();
    CHECK(same_as_defaults(HostPadRemap_Global()));
    unsetenv("PE_CONTROLS_CONFIG");

    unlink(path);
    rmdir(dir);
}

/* ── the dev-menu combo and the remap ────────────────────────────────── */

static uint16_t combo_step(HostPadCombo *c, uint16_t held, uint64_t t, HostPadComboEvents *ev)
{
    HostPadOrigin o;
    o.key = 1u;
    o.held = held;
    return HostPadCombo_Step(c, &o, 1, 0, t, ev);
}

static void test_combo_unaffected(void)
{
    HostPadRemap r;
    HostPadCombo c;
    HostPadComboEvents ev;
    Log l;
    uint16_t out;

    /* the most hostile remap: Select and Start both become Cross (and Cross stays) */
    parse(&r, "pad.select=cross\npad.start=cross\npad.cross=cross\n", &l);
    log_free(&l);

    /* the combo reads physical bits, so Select+Start still opens the menu, and the
     * game is handed nothing while it does */
    memset(&c, 0, sizeof c);
    (void)combo_step(&c, 0, 50, &ev);
    out = combo_step(&c, HOST_PAD_SELECT, 100, &ev);
    CHECK(!ev.open && HostPadRemap_Pad(&r, out) == 0);
    out = combo_step(&c, HOST_PAD_SELECT | HOST_PAD_START, 150, &ev);
    CHECK(ev.open == 1 && out == 0 && HostPadRemap_PadWord(&r, 0, out) == 0xFFFFu);

    /* what the combo delivers is what the remap sees: a short lone Select is
     * delivered late (as Select), then remapped */
    memset(&c, 0, sizeof c);
    parse(&r, "pad.select=start\npad.start=select\n", &l);
    log_free(&l);
    (void)combo_step(&c, 0, 900, &ev);
    CHECK(combo_step(&c, HOST_PAD_SELECT, 1000, &ev) == 0);
    out = combo_step(&c, 0, 1050, &ev);
    CHECK(out == HOST_PAD_SELECT && !ev.open);
    CHECK(HostPadRemap_Pad(&r, out) == HOST_PAD_START);
    CHECK(HostPadRemap_PadWord(&r, 0, out) == (uint16_t)~HOST_PAD_START);

    /* a swapped Select/Start is still the same physical combo */
    memset(&c, 0, sizeof c);
    (void)combo_step(&c, 0, 2000, &ev);
    (void)combo_step(&c, HOST_PAD_START, 2050, &ev);
    (void)combo_step(&c, HOST_PAD_START | HOST_PAD_SELECT, 2100, &ev);
    CHECK(ev.open == 1);
}

/* ── robustness: no input breaks the table invariants ────────────────── */

static void test_fuzz_invariants(void)
{
    static const char alphabet[] = "padkey.=,#\n\r\t 0123456789xXabcdefzZ_ConsrlupFEsc crosspadstartselect\xFF";
    HostPadRemap r;
    unsigned seed = 12345u;
    char text[1500];
    int it;
    size_t i;
    for (it = 0; it < 3000; it++) {
        size_t n = 1 + (seed >> 8) % sizeof text;
        for (i = 0; i < n; i++) {
            seed = seed * 1103515245u + 12345u;
            text[i] = alphabet[(seed >> 16) % (sizeof alphabet - 1)];
        }
        /* splice in valid-looking lines so deep paths are reached */
        if (it % 3 == 0) memcpy(text, "key.cross=a,b\npad.up=down\n", n > 26 ? 26 : n);
        (void)HostPadRemap_Parse(&r, text, n, "fuzz", NULL);
        if (!invariants_ok(&r)) { fprintf(stderr, "iteration %d broke the invariants\n", it); CHECK(!"invariants"); return; }
        if (r.loaded != 0 && r.loaded > 28) { CHECK(!"loaded bound"); return; }
    }
}

int main(void)
{
    test_identity_no_file();          /* first: the process-wide config before any init */
    test_parser_valid();
    test_parser_invalid();
    test_parser_duplicates();
    test_swap();
    test_key_defaults();
    test_key_override();
    test_reserved_keys();
    test_win32_vk();
    test_config_path();
    test_load_file();
    test_combo_unaffected();
    test_fuzz_invariants();
    if (g_fail) return 1;
    printf("host-pad-remap tests: OK\n");
    return 0;
}
