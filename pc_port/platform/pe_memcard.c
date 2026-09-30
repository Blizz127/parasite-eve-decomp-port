/*
 * Host memory-card backend over a raw 128 KiB PS1 card image.  See
 * pe_memcard.h for the image layout and the policy.  Clean-room host code:
 * written from the public psx-spx format / BIOS-function descriptions.
 */
#include "pe_memcard.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#ifdef _WIN32
#include <direct.h>
#endif

#define MC_DIR_FRAMES   15u
#define MC_STATE_FREE   0xA0u
#define MC_STATE_FIRST  0x51u
#define MC_STATE_MIDDLE 0x52u
#define MC_STATE_LAST   0x53u
#define MC_STATE_DEL_FIRST  0xA1u
#define MC_STATE_DEL_MIDDLE 0xA2u
#define MC_STATE_DEL_LAST   0xA3u
#define MC_NAME_MAX     20u
#define MC_MAX_FDS      16
#define MC_FD_BASE      2   /* 0/1 are the BIOS tty handles */

typedef struct {
    int      configured;          /* SetPath called (else env/default) */
    int      present;             /* card in the port */
    int      memory_only;         /* "" path: never persisted */
    char     path[1024];
    int      loaded;
    int      formatted;           /* last known */
    int      new_card;            /* _card_info reports NEWCARD once */
    uint8_t  image[PE_MC_IMAGE_SIZE];
} McPort;

typedef struct {
    int      used;
    int      port;
    int      first_block;         /* 1..15 */
    uint32_t size;                /* bytes */
    uint32_t pos;
    uint32_t mode;
} McFile;

static McPort  g_ports[PE_MC_PORTS];
static McFile  g_files[MC_MAX_FDS];
static char    g_last_error[256];
static int     g_boundary_mode;

/* firstfile/nextfile search state */
static struct {
    int  active;
    int  port;
    int  next_block;
    char pattern[MC_NAME_MAX + 1];
} g_search;

static void set_error(const char *what, const char *path)
{
    snprintf(g_last_error, sizeof g_last_error, "%s %s: %s", what,
             path ? path : "", strerror(errno));
    fprintf(stderr, "[MEMCARD] %s\n", g_last_error);
}

const char *PE_Memcard_LastError(void) { return g_last_error; }

void PE_Memcard_SetBoundaryMode(int enabled) { g_boundary_mode = enabled != 0; }
int  PE_Memcard_BoundaryMode(void) { return g_boundary_mode; }

/* ── pure image helpers ─────────────────────────────────────────────── */

uint8_t PE_Memcard_FrameChecksum(const uint8_t *frame)
{
    uint8_t x = 0;
    for (unsigned i = 0; i < PE_MC_FRAME_SIZE - 1u; i++) x ^= frame[i];
    return x;
}

static void seal_frame(uint8_t *frame)
{
    frame[PE_MC_FRAME_SIZE - 1u] = PE_Memcard_FrameChecksum(frame);
}

static void put32(uint8_t *p, uint32_t v)
{
    p[0] = (uint8_t)v; p[1] = (uint8_t)(v >> 8);
    p[2] = (uint8_t)(v >> 16); p[3] = (uint8_t)(v >> 24);
}
static uint32_t get32(const uint8_t *p)
{
    return (uint32_t)p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 |
           (uint32_t)p[3] << 24;
}
static void put16(uint8_t *p, uint16_t v) { p[0] = (uint8_t)v; p[1] = (uint8_t)(v >> 8); }
static uint16_t get16(const uint8_t *p) { return (uint16_t)(p[0] | p[1] << 8); }

static void free_dir_frame(uint8_t *f)
{
    memset(f, 0, PE_MC_FRAME_SIZE);
    put32(f, MC_STATE_FREE);
    put16(f + 8, 0xFFFFu);
    seal_frame(f);
}

