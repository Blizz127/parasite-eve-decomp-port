/* F12 screenshots: a minimal PNG writer and the screenshot folder.
 *
 * The writer produces an 8-bit RGB truecolour PNG with filter type 0 on every
 * row and the zlib stream made of stored (uncompressed) deflate blocks, with
 * the CRC-32 of every chunk and the Adler-32 trailer.  It has no dependency
 * and no compression, so a 320x240 frame is about 230 KB.
 *
 * Header-only on purpose (like host_window_scale.h): the window backends are
 * compiled directly into several test executables, and a separate .c would
 * have to be linked into each of them.  Every function is static inline. */
#ifndef HOST_PNG_H
#define HOST_PNG_H

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#ifdef _WIN32
#include "pe_posix_win32.h"   /* mkdir(path, mode) -> the CRT form */
#endif

#define HOST_PNG_SIG_LEN 8u
#define HOST_PNG_MAX_DIM 16384

static uint32_t hp_crc_table[256];
static int hp_crc_ready;

static inline void hp_crc_init(void)
{
    for (uint32_t n = 0; n < 256u; n++) {
        uint32_t c = n;
        for (int k = 0; k < 8; k++) c = (c & 1u) ? 0xEDB88320u ^ (c >> 1) : c >> 1;
        hp_crc_table[n] = c;
    }
    hp_crc_ready = 1;
}

/* Running CRC: `crc` is the register before the final xor. */
static inline uint32_t hp_crc_update(uint32_t crc, const uint8_t *p, size_t n)
{
    if (!hp_crc_ready) hp_crc_init();
    for (size_t i = 0; i < n; i++) crc = hp_crc_table[(crc ^ p[i]) & 0xFFu] ^ (crc >> 8);
    return crc;
}

/* Standard CRC-32 (polynomial 0xEDB88320, init and final xor 0xFFFFFFFF). */
static inline uint32_t HostPng_Crc32(const uint8_t *p, size_t n)
{
    return hp_crc_update(0xFFFFFFFFu, p, n) ^ 0xFFFFFFFFu;
}

/* zlib Adler-32 (modulus 65521, starts at 1). */
static inline uint32_t HostPng_Adler32(const uint8_t *p, size_t n)
{
    uint32_t a = 1u, b = 0u;
    for (size_t i = 0; i < n; i++) {
        a = (a + p[i]) % 65521u;
        b = (b + a) % 65521u;
    }
    return (b << 16) | a;
}

static inline void hp_put32(uint8_t *p, uint32_t v)
{
    p[0] = (uint8_t)(v >> 24);
    p[1] = (uint8_t)(v >> 16);
    p[2] = (uint8_t)(v >> 8);
    p[3] = (uint8_t)v;
}

/* The file size HostPng_Encode writes for a w x h frame; 0 when w or h is
 * not in 1..16384. */
static inline size_t HostPng_EncodedSize(int w, int h)
{
    size_t row, raw, blocks, zlib;
    if (w <= 0 || h <= 0 || w > HOST_PNG_MAX_DIM || h > HOST_PNG_MAX_DIM) return 0;
    row = 1u + 3u * (size_t)w;                 /* filter byte + RGB */
    raw = row * (size_t)h;
    blocks = (raw + 65534u) / 65535u;          /* stored blocks hold up to 65535 bytes */
    zlib = 2u + raw + 5u * blocks + 4u;        /* header, blocks (5-byte headers), Adler-32 */
    return HOST_PNG_SIG_LEN + (12u + 13u) + (12u + zlib) + 12u;
}

/* Raw (pre-deflate) byte i: every row is a filter byte 0 then w*3 RGB bytes. */
static inline uint8_t hp_raw_byte(const uint8_t *rgb, size_t row_len, size_t w3, size_t i)
{
    size_t y = i / row_len, x = i % row_len;
    return x == 0 ? 0u : rgb[y * w3 + (x - 1u)];
}

