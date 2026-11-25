/*
 * Disc cache (host side) — see pe_disc_cache.h.
 */
#include "pe_disc_cache.h"
#include "pe_disc.h"
#include "pe_disc_check.h"
#include "pe_guest_ram.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PE_EXE_HDR 0x800u

static uint8_t *s_exe;
static uint32_t s_exe_size;
static uint32_t s_taddr, s_tsize;
static int s_tried;
static char s_err[256];

static void Chomp(char *s)
{
    size_t n = strlen(s);
    while (n > 0 && (s[n - 1] == '\n' || s[n - 1] == '\r' || s[n - 1] == ' '))
        s[--n] = '\0';
}

int PE_DiscCache_ImagePath(char *out, size_t out_size)
{
    const char *env = getenv("PE_DISC1_BIN");
    FILE *fp;
    size_t used = 0;
    int c, failed = 0, overflow_trailing = 0;

    if (!out || !out_size) return -1;
    out[0] = '\0';
    if (env && env[0]) {
        int n = snprintf(out, out_size, "%s", env);
        if (n < 0 || (size_t)n >= out_size) { out[0] = '\0'; return -1; }
        return 0;
    }
    fp = fopen("local/pe_disc1.path", "r");
    if (!fp) return -1;
    while ((c = fgetc(fp)) != EOF && c != '\n') {
        if (!c) { failed = 1; break; }
        if (overflow_trailing || used >= out_size - 1u) {
            if (c != ' ' && c != '\r') { failed = 1; break; }
            overflow_trailing = 1;
        } else {
            out[used++] = (char)c;
        }
    }
    if (ferror(fp)) failed = 1;
    if (fclose(fp) != 0) failed = 1;
    if (failed) { out[0] = '\0'; return -1; }
    out[used] = '\0';
    Chomp(out);
    return out[0] ? 0 : -1;
}

static int KnownExeHash(const char *sha1)
{
    unsigned i;
    for (i = 0; i < g_pe_known_disc_count; i++)
        if (g_pe_known_discs[i].exe_sha1 && !strcmp(g_pe_known_discs[i].exe_sha1, sha1))
            return 1;
    return 0;
}

static uint8_t *ReadWhole(const char *path, uint32_t *size)
{
    FILE *fp = fopen(path, "rb");
    long n;
    uint8_t *buf;
    if (!fp)
        return NULL;
    if (fseek(fp, 0, SEEK_END) != 0 || (n = ftell(fp)) <= 0 || n > (16L << 20)) {
        fclose(fp);
        return NULL;
    }
    rewind(fp);
    buf = (uint8_t *)malloc((size_t)n);
    if (buf && fread(buf, 1, (size_t)n, fp) != (size_t)n) {
        free(buf);
        buf = NULL;
    }
    fclose(fp);
    *size = (uint32_t)n;
    return buf;
}

/* Resolve one complete current-key line before opening cache contents. */
static int CacheContentPath(char *out, size_t out_size, const char *leaf)
{
    char root[768], key[128], path[1024];
    const char *env = getenv("PE_DISC_CACHE_DIR");
    int n = snprintf(root, sizeof(root), "%s", (env && env[0]) ? env : "build/disc-cache");
    if (n < 0 || (size_t)n >= sizeof(root)) return 0;
    n = snprintf(path, sizeof(path), "%s/current", root);
    if (n < 0 || (size_t)n >= sizeof(path)) return 0;
    FILE *fp = fopen(path, "r");
    if (!fp) return 0;
    size_t used = 0;
    int c, valid = 1, overflow_trailing = 0;
    while ((c = fgetc(fp)) != EOF && c != '\n') {
        if (!c) { valid = 0; break; }
        if (overflow_trailing || used >= sizeof(key) - 1u) {
            if (c != ' ' && c != '\r') { valid = 0; break; }
            overflow_trailing = 1;
        } else {
            key[used++] = (char)c;
        }
    }
    if (ferror(fp)) valid = 0;
    if (fclose(fp) != 0) valid = 0;
    if (!valid) return 0;
    key[used] = '\0';
    Chomp(key);
    if (!key[0]) return 0;
    n = snprintf(out, out_size, "%s/%s/%s", root, key, leaf);
    return n >= 0 && (size_t)n < out_size;
}