void PE_Memcard_FormatImage(uint8_t image[PE_MC_IMAGE_SIZE])
{
    memset(image, 0, PE_MC_IMAGE_SIZE);
    image[0] = 'M'; image[1] = 'C';
    seal_frame(image);
    for (unsigned i = 1; i <= MC_DIR_FRAMES; i++)
        free_dir_frame(image + i * PE_MC_FRAME_SIZE);
    for (unsigned i = 16; i <= 35; i++) {
        uint8_t *f = image + i * PE_MC_FRAME_SIZE;
        put32(f, 0xFFFFFFFFu);
        put16(f + 8, 0xFFFFu);
        seal_frame(f);
    }
    memcpy(image + 63u * PE_MC_FRAME_SIZE, image, PE_MC_FRAME_SIZE);
}

int PE_Memcard_ImageIsFormatted(const uint8_t image[PE_MC_IMAGE_SIZE])
{
    return image[0] == 'M' && image[1] == 'C' &&
           PE_Memcard_FrameChecksum(image) == image[PE_MC_FRAME_SIZE - 1u];
}

/* ── port configuration / persistence ───────────────────────────────── */

static int make_dirs(const char *path)
{
    char tmp[1024];
    size_t n = strlen(path);
    if (n >= sizeof tmp) return -1;
    memcpy(tmp, path, n + 1);
    for (size_t i = 1; i < n; i++) {
        if (tmp[i] == '/' || tmp[i] == '\\') {
            char c = tmp[i];
            tmp[i] = 0;
#ifdef _WIN32
            if (_mkdir(tmp) != 0 && errno != EEXIST) { tmp[i] = c; return -1; }
#else
            if (mkdir(tmp, 0755) != 0 && errno != EEXIST) { tmp[i] = c; return -1; }
#endif
            tmp[i] = c;
        }
    }
    return 0;
}

static void default_path(int port, char *out, size_t cap)
{
    const char *env = getenv(port == 0 ? "PE_MEMCARD1" : "PE_MEMCARD2");
    out[0] = 0;
    if (env && env[0]) { snprintf(out, cap, "%s", env); return; }
    if (port != 0) return;                 /* slot 2 empty unless configured */
#ifdef _WIN32
    {
        const char *base = getenv("APPDATA");
        if (base && base[0]) {
            snprintf(out, cap, "%s\\parasite-eve-port\\memcard1.mcd", base);
            return;
        }
    }
#endif
    {
        const char *xdg = getenv("XDG_DATA_HOME");
        const char *home = getenv("HOME");
        if (xdg && xdg[0])
            snprintf(out, cap, "%s/parasite-eve-port/memcard1.mcd", xdg);
        else if (home && home[0])
            snprintf(out, cap, "%s/.local/share/parasite-eve-port/memcard1.mcd", home);
        else
            snprintf(out, cap, "memcard1.mcd");
    }
}

static void resolve(int port)
{
    McPort *p = &g_ports[port];
    if (p->configured) return;
    p->configured = 1;
    default_path(port, p->path, sizeof p->path);
    p->present = p->path[0] != 0;
    p->memory_only = 0;
}

static int save_image(McPort *p)
{
    char tmp[1100];
    FILE *f;
    if (p->memory_only || !p->present) return 1;
    if (make_dirs(p->path) != 0) { set_error("mkdir for", p->path); return 0; }
    snprintf(tmp, sizeof tmp, "%s.tmp", p->path);
    f = fopen(tmp, "wb");
    if (!f) { set_error("open", tmp); return 0; }
    if (fwrite(p->image, 1, PE_MC_IMAGE_SIZE, f) != PE_MC_IMAGE_SIZE) {
        set_error("write", tmp); fclose(f); remove(tmp); return 0;
    }
    if (fclose(f) != 0) { set_error("close", tmp); remove(tmp); return 0; }
#ifdef _WIN32
    remove(p->path);
#endif
    if (rename(tmp, p->path) != 0) { set_error("rename to", p->path); remove(tmp); return 0; }
    return 1;
}

