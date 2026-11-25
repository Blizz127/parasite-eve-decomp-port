/*
 * Phase 6E-A batch 3 — Read-only host access to the user-supplied Disc 1
 * image.  See pe_disc.h for the format contract.
 */
#include "pe_disc.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct PE_Disc {
    FILE          *fp;          /* non-NULL for file-backed images   */
    const uint8_t *mem;         /* non-NULL for memory fixtures      */
    uint32_t       user_sectors;
};

static PE_Disc *g_active_disc = NULL;

void PE_Disc_SetActive(PE_Disc *d) { g_active_disc = d; }
PE_Disc *PE_Disc_GetActive(void)   { return g_active_disc; }

uint32_t PE_Disc_UserSectorCount(const PE_Disc *d)
{
    return d ? d->user_sectors : 0;
}

/* ── Validation helpers ─────────────────────────────────────────────── */

static bool PE_Disc_CheckSync(const uint8_t raw[PE_DISC_RAW_SECTOR])
{
    static const uint8_t want[12] = {
        0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00
    };
    if (memcmp(raw, want, sizeof(want)) != 0) return false;
    return raw[15] == 0x02; /* mode 2 */
}

static bool PE_Disc_ReadRaw(PE_Disc *d, uint32_t raw_lba,
                            uint8_t out[PE_DISC_RAW_SECTOR])
{
    if (raw_lba >= d->user_sectors) return false;
    if (d->mem) {
        memcpy(out, d->mem + (size_t)raw_lba * PE_DISC_RAW_SECTOR,
               PE_DISC_RAW_SECTOR);
        return true;
    }
    if (fseek(d->fp, (long)((uint64_t)raw_lba * PE_DISC_RAW_SECTOR),
              SEEK_SET) != 0)
        return false;
    return fread(out, 1, PE_DISC_RAW_SECTOR, d->fp) == PE_DISC_RAW_SECTOR;
}

static bool PE_Disc_Validate(PE_Disc *d, char *err, size_t err_size)
{
    uint8_t raw[PE_DISC_RAW_SECTOR];
    if (d->user_sectors < 17) {
        snprintf(err, err_size, "image too small for an ISO9660 PVD");
        return false;
    }
    if (!PE_Disc_ReadRaw(d, 0, raw) || !PE_Disc_CheckSync(raw)) {
        snprintf(err, err_size,
                 "sector 0 sync/mode check failed (not MODE2/2352)");
        return false;
    }
    return true;
}

/* ── Cue sheet ──────────────────────────────────────────────────────── */

/* Recognize a complete TRACK directive, not a comment or file name that
 * happens to contain the mode token. Keywords are ASCII case-insensitive. */
static bool PE_Cue_WordEqual(const char *a, const char *b)
{
    while (*a && *b) {
        unsigned char x = (unsigned char)*a++, y = (unsigned char)*b++;
        if (x >= 'a' && x <= 'z') x = (unsigned char)(x - 'a' + 'A');
        if (y >= 'a' && y <= 'z') y = (unsigned char)(y - 'a' + 'A');
        if (x != y) return false;
    }
    return !*a && !*b;
}

/* 0 is not a TRACK directive, 1 is the supported track, -1 is an
 * unsupported or incomplete TRACK directive. */
static int PE_Cue_TrackKind(const char *line)
{
    char directive[16], number[16], mode[32];
    int fields = sscanf(line, " %15s %15s %31s", directive, number, mode);
    if (fields < 1 || !PE_Cue_WordEqual(directive, "TRACK")) return 0;
    if (fields != 3 || !PE_Cue_WordEqual(mode, "MODE2/2352")) return -1;
    const char *p = number;
    while (*p == '0') ++p;
    return strcmp(p, "1") == 0 ? 1 : -1;
}

/* Track-start INDEX 01 must agree with the raw mapping: LBA 0 is byte 0.
 * 0 is not INDEX, 1 is zero-start INDEX 01, 2 is another numeric INDEX,
 * and -1 is an incomplete or unsupported track-start declaration. */