static uint8_t *FromCache(uint32_t *size)
{
    char path[1024];
    uint8_t *buf;
    char sha[41];

    if (!CacheContentPath(path, sizeof(path), "SLUS_006.62")) return NULL;
    buf = ReadWhole(path, size);
    if (!buf) return NULL;
    PE_DiscCheck_Sha1Hex(buf, *size, sha);
    if (!KnownExeHash(sha)) {
        free(buf);
        return NULL;
    }
    return buf;
}

static uint8_t *FromImage(uint32_t *size)
{
    char path[1024], err[256];
    PE_Disc *d;
    PE_DiscIdentity id;
    uint8_t *buf = NULL;

    if (PE_DiscCache_ImagePath(path, sizeof(path)) != 0) {
        snprintf(s_err, sizeof(s_err), "no disc configured (set PE_DISC1_BIN, write "
                 "local/pe_disc1.path, or run tools/extract/disc_cache.py populate)");
        return NULL;
    }
    d = PE_Disc_Open(path, err, sizeof(err));
    if (!d) {
        snprintf(s_err, sizeof(s_err), "%.120s: %.120s", path, err);
        return NULL;
    }
    /* any verified disc carries the (identical) EXE */
    if (PE_DiscCheck_Identify(d, 0xFFu, &id, err, sizeof(err)) != 0) {
        snprintf(s_err, sizeof(s_err), "%.120s: %.120s", path, err);
    } else if (PE_DiscCheck_ReadFile(d, id.known->boot, &buf, size) != 0) {
        snprintf(s_err, sizeof(s_err), "%.220s: boot file is unreadable", path);
        buf = NULL;
    }
    PE_Disc_Close(d);
    return buf;
}

const uint8_t *PE_DiscCache_Exe(uint32_t *size, char *err, size_t err_size)
{
    if (!s_tried) {
        s_tried = 1;
        s_exe = FromCache(&s_exe_size);
        if (!s_exe)
            s_exe = FromImage(&s_exe_size);
        if (s_exe) {
            if (s_exe_size < PE_EXE_HDR || memcmp(s_exe, "PS-X EXE", 8) != 0) {
                free(s_exe);
                s_exe = NULL;
                snprintf(s_err, sizeof(s_err), "boot file is not a PS-X EXE");
            } else {
                memcpy(&s_taddr, s_exe + 0x18, 4);
                memcpy(&s_tsize, s_exe + 0x1C, 4);
                if (s_tsize > s_exe_size - PE_EXE_HDR)
                    s_tsize = s_exe_size - PE_EXE_HDR;
            }
        }
    }
    if (size)
        *size = s_exe ? s_exe_size : 0;
    if (!s_exe && err && err_size)
        snprintf(err, err_size, "%s", s_err);
    return s_exe;
}

/* Mirror the guest-RAM contract without adding a runtime RAM dependency. */
static int ExeRamOffset(uint32_t address, uint32_t length, uint32_t *offset)
{
    uint32_t segment = address & 0xE0000000u;
    uint32_t physical = address & 0x1FFFFFFFu;
    if ((segment != 0u && segment != 0x80000000u && segment != 0xA0000000u) ||
        physical >= PE_RAM_SIZE || length > PE_RAM_SIZE - physical)
        return 0;
    *offset = physical;
    return 1;
}

int PE_DiscCache_ExeRead(uint32_t addr, void *out, uint32_t len)
{
    uint32_t base, request, off;
    if ((!out && len) || !PE_DiscCache_Exe(NULL, NULL, 0))
        return -1;
    if (!ExeRamOffset(s_taddr, s_tsize, &base) ||
        !ExeRamOffset(addr, len, &request) || request < base)
        return -1;
    off = request - base;
    if (off > s_tsize || len > s_tsize - off)
        return -1;
    /* A zero-length request may use NULL, but still needs a valid address. */
    if (len) memcpy(out, s_exe + PE_EXE_HDR + off, len);
    return 0;
}