static McPort *load_port(int port)
{
    McPort *p;
    if (port < 0 || port >= PE_MC_PORTS) return NULL;
    resolve(port);
    p = &g_ports[port];
    if (!p->present) return NULL;
    if (p->loaded) return p;
    p->loaded = 1;
    p->new_card = 1;
    if (p->memory_only) {
        PE_Memcard_FormatImage(p->image);
    } else {
        FILE *f = fopen(p->path, "rb");
        if (!f) {
            /* Missing image: create a freshly formatted card. */
            PE_Memcard_FormatImage(p->image);
            if (save_image(p))
                fprintf(stderr, "[MEMCARD] created formatted card image %s\n", p->path);
        } else {
            size_t n = fread(p->image, 1, PE_MC_IMAGE_SIZE, f);
            int extra = fgetc(f) != EOF;
            fclose(f);
            if (n != PE_MC_IMAGE_SIZE || extra) {
                /* Not a raw 128 KiB image: an unformatted card to the game;
                 * the file is only replaced if the game formats it. */
                fprintf(stderr, "[MEMCARD] %s is not a 128 KiB raw card image "
                        "(%zu bytes): presented as unformatted\n", p->path, n);
                memset(p->image, 0, PE_MC_IMAGE_SIZE);
            }
        }
    }
    p->formatted = PE_Memcard_ImageIsFormatted(p->image);
    return p;
}

void PE_Memcard_SetPath(int port, const char *path)
{
    McPort *p;
    if (port < 0 || port >= PE_MC_PORTS) return;
    p = &g_ports[port];
    for (int i = 0; i < MC_MAX_FDS; i++)
        if (g_files[i].used && g_files[i].port == port) g_files[i].used = 0;
    if (g_search.port == port) g_search.active = 0;
    memset(p, 0, sizeof *p);
    p->configured = 1;
    if (!path) return;
    p->present = 1;
    if (!path[0]) p->memory_only = 1;
    else snprintf(p->path, sizeof p->path, "%s", path);
}

void PE_Memcard_Reset(void)
{
    memset(g_ports, 0, sizeof g_ports);
    memset(g_files, 0, sizeof g_files);
    memset(&g_search, 0, sizeof g_search);
    g_last_error[0] = 0;
}

const char *PE_Memcard_Path(int port)
{
    if (port < 0 || port >= PE_MC_PORTS) return NULL;
    resolve(port);
    if (!g_ports[port].present || g_ports[port].memory_only) return NULL;
    return g_ports[port].path;
}

int PE_Memcard_Present(int port)
{
    if (port < 0 || port >= PE_MC_PORTS) return 0;
    resolve(port);
    return g_ports[port].present;
}

int PE_Memcard_IsFormatted(int port)
{
    McPort *p = load_port(port);
    return p && PE_Memcard_ImageIsFormatted(p->image);
}

const uint8_t *PE_Memcard_Image(int port)
{
    McPort *p = load_port(port);
    return p ? p->image : NULL;
}

/* ── names / directory ──────────────────────────────────────────────── */

/* "buXY:rest" -> port X, *rest.  Returns -1 when not a card device. */
static int parse_device(const char *name, const char **rest)
{
    if (!name || name[0] != 'b' || name[1] != 'u') return -1;
    if (name[2] < '0' || name[2] > '1') return -1;
    if (name[3] < '0' || name[3] > '9') return -1;
    if (name[4] != ':') return -1;
    if (rest) *rest = name + 5;
    return name[2] - '0';
}

static uint8_t *dir_frame(McPort *p, int block)
{
    return p->image + (unsigned)block * PE_MC_FRAME_SIZE;
}

static int name_equals(const uint8_t *frame, const char *name)
{
    char stored[MC_NAME_MAX + 1];
    memcpy(stored, frame + 0x0A, MC_NAME_MAX);
    stored[MC_NAME_MAX] = 0;
    return strncmp(stored, name, MC_NAME_MAX) == 0;
}