static int PE_Cue_IndexKind(const char *line)
{
    char directive[16], number[16], time[32];
    int number_end = -1;
    int fields = sscanf(line, " %15s %15s%n %31s", directive, number, &number_end, time);
    if (fields < 1 || !PE_Cue_WordEqual(directive, "INDEX")) return 0;
    if (fields != 3 || number_end < 0 ||
        !isspace((unsigned char)line[number_end])) return -1;
    for (const char *p = number; *p; ++p)
        if (*p < '0' || *p > '9') return -1;
    const char *p = number;
    while (*p == '0') ++p;
    if (strcmp(p, "1") != 0) return 2;
    return strcmp(time, "00:00:00") == 0 ? 1 : -1;
}

/* An overlong physical line with only whitespace, or with a TRACK/INDEX
 * keyword split at its first chunk boundary, cannot be classified safely. */
static bool PE_Cue_AmbiguousPrefix(const char *line)
{
    while (isspace((unsigned char)*line)) ++line;
    if (!*line) return true;
    const char *keywords[] = {"TRACK", "INDEX"};
    for (unsigned k = 0; k < 2; ++k) {
        const char *p = line, *word = keywords[k];
        bool matches = true;
        while (*p && *word && !isspace((unsigned char)*p)) {
            unsigned char c = (unsigned char)*p++;
            if (c >= 'a' && c <= 'z') c = (unsigned char)(c - 'a' + 'A');
            if (c != (unsigned char)*word++) { matches = false; break; }
        }
        if (matches && !*p && *word) return true;
    }
    return false;
}

/* Parse the sibling .cue of bin_path, if it exists.  Returns:
 *   1  cue found and declares only track 1 in MODE2/2352
 *   0  no sibling cue exists
 *  -1  cue cannot be read completely or has unsupported tracks */
static int PE_Disc_CheckCue(const char *bin_path)
{
    char cue[4096];
    FILE *fp;
    char line[512];
    int ok = 0, invalid_track = 0, index01_seen = 0, at_line_start = 1;

    int path_len = snprintf(cue, sizeof(cue), "%s", bin_path);
    if (path_len < 0 || (size_t)path_len >= sizeof(cue)) return -1;
    {
        char *dot = strrchr(cue, '.');
        char *slash = strrchr(cue, '/');
        /* Windows separators: a backslash also ends the directory part. */
        char *backslash = strrchr(cue, '\\');
        if (backslash && (!slash || backslash > slash)) slash = backslash;
        if (dot && (!slash || dot > slash)) {
            if ((size_t)(dot - cue) > sizeof(cue) - sizeof(".cue"))
                return -1;
            strcpy(dot, ".cue");
        } else {
            size_t n = strlen(cue);
            if (n + 4 >= sizeof(cue)) return -1;
            strcpy(cue + n, ".cue");
        }
    }
    fp = fopen(cue, "r");
    if (!fp) return errno == ENOENT ? 0 : -1;
    while (fgets(line, sizeof(line), fp)) {
        size_t n = strlen(line);
        int complete = n < sizeof(line) - 1u || (n && line[n - 1u] == '\n');
        if (at_line_start) {
            int track = PE_Cue_TrackKind(line);
            int index = PE_Cue_IndexKind(line);
            if (!complete && PE_Cue_AmbiguousPrefix(line)) invalid_track = 1;
            if (track && (!complete || track < 0 || ok)) invalid_track = 1;
            else if (track > 0) ok = 1;
            if (index && (!complete || index < 0 || !ok || (index == 1 && index01_seen)))
                invalid_track = 1;
            else if (index == 1) index01_seen = 1;
        }
        at_line_start = complete;
    }
    int failed = ferror(fp);
    if (fclose(fp) != 0) failed = 1;
    return !failed && ok && !invalid_track ? 1 : -1;
}

/* ── Lifecycle ──────────────────────────────────────────────────────── */

