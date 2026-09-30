/*
 * Disc cache (host side) — see pe_disc_cache.h.
 */
#include "pe_disc_cache.h"
#include "pe_disc.h"
#include "pe_disc_check.h"
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
    if (env && env[0]) {
        snprintf(out, out_size, "%s", env);
        return 0;
    }
    fp = fopen("local/pe_disc1.path", "r");
    if (!fp)
        return -1;
    if (!fgets(out, (int)out_size, fp)) {
        fclose(fp);
        return -1;
    }
    fclose(fp);
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

static uint8_t *FromCache(uint32_t *size)
{
    char root[768], key[128], path[1024];
    const char *env = getenv("PE_DISC_CACHE_DIR");
    FILE *fp;
    uint8_t *buf;
    char sha[41];

    snprintf(root, sizeof(root), "%s", (env && env[0]) ? env : "build/disc-cache");
    snprintf(path, sizeof(path), "%s/current", root);
    fp = fopen(path, "r");
    if (!fp)
        return NULL;
    if (!fgets(key, (int)sizeof(key), fp)) {
        fclose(fp);
        return NULL;
    }
    fclose(fp);
    Chomp(key);
    snprintf(path, sizeof(path), "%s/%s/SLUS_006.62", root, key);
    buf = ReadWhole(path, size);
    if (!buf)
        return NULL;
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
        snprintf(s_err, sizeof(s_err), "%s: %s", path, err);
        return NULL;
    }
    /* any verified disc carries the (identical) EXE */
    if (PE_DiscCheck_Identify(d, 0xFFu, &id, err, sizeof(err)) != 0 ||
        PE_DiscCheck_ReadFile(d, id.known->boot, &buf, size) != 0) {
        snprintf(s_err, sizeof(s_err), "%s: %s", path, err);
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

int PE_DiscCache_ExeRead(uint32_t addr, void *out, uint32_t len)
{
    uint32_t off;
    if (!PE_DiscCache_Exe(NULL, NULL, 0))
        return -1;
    off = (addr & 0x1FFFFFFFu) - (s_taddr & 0x1FFFFFFFu);
    if ((addr & 0x1FFFFFFFu) < (s_taddr & 0x1FFFFFFFu) || off > s_tsize ||
        len > s_tsize - off)
        return -1;
    memcpy(out, s_exe + PE_EXE_HDR + off, len);
    return 0;
}

int PE_DiscCache_ExeWord(uint32_t addr, uint32_t *out)
{
    uint8_t b[4];
    if (PE_DiscCache_ExeRead(addr, b, 4) != 0)
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
        if (s_disc && PE_DiscCheck_Identify(s_disc, 0xFFu, &id, err, sizeof(err)) == 0 &&
            PE_Disc_FindFile(s_disc, "\\PE.IMG;1", &s_peimg_lba, &s_peimg_size, NULL, 0))
            return;
        snprintf(s_peimg_err, sizeof(s_peimg_err), "%s: %s", path, err);
        if (s_disc)
            PE_Disc_Close(s_disc);
        s_disc = NULL;
    }
    {
        char root[768], key[128], p2[1024];
        const char *env = getenv("PE_DISC_CACHE_DIR");
        FILE *fp;
        snprintf(root, sizeof(root), "%s", (env && env[0]) ? env : "build/disc-cache");
        snprintf(p2, sizeof(p2), "%s/current", root);
        fp = fopen(p2, "r");
        if (fp && fgets(key, (int)sizeof(key), fp)) {
            Chomp(key);
            snprintf(p2, sizeof(p2), "%s/%s/PE.IMG", root, key);
            s_peimg_fp = fopen(p2, "rb");
            if (s_peimg_fp && fseek(s_peimg_fp, 0, SEEK_END) == 0) {
                s_peimg_size = (uint32_t)ftell(s_peimg_fp);
                /* the cache's EXE must also be verified before PE.IMG is used */
                if (PE_DiscCache_Exe(NULL, NULL, 0) == NULL) {
                    fclose(s_peimg_fp);
                    s_peimg_fp = NULL;
                }
            }
        }
        if (fp)
            fclose(fp);
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