static int find_file(McPort *p, const char *name)
{
    for (int b = 1; b <= (int)MC_DIR_FRAMES; b++) {
        const uint8_t *f = dir_frame(p, b);
        if (get32(f) == MC_STATE_FIRST && name_equals(f, name)) return b;
    }
    return -1;
}

static int block_is_free(McPort *p, int b)
{
    uint32_t s = get32(dir_frame(p, b));
    return s == MC_STATE_FREE || s == MC_STATE_DEL_FIRST ||
           s == MC_STATE_DEL_MIDDLE || s == MC_STATE_DEL_LAST;
}

/* Data offset of byte `pos` of the file starting at `first`; -1 if the
 * chain is shorter. */
static long file_offset(McPort *p, int first, uint32_t pos)
{
    int b = first;
    uint32_t hops = pos / PE_MC_BLOCK_SIZE;
    while (hops--) {
        uint16_t next = get16(dir_frame(p, b) + 8);
        if (next == 0xFFFFu || next >= MC_DIR_FRAMES) return -1;
        b = next + 1;
    }
    return (long)b * PE_MC_BLOCK_SIZE + (long)(pos % PE_MC_BLOCK_SIZE);
}

static int create_file(McPort *p, const char *name, uint32_t blocks)
{
    int chain[MC_DIR_FRAMES];
    uint32_t found = 0;
    if (blocks == 0) blocks = 1;
    if (blocks > MC_DIR_FRAMES) return -1;
    for (int b = 1; b <= (int)MC_DIR_FRAMES && found < blocks; b++)
        if (block_is_free(p, b)) chain[found++] = b;
    if (found < blocks) return -1;
    for (uint32_t i = 0; i < blocks; i++) {
        uint8_t *f = dir_frame(p, chain[i]);
        memset(f, 0, PE_MC_FRAME_SIZE);
        put32(f, blocks == 1 || i == 0 ? MC_STATE_FIRST
                 : i + 1 == blocks ? MC_STATE_LAST : MC_STATE_MIDDLE);
        if (i == 0) {
            put32(f + 4, blocks * PE_MC_BLOCK_SIZE);
            memcpy(f + 0x0A, name, strnlen(name, MC_NAME_MAX));
        }
        put16(f + 8, i + 1 < blocks ? (uint16_t)(chain[i + 1] - 1) : 0xFFFFu);
        seal_frame(f);
        /* A new file starts zero-filled. */
        memset(p->image + (size_t)chain[i] * PE_MC_BLOCK_SIZE, 0, PE_MC_BLOCK_SIZE);
    }
    return chain[0];
}

static McFile *get_file(int fd)
{
    int i = fd - MC_FD_BASE;
    if (i < 0 || i >= MC_MAX_FDS || !g_files[i].used) return NULL;
    return &g_files[i];
}

/* ── BIOS file functions ────────────────────────────────────────────── */

int PE_Memcard_Open(const char *name, uint32_t mode)
{
    const char *rest;
    int port = parse_device(name, &rest), first, slot = -1;
    McPort *p;
    if (port < 0 || !rest[0]) return -1;
    p = load_port(port);
    if (!p || !PE_Memcard_ImageIsFormatted(p->image)) return -1;
    for (int i = 0; i < MC_MAX_FDS; i++) if (!g_files[i].used) { slot = i; break; }
    if (slot < 0) return -1;
    first = find_file(p, rest);
    if (mode & 0x200u) {                         /* FCREAT */
        if (first >= 0) return -1;               /* already exists */
        first = create_file(p, rest, mode >> 16);
        if (first < 0) return -1;
        if (!save_image(p)) return -1;
        fprintf(stderr, "[MEMCARD] port %d: created %s (%u block(s) from %d)\n",
                port, rest, (mode >> 16) ? mode >> 16 : 1u, first);
    } else if (first < 0) {
        return -1;
    }
    g_files[slot].used = 1;
    g_files[slot].port = port;
    g_files[slot].first_block = first;
    g_files[slot].size = get32(dir_frame(p, first) + 4);
    g_files[slot].pos = 0;
    g_files[slot].mode = mode;
    return slot + MC_FD_BASE;
}