PE_Disc *PE_Disc_Open(const char *bin_path, char *err, size_t err_size)
{
    PE_Disc *d;
    long end;
    int cue;

    if (!bin_path) {
        snprintf(err, err_size, "no image path");
        return NULL;
    }
    d = calloc(1, sizeof(*d));
    if (!d) {
        snprintf(err, err_size, "out of memory");
        return NULL;
    }
    d->fp = fopen(bin_path, "rb");
    if (!d->fp) {
        snprintf(err, err_size, "cannot open '%s' read-only", bin_path);
        free(d);
        return NULL;
    }
    if (fseek(d->fp, 0, SEEK_END) != 0 || (end = ftell(d->fp)) <= 0) {
        snprintf(err, err_size, "cannot size '%s'", bin_path);
        fclose(d->fp);
        free(d);
        return NULL;
    }
    if ((uint64_t)end % PE_DISC_RAW_SECTOR != 0) {
        snprintf(err, err_size,
                 "size %ld is not a multiple of 2352 (not a raw MODE2 image)",
                 end);
        fclose(d->fp);
        free(d);
        return NULL;
    }
    if ((uint64_t)end / PE_DISC_RAW_SECTOR > UINT32_MAX) {
        snprintf(err, err_size, "image has too many sectors");
        fclose(d->fp);
        free(d);
        return NULL;
    }
    d->user_sectors = (uint32_t)((uint64_t)end / PE_DISC_RAW_SECTOR);

    cue = PE_Disc_CheckCue(bin_path);
    if (cue < 0) {
        snprintf(err, err_size,
                 "sibling .cue unreadable or unsupported track/index layout");
        fclose(d->fp);
        free(d);
        return NULL;
    }
    if (!PE_Disc_Validate(d, err, err_size)) {
        fclose(d->fp);
        free(d);
        return NULL;
    }
    return d;
}

PE_Disc *PE_Disc_OpenMemory(const uint8_t *image, size_t size)
{
    PE_Disc *d;
    char err[128];

    if (!image || size % PE_DISC_RAW_SECTOR != 0 ||
        (uint64_t)size / PE_DISC_RAW_SECTOR > UINT32_MAX) return NULL;
    d = calloc(1, sizeof(*d));
    if (!d) return NULL;
    d->mem = image;
    d->user_sectors = (uint32_t)(size / PE_DISC_RAW_SECTOR);
    if (!PE_Disc_Validate(d, err, sizeof(err))) {
        free(d);
        return NULL;
    }
    return d;
}

void PE_Disc_Close(PE_Disc *d)
{
    if (!d) return;
    if (g_active_disc == d) g_active_disc = NULL;
    if (d->fp) fclose(d->fp);
    free(d);
}

/* ── Reads ──────────────────────────────────────────────────────────── */

bool PE_Disc_ReadRawSector(const PE_Disc *d, uint32_t lba,
                           uint8_t out[PE_DISC_RAW_SECTOR])
{
    return d && out && PE_Disc_ReadRaw((PE_Disc *)d, lba, out);
}

bool PE_Disc_ReadUserSector(const PE_Disc *d, uint32_t lba,
                            uint8_t out[PE_DISC_USER_SECTOR])
{
    uint8_t raw[PE_DISC_RAW_SECTOR];
    if (!d || !out) return false;
    if (!PE_Disc_ReadRaw((PE_Disc *)d, lba, raw)) return false;
    memcpy(out, raw + PE_DISC_USER_OFFSET, PE_DISC_USER_SECTOR);
    return true;
}

bool PE_Disc_ReadUserData(const PE_Disc *d, uint32_t lba, uint32_t offset,
                          void *out_v, uint32_t len)
{
    uint8_t *out = out_v;
    uint8_t sector[PE_DISC_USER_SECTOR];

    if (!d || !out) return false;
    if (offset >= PE_DISC_USER_SECTOR) return false;
    /* Validate the entire logical range before copying any bytes. Widen
     * before arithmetic so large lengths/LBAs cannot wrap into the image. */
    uint64_t start=(uint64_t)lba*PE_DISC_USER_SECTOR+offset;
    if(start+len>(uint64_t)d->user_sectors*PE_DISC_USER_SECTOR) return false;
    while (len > 0) {
        uint32_t chunk = PE_DISC_USER_SECTOR - offset;
        if (chunk > len) chunk = len;
        if (!PE_Disc_ReadUserSector(d, lba, sector)) return false;
        memcpy(out, sector + offset, chunk);
        out += chunk;
        len -= chunk;
        offset = 0;
        lba++;
    }
    return true;
}