int PE_DiscCache_ExeWord(uint32_t addr, uint32_t *out)
{
    uint8_t b[4];
    if (!out || PE_DiscCache_ExeRead(addr, b, 4) != 0)
        return -1;
    *out = (uint32_t)b[0] | (uint32_t)b[1] << 8 | (uint32_t)b[2] << 16 | (uint32_t)b[3] << 24;
    return 0;
}

/* ── PE.IMG ─────────────────────────────────────────────────────────── */
static PE_Disc *s_disc;
static uint32_t s_peimg_lba, s_peimg_size;
static FILE *s_peimg_fp;
static int s_peimg_tried;
static char s_peimg_err[256];

static void OpenPeImg(void)
{
    char path[1024], err[256];
    PE_DiscIdentity id;
    s_peimg_tried = 1;
    if (PE_DiscCache_ImagePath(path, sizeof(path)) == 0) {
        s_disc = PE_Disc_Open(path, err, sizeof(err));
        if (!s_disc) {
            snprintf(s_peimg_err, sizeof(s_peimg_err), "%.120s: %.120s", path, err);
        } else if (PE_DiscCheck_Identify(s_disc, 0xFFu, &id, err, sizeof(err)) != 0) {
            snprintf(s_peimg_err, sizeof(s_peimg_err), "%.120s: %.120s", path, err);
        } else if (PE_Disc_FindFile(s_disc, "\\PE.IMG;1", &s_peimg_lba, &s_peimg_size, NULL, 0)) {
            return;
        } else {
            snprintf(s_peimg_err, sizeof(s_peimg_err), "%.220s: PE.IMG;1 not found or invalid", path);
        }
        if (s_disc)
            PE_Disc_Close(s_disc);
        s_disc = NULL;
    }
    {
        char p2[1024];
        if (CacheContentPath(p2, sizeof(p2), "PE.IMG")) {
            s_peimg_fp = fopen(p2, "rb");
            if (s_peimg_fp) {
                long end;
                if (fseek(s_peimg_fp, 0, SEEK_END) != 0 ||
                    (end = ftell(s_peimg_fp)) < 0 || (uint64_t)end > UINT32_MAX) {
                    fclose(s_peimg_fp);
                    s_peimg_fp = NULL;
                    s_peimg_size = 0;
                    if (!s_peimg_err[0])
                        snprintf(s_peimg_err, sizeof(s_peimg_err),
                                 "cached PE.IMG size unavailable or unsupported");
                } else {
                    s_peimg_size = (uint32_t)end;
                    /* the cache's EXE must also be verified before PE.IMG is used */
                    if (PE_DiscCache_Exe(NULL, NULL, 0) == NULL) {
                        fclose(s_peimg_fp);
                        s_peimg_fp = NULL;
                        s_peimg_size = 0;
                    }
                }
            }
        }
        if (!s_peimg_fp && !s_peimg_err[0])
            snprintf(s_peimg_err, sizeof(s_peimg_err), "no disc configured and no disc cache");
    }
}

int PE_DiscCache_FileRead(int src, uint32_t off, void *out, uint32_t len,
                          char *err, size_t err_size)
{
    if (src == PE_DISC_SRC_EXE) {
        uint32_t size;
        const uint8_t *exe = PE_DiscCache_Exe(&size, err, err_size);
        if (!exe)
            return -1;
        if (off > size || len > size - off) {
            if (err && err_size) snprintf(err, err_size, "EXE range out of bounds");
            return -1;
        }
        memcpy(out, exe + off, len);
        return 0;
    }
    if (src != PE_DISC_SRC_PEIMG)
        return -1;
    if (!s_peimg_tried)
        OpenPeImg();
    if (off > s_peimg_size || len > s_peimg_size - off) {
        if (err && err_size)
            snprintf(err, err_size, "%s", s_peimg_err[0] ? s_peimg_err : "PE.IMG range out of bounds");
        return -1;
    }
    if (s_disc)
        return PE_Disc_ReadUserData(s_disc, s_peimg_lba + off / PE_DISC_USER_SECTOR,
                                    off % PE_DISC_USER_SECTOR, out, len) ? 0 : -1;
    if (s_peimg_fp && fseek(s_peimg_fp, (long)off, SEEK_SET) == 0 &&
        fread(out, 1, len, s_peimg_fp) == len)
        return 0;
    return -1;
}