int PE_Memcard_Lseek(int fd, int32_t offset, int whence)
{
    McFile *f = get_file(fd);
    int64_t pos;
    if (!f) return -1;
    pos = whence == 1 ? (int64_t)f->pos + offset : (int64_t)offset;
    if (whence != 0 && whence != 1) return -1;
    if (pos < 0 || pos > (int64_t)f->size) return -1;
    f->pos = (uint32_t)pos;
    return (int)f->pos;
}

/* Card transfers are whole 128-byte sectors; a transfer may cross into the
 * next block of the chain. */
static int transfer(int fd, uint8_t *dst, const uint8_t *src, int32_t length, int write)
{
    McFile *f = get_file(fd);
    McPort *p;
    int32_t done = 0;
    if (!f || length < 0) return -1;
    if ((length & 0x7F) || (f->pos & 0x7Fu)) return -1;
    if (write ? !(f->mode & 2u) : !(f->mode & 1u)) return -1;
    p = load_port(f->port);
    if (!p) return -1;
    if ((uint32_t)length > f->size - f->pos) length = (int32_t)(f->size - f->pos);
    while (done < length) {
        long off = file_offset(p, f->first_block, f->pos);
        uint32_t chunk = PE_MC_BLOCK_SIZE - (f->pos % PE_MC_BLOCK_SIZE);
        if (off < 0) break;
        if (chunk > (uint32_t)(length - done)) chunk = (uint32_t)(length - done);
        if (write) memcpy(p->image + off, src + done, chunk);
        else memcpy(dst + done, p->image + off, chunk);
        done += (int32_t)chunk;
        f->pos += chunk;
    }
    if (write && done > 0 && !save_image(p)) return -1;
    return done;
}

int PE_Memcard_Read(int fd, void *dst, int32_t length)
{
    return transfer(fd, (uint8_t *)dst, NULL, length, 0);
}

int PE_Memcard_Write(int fd, const void *src, int32_t length)
{
    return transfer(fd, NULL, (const uint8_t *)src, length, 1);
}

int PE_Memcard_Close(int fd)
{
    McFile *f = get_file(fd);
    if (!f) return -1;
    if (f->mode & 2u)
        fprintf(stderr, "[MEMCARD] port %d: closed file at block %d after writing to %u/%u%s%s\n",
                f->port, f->first_block, f->pos, f->size,
                g_ports[f->port].memory_only ? "" : ", saved ",
                g_ports[f->port].memory_only ? "" : g_ports[f->port].path);
    f->used = 0;
    return fd;
}

int PE_Memcard_Format(const char *device)
{
    int port = parse_device(device, NULL);
    McPort *p;
    if (port < 0) return 0;
    p = load_port(port);
    if (!p) return 0;
    for (int i = 0; i < MC_MAX_FDS; i++)
        if (g_files[i].used && g_files[i].port == port) g_files[i].used = 0;
    PE_Memcard_FormatImage(p->image);
    p->formatted = 1;
    fprintf(stderr, "[MEMCARD] port %d: formatted\n", port);
    return save_image(p) ? 1 : 0;
}

/* psx-spx: '?' matches any one character, '*' matches the remainder. */
static int pattern_match(const char *pattern, const uint8_t *frame)
{
    char stored[MC_NAME_MAX + 1];
    memcpy(stored, frame + 0x0A, MC_NAME_MAX);
    stored[MC_NAME_MAX] = 0;
    for (unsigned i = 0; ; i++) {
        char c = pattern[i];
        if (c == '*') return 1;
        if (c == 0) return stored[i] == 0;
        if (i >= MC_NAME_MAX) return 0;
        if (c != '?' && c != stored[i]) return 0;
        if (c == '?' && stored[i] == 0) return 0;
    }
}