/* One chunk: length, type, data, CRC over type + data.  Returns its size. */
static inline size_t hp_put_chunk(uint8_t *p, const char *type, const uint8_t *data, size_t n)
{
    hp_put32(p, (uint32_t)n);
    memcpy(p + 4, type, 4);
    if (n) memcpy(p + 8, data, n);
    hp_put32(p + 8 + n, hp_crc_update(0xFFFFFFFFu, p + 4, 4u + n) ^ 0xFFFFFFFFu);
    return 12u + n;
}

/* Encodes the RGB frame (3 bytes per pixel, rows top to bottom) into out.
 * Returns the bytes written, or 0 when cap is too small or the size is out of
 * range. */
static inline size_t HostPng_Encode(uint8_t *out, size_t cap, const uint8_t *rgb, int w, int h)
{
    static const uint8_t sig[HOST_PNG_SIG_LEN] = {0x89, 'P', 'N', 'G', 0x0D, 0x0A, 0x1A, 0x0A};
    size_t total = HostPng_EncodedSize(w, h);
    size_t row_len, w3, raw, blocks, zlen, o, done = 0;
    uint32_t a = 1u, b = 0u, crc;
    uint8_t ihdr[13];
    if (!out || !rgb || total == 0 || cap < total) return 0;
    row_len = 1u + 3u * (size_t)w;
    w3 = 3u * (size_t)w;
    raw = row_len * (size_t)h;
    blocks = (raw + 65534u) / 65535u;
    zlen = 2u + raw + 5u * blocks + 4u;
    memcpy(out, sig, HOST_PNG_SIG_LEN);
    o = HOST_PNG_SIG_LEN;
    hp_put32(ihdr, (uint32_t)w);
    hp_put32(ihdr + 4, (uint32_t)h);
    ihdr[8] = 8;     /* bit depth */
    ihdr[9] = 2;     /* colour type: truecolour RGB */
    ihdr[10] = 0;    /* compression: deflate */
    ihdr[11] = 0;    /* filter method 0 */
    ihdr[12] = 0;    /* no interlace */
    o += hp_put_chunk(out + o, "IHDR", ihdr, sizeof ihdr);
    /* IDAT: the zlib stream (0x78 0x01 = deflate, 32K window, fastest) */
    hp_put32(out + o, (uint32_t)zlen);
    memcpy(out + o + 4, "IDAT", 4);
    crc = hp_crc_update(0xFFFFFFFFu, out + o + 4, 4u);
    o += 8;
    out[o] = 0x78;
    out[o + 1] = 0x01;
    crc = hp_crc_update(crc, out + o, 2u);
    o += 2;
    while (done < raw) {
        size_t len = raw - done < 65535u ? raw - done : 65535u;
        size_t nlen = ~len;
        uint8_t hdr[5];
        hdr[0] = (done + len == raw) ? 1u : 0u;   /* BFINAL; BTYPE 00 = stored */
        hdr[1] = (uint8_t)len;
        hdr[2] = (uint8_t)(len >> 8);
        hdr[3] = (uint8_t)nlen;
        hdr[4] = (uint8_t)(nlen >> 8);
        memcpy(out + o, hdr, sizeof hdr);
        crc = hp_crc_update(crc, hdr, sizeof hdr);
        o += sizeof hdr;
        for (size_t k = 0; k < len; k++) {
            uint8_t v = hp_raw_byte(rgb, row_len, w3, done + k);
            out[o + k] = v;
            a = (a + v) % 65521u;
            b = (b + a) % 65521u;
        }
        crc = hp_crc_update(crc, out + o, len);
        o += len;
        done += len;
    }
    hp_put32(out + o, (b << 16) | a);
    crc = hp_crc_update(crc, out + o, 4u);
    o += 4;
    hp_put32(out + o, crc ^ 0xFFFFFFFFu);
    o += 4;
    o += hp_put_chunk(out + o, "IEND", NULL, 0);
    return o == total ? o : 0;
}

