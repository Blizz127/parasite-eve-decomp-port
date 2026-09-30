/*
 * Phase 6E-B2 — Retail executable image backing for guest RAM.
 * See pe_guest_image.h for the rationale (RNG code-as-data reads).
 */
#include "pe_guest_image.h"
#include "pe_guest_ram.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define PE_EXE_HEADER_SIZE  0x800u
#define PE_EXE_TADDR_OFF    0x18u
#define PE_EXE_TSIZE_OFF    0x1Cu
#define PE_SYSTEM_CNF       "\\SYSTEM.CNF;1"
#define PE_SYSTEM_CNF_MAX   2048u

/* Extract the BOOT = cdrom:<path> executable name from SYSTEM.CNF.
 * out receives the ISO9660 path (e.g. "\SLUS_006.62;1"). */
static int ParseBootPath(const uint8_t *cnf, uint32_t size,
                         char *out, size_t out_size)
{
    uint32_t i = 0;
    while (i < size) {
        /* line start: look for "BOOT" */
        if (i + 4 <= size && memcmp(cnf + i, "BOOT", 4) == 0) {
            i += 4;
            while (i < size && (cnf[i] == ' ' || cnf[i] == '=' ||
                                cnf[i] == '\t'))
                i++;
            if (i + 6 > size || memcmp(cnf + i, "cdrom:", 6) != 0)
                return -1;
            i += 6;
            size_t n = 0;
            while (i < size && cnf[i] != '\r' && cnf[i] != '\n' &&
                   cnf[i] != ' ' && cnf[i] != '\t') {
                if (n + 1 >= out_size)
                    return -1;
                out[n++] = (char)cnf[i++];
            }
            out[n] = '\0';
            return n > 1 ? 0 : -1;
        }
        /* skip to next line */
        while (i < size && cnf[i] != '\n')
            i++;
        if (i < size)
            i++;
    }
    return -1;
}

int PE_GuestImage_LoadExe(const PE_Disc *disc, char *err, size_t err_size)
{
    uint8_t cnf[PE_SYSTEM_CNF_MAX];
    uint8_t header[PE_DISC_USER_SECTOR];
    uint8_t chunk[PE_DISC_USER_SECTOR];
    char boot_path[64];
    uint32_t lba, size, cnf_size;
    uint32_t taddr, tsize, done;

    if (!disc) {
        snprintf(err, err_size, "no disc image");
        return -1;
    }

    /* 1. SYSTEM.CNF -> BOOT path (the BIOS boot contract). */
    if (!PE_Disc_FindFile(disc, PE_SYSTEM_CNF, &lba, &cnf_size, NULL, 0) ||
        cnf_size == 0 || cnf_size > PE_SYSTEM_CNF_MAX) {
        snprintf(err, err_size, "SYSTEM.CNF missing or oversized");
        return -1;
    }
    if (!PE_Disc_ReadUserData(disc, lba, 0, cnf, cnf_size)) {
        snprintf(err, err_size, "SYSTEM.CNF read failed");
        return -1;
    }
    if (ParseBootPath(cnf, cnf_size, boot_path, sizeof(boot_path)) != 0) {
        snprintf(err, err_size, "SYSTEM.CNF has no cdrom: BOOT path");
        return -1;
    }

    /* 2. Locate the executable and read its header sector. */
    if (!PE_Disc_FindFile(disc, boot_path, &lba, &size, NULL, 0)) {
        snprintf(err, err_size, "boot executable '%s' not found", boot_path);
        return -1;
    }
    if (!PE_Disc_ReadUserSector(disc, lba, header)) {
        snprintf(err, err_size, "boot executable header read failed");
        return -1;
    }
    if (memcmp(header, "PS-X EXE", 8) != 0) {
        snprintf(err, err_size, "'%s' is not a PS-X EXE", boot_path);
        return -1;
    }
    memcpy(&taddr, header + PE_EXE_TADDR_OFF, 4);
    memcpy(&tsize, header + PE_EXE_TSIZE_OFF, 4);

    /* 3. Geometry validation: exact file size, guest-RAM bounds. */
    if (size != PE_EXE_HEADER_SIZE + tsize || tsize == 0) {
        snprintf(err, err_size,
                 "exe size mismatch: file=%u header tsize=%u", size, tsize);
        return -1;
    }
    if (!PE_RangeIsRam(taddr, tsize)) {
        snprintf(err, err_size,
                 "exe range 0x%08X+0x%X outside guest RAM", taddr, tsize);
        return -1;
    }

    /* 4. Stream the image into guest RAM (bounds-checked throughout).
     * The PS-X EXE header is exactly one user sector, so the body begins
     * at the next sector, offset 0. */
    done = 0;
    while (done < tsize) {
        uint32_t want = tsize - done;
        if (want > sizeof(chunk))
            want = sizeof(chunk);
        if (!PE_Disc_ReadUserData(disc, lba + 1 + done / PE_DISC_USER_SECTOR,
                                  done % PE_DISC_USER_SECTOR, chunk, want)) {
            snprintf(err, err_size, "exe body read failed at +0x%X", done);
            return -1;
        }
        memcpy(PE_Translate(taddr + done, want), chunk, want);
        done += want;
    }
    return 0;
}