static int search_next(PeMcDirEntry *out)
{
    McPort *p = load_port(g_search.port);
    if (!g_search.active || !p || !PE_Memcard_ImageIsFormatted(p->image)) {
        g_search.active = 0;
        return 0;
    }
    while (g_search.next_block <= (int)MC_DIR_FRAMES) {
        int b = g_search.next_block++;
        const uint8_t *f = dir_frame(p, b);
        if (get32(f) != MC_STATE_FIRST || !pattern_match(g_search.pattern, f)) continue;
        memset(out, 0, sizeof *out);
        memcpy(out->name, f + 0x0A, MC_NAME_MAX);
        out->attr = get32(f);
        out->size = get32(f + 4);
        out->next = 0;
        out->head = (uint32_t)b;
        return 1;
    }
    g_search.active = 0;
    return 0;
}

int PE_Memcard_FirstFile(const char *pattern, PeMcDirEntry *out)
{
    const char *rest;
    int port = parse_device(pattern, &rest);
    g_search.active = 0;
    if (port < 0 || !load_port(port)) return 0;
    g_search.active = 1;
    g_search.port = port;
    g_search.next_block = 1;
    snprintf(g_search.pattern, sizeof g_search.pattern, "%s", rest[0] ? rest : "*");
    return search_next(out);
}

int PE_Memcard_NextFile(PeMcDirEntry *out)
{
    return search_next(out);
}

int PE_Memcard_Delete(const char *name)
{
    const char *rest;
    int port = parse_device(name, &rest), b;
    McPort *p;
    if (port < 0) return 0;
    p = load_port(port);
    if (!p || !PE_Memcard_ImageIsFormatted(p->image)) return 0;
    b = find_file(p, rest);
    if (b < 0) return 0;
    for (int guard = 0; b >= 1 && b <= (int)MC_DIR_FRAMES && guard < (int)MC_DIR_FRAMES; guard++) {
        uint8_t *f = dir_frame(p, b);
        uint32_t s = get32(f);
        uint16_t next = get16(f + 8);
        put32(f, s == MC_STATE_FIRST ? MC_STATE_DEL_FIRST
                 : s == MC_STATE_LAST ? MC_STATE_DEL_LAST : MC_STATE_DEL_MIDDLE);
        seal_frame(f);
        if (next == 0xFFFFu) break;
        b = next + 1;
    }
    for (int i = 0; i < MC_MAX_FDS; i++)
        if (g_files[i].used && g_files[i].port == port &&
            block_is_free(p, g_files[i].first_block)) g_files[i].used = 0;
    fprintf(stderr, "[MEMCARD] port %d: deleted %s\n", port, rest);
    return save_image(p) ? 1 : 0;
}

/* ── libcard card status ─────────────────────────────────────────────── */

int PE_Memcard_CardInfoResult(uint32_t chan)
{
    McPort *p = load_port((int)(chan >> 4));
    if ((chan & 0xFu) != 0u || !p) return PE_MC_EVENT_TIMEOUT;
    return p->new_card ? PE_MC_EVENT_NEWCARD : PE_MC_EVENT_IOE;
}

int PE_Memcard_CardClearResult(uint32_t chan)
{
    McPort *p = load_port((int)(chan >> 4));
    if ((chan & 0xFu) != 0u || !p) return PE_MC_EVENT_TIMEOUT;
    p->new_card = 0;
    return PE_MC_EVENT_IOE;
}

int PE_Memcard_CardLoadResult(uint32_t chan)
{
    McPort *p = load_port((int)(chan >> 4));
    if ((chan & 0xFu) != 0u || !p) return PE_MC_EVENT_TIMEOUT;
    return PE_Memcard_ImageIsFormatted(p->image) ? PE_MC_EVENT_IOE : PE_MC_EVENT_NEWCARD;
}