/* The screenshot folder: $XDG_DATA_HOME/parasite-eve-port/screenshots, or
 * $HOME/.local/share/... when XDG_DATA_HOME is unset or empty (Windows:
 * %APPDATA%/parasite-eve-port/screenshots).  Returns 0 and the path, or -1
 * when there is no base directory. */
static inline int HostShot_Dir(char *out, size_t n)
{
    const char *x;
    int len;
    if (!out || n == 0) return -1;
    out[0] = 0;
#ifdef _WIN32
    x = getenv("APPDATA");
    if (!x || !*x) return -1;
    len = snprintf(out, n, "%s/parasite-eve-port/screenshots", x);
#else
    x = getenv("XDG_DATA_HOME");
    if (x && *x) len = snprintf(out, n, "%s/parasite-eve-port/screenshots", x);
    else {
        x = getenv("HOME");
        if (!x || !*x) return -1;
        len = snprintf(out, n, "%s/.local/share/parasite-eve-port/screenshots", x);
    }
#endif
    return (len > 0 && (size_t)len < n) ? 0 : -1;
}

/* Creates every missing component of `path` (errors ignored: the file open
 * that follows reports a folder that could not be made). */
static inline void hp_make_dirs(const char *path)
{
    char buf[1100];
    size_t len = strlen(path);
    if (len >= sizeof buf) return;
    memcpy(buf, path, len + 1u);
    for (size_t i = 1; i <= len; i++) {
        if (buf[i] == '/' || buf[i] == 0) {
            char c = buf[i];
            buf[i] = 0;
            (void)mkdir(buf, 0700);
            buf[i] = c;
        }
    }
}

/* Writes the frame to the next free pe-<UTC>-<n>.png in HostShot_Dir (the
 * folder is created on demand; a file is never overwritten; <n> counts from 1
 * per process).  Returns 0 and the full path, or -1. */
static inline int HostShot_Save(const uint8_t *rgb, int w, int h, char *path, size_t n)
{
    static unsigned seq;
    char dir[1024], stamp[32], file[1200];
    time_t now;
    struct tm *utc;
    size_t need;
    uint8_t *buf;
    FILE *f;
    if (path && n) path[0] = 0;
    need = HostPng_EncodedSize(w, h);
    if (!rgb || need == 0 || !path || n == 0 || HostShot_Dir(dir, sizeof dir) != 0) return -1;
    hp_make_dirs(dir);
    if (time(&now) == (time_t)-1 || !(utc = gmtime(&now))) return -1;
    if (strftime(stamp, sizeof stamp, "%Y%m%dT%H%M%SZ", utc) == 0) return -1;
    buf = (uint8_t *)malloc(need);
    if (!buf) return -1;
    if (HostPng_Encode(buf, need, rgb, w, h) != need) { free(buf); return -1; }
    for (int tries = 0; tries < 10000; tries++) {
        seq++;
        if (snprintf(file, sizeof file, "%s/pe-%s-%u.png", dir, stamp, seq) >= (int)sizeof file) break;
        f = fopen(file, "rb");          /* never overwrite an existing shot */
        if (f) { fclose(f); continue; }
        f = fopen(file, "wb");
        if (!f) break;
        if (fwrite(buf, 1, need, f) != need) { fclose(f); (void)remove(file); break; }
        if (fclose(f) != 0) { (void)remove(file); break; }
        if (snprintf(path, n, "%s", file) >= (int)n) { (void)remove(file); break; }
        free(buf);
        return 0;
    }
    free(buf);
    return -1;
}

/* F12: HostShot_Save, then one "[SCREENSHOT] <path> (WxH)" line on stderr.
 * Returns 0 when the file was written. */
static inline int HostShot_Take(const uint8_t *rgb, int w, int h)
{
    char path[1200];
    if (HostShot_Save(rgb, w, h, path, sizeof path) != 0) {
        fprintf(stderr, "[SCREENSHOT] not saved: no presented frame yet, or no writable screenshot folder\n");
        return -1;
    }
    fprintf(stderr, "[SCREENSHOT] %s (%dx%d)\n", path, w, h);
    return 0;
}

#endif /* HOST_PNG_H */