/* ── ISO9660 ────────────────────────────────────────────────────────── */

static uint32_t PE_Iso_Le32(const uint8_t *p)
{
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

/* Validate that [extent, extent + ceil(size/2048)) lies inside the image. */
static bool PE_Iso_ExtentInRange(const PE_Disc *d, uint32_t extent,
                                 uint32_t size)
{
    uint64_t last = (uint64_t)extent + ((uint64_t)size + PE_DISC_USER_SECTOR - 1) /
                    PE_DISC_USER_SECTOR;
    if (size == 0) last = (uint64_t)extent;
    return last <= d->user_sectors;
}

static bool PE_Iso_IsPVD(const uint8_t *pvd)
{
    return pvd[0] == 1 && memcmp(pvd + 1, "CD001", 5) == 0 && pvd[6] == 1;
}

bool PE_Disc_VerifyPVD(const PE_Disc *d)
{
    uint8_t pvd[PE_DISC_USER_SECTOR];
    if (!PE_Disc_ReadUserSector(d, 16, pvd)) return false;
    return PE_Iso_IsPVD(pvd);
}

bool PE_Disc_VolumeId(const PE_Disc *d, char *out, size_t out_size)
{
    uint8_t pvd[PE_DISC_USER_SECTOR];
    size_t n;
    if (!d || !out || out_size == 0) return false;
    if (!PE_Disc_ReadUserSector(d, 16, pvd)) return false;
    if (!PE_Iso_IsPVD(pvd)) return false;
    /* Volume identifier is 32 bytes at PVD+40, space-padded. */
    n = 32;
    while (n > 0 && pvd[40 + n - 1] == ' ') n--;
    if (n >= out_size) n = out_size - 1;
    memcpy(out, pvd + 40, n);
    out[n] = '\0';
    return true;
}

int PE_Disc_BootKind(const PE_Disc *d)
{
    uint32_t lba, size;
    int disc1, disc2;
    if (!d) return 0;
    disc1 = PE_Disc_FindFile(d, "\\SLUS_006.62;1", &lba, &size, NULL, 0);
    disc2 = PE_Disc_FindFile(d, "\\SLUS_006.68;1", &lba, &size, NULL, 0);
    if (disc1 && !disc2) return 1;
    if (disc2 && !disc1) return 2;
    return 0;
}

/* Read the root directory record embedded in the PVD (offset 156). */
static bool PE_Iso_RootRecord(const PE_Disc *d, uint32_t *extent,
                              uint32_t *size)
{
    uint8_t pvd[PE_DISC_USER_SECTOR];
    const uint8_t *rec;
    if (!PE_Disc_ReadUserSector(d, 16, pvd)) return false;
    if (!PE_Iso_IsPVD(pvd)) return false;
    rec = pvd + 156;
    /* The embedded root record is a directory identified by the single
     * special zero byte; reject an unrelated or truncated record. */
    if (rec[0] < 34 || !(rec[25] & 0x02u) || rec[32] != 1u || rec[33] != 0u)
        return false;
    *extent = PE_Iso_Le32(rec + 2);
    *size = PE_Iso_Le32(rec + 10);
    return PE_Iso_ExtentInRange(d, *extent, *size > 0 ? *size : 1);
}

/* Scan one directory extent for component.  Returns 1 found, 0 not found,
 * -1 malformed. */
static int PE_Iso_ScanDir(const PE_Disc *d, uint32_t dir_extent,
                          uint32_t dir_size, const char *component,
                          uint32_t *extent, uint32_t *size, int *is_dir,
                          char *name_out, size_t name_size)
{
    uint8_t *buf;
    uint32_t pos = 0;
    int result = 0;
    size_t comp_len = strlen(component);

    if (dir_size == 0 || dir_size > 64u * 1024u * 1024u) return -1;
    if (!PE_Iso_ExtentInRange(d, dir_extent, dir_size)) return -1;
    buf = malloc(dir_size);
    if (!buf) return -1;
    if (!PE_Disc_ReadUserData(d, dir_extent, 0, buf, dir_size)) {
        free(buf);
        return -1;
    }
    while (pos < dir_size) {
        uint32_t in_sector = pos % PE_DISC_USER_SECTOR;
        uint8_t len_dr;
        const uint8_t *rec;
        uint8_t id_len;

        if (in_sector == 0 && buf[pos] == 0) {
            pos += PE_DISC_USER_SECTOR;
            continue;
        }
        if (buf[pos] == 0) {
            /* padding to the end of this sector */
            pos += PE_DISC_USER_SECTOR - in_sector;
            continue;
        }
        len_dr = buf[pos];
        if (len_dr < 34 || pos + len_dr > dir_size ||
            in_sector + len_dr > PE_DISC_USER_SECTOR) {
            result = -1; /* malformed record: fail safely */
            break;
        }
        rec = buf + pos;
        id_len = rec[32];
        if (33u + id_len > len_dr) {
            result = -1;
            break;
        }
        /* skip "." (0x00) and ".." (0x01) entries */
        if (!(id_len == 1 && (rec[33] == 0x00 || rec[33] == 0x01)) &&
            id_len == comp_len &&
            memcmp(rec + 33, component, comp_len) == 0) {
            *extent = PE_Iso_Le32(rec + 2);
            *size = PE_Iso_Le32(rec + 10);
            *is_dir = (rec[25] & 0x02) != 0;
            if (name_out && name_size > 0) {
                size_t n = id_len < name_size - 1 ? id_len : name_size - 1;
                memcpy(name_out, rec + 33, n);
                name_out[n] = '\0';
            }
            result = 1;
            break;
        }
        pos += len_dr;
    }
    free(buf);
    return result;
}

bool PE_Disc_FindFile(const PE_Disc *d, const char *path,
                      uint32_t *extent_lba, uint32_t *size,
                      char *name_out, size_t name_size)
{
    uint32_t extent, dir_size;
    const char *p;
    int depth;

    if (!d || !path || !extent_lba || !size) return false;
    if (path[0] != '\\') return false;
    if (!PE_Iso_RootRecord(d, &extent, &dir_size)) return false;

    p = path + 1;
    for (depth = 0; depth < 8; depth++) {
        char component[32], leaf_name[32];
        const char *sep = strchr(p, '\\');
        size_t len = sep ? (size_t)(sep - p) : strlen(p);
        uint32_t rec_extent, rec_size;
        int is_dir = 0;
        int r;

        if (len == 0 || len > 31) return false;
        memcpy(component, p, len);
        component[len] = '\0';

        r = PE_Iso_ScanDir(d, extent, dir_size, component,
                           &rec_extent, &rec_size, &is_dir,
                           sep ? NULL : leaf_name, sizeof(leaf_name));
        if (r <= 0) return false;
        if (!sep) {
            /* leaf component */
            if (!PE_Iso_ExtentInRange(d, rec_extent,
                                      rec_size > 0 ? rec_size : 1)) return false;
            if (name_out && name_size) {
                size_t n = strlen(leaf_name);
                if (n >= name_size) n = name_size - 1;
                memcpy(name_out, leaf_name, n);
                name_out[n] = '\0';
            }
            *extent_lba = rec_extent;
            *size = rec_size;
            return true;
        }
        if (!is_dir) return false;
        if (!PE_Iso_ExtentInRange(d, rec_extent,
                                  rec_size > 0 ? rec_size : 1))
            return false;
        extent = rec_extent;
        dir_size = rec_size;
        p = sep + 1;
    }
    return false; /* more than 8 components: retail limit */
}
