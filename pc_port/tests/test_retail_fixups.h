/*
 * Retail-byte fixups for the generated oracle test headers.
 *
 * tools/analysis/strip_retail_seeds.py zeroes every retail-derived region of
 * a retail_*_cases.h header (EXE .data/.rodata, libgte tables, PE.IMG data)
 * and appends a RETAILFIX_<stem>[] table.  A test that uses such a header
 * starts with
 *
 *     TEST_RETAIL_DISC1(<name>);  TEST_RETAIL_FIXUPS(RETAILFIX_<stem>);
 *
 * which SKIPs when no disc is configured and otherwise restores the regions
 * from the user's disc (pe_disc_cache.h) before the seed tables are used.
 * Re-applying is idempotent (the restored bytes are identical every time).
 */
#ifndef TEST_RETAIL_FIXUPS_H
#define TEST_RETAIL_FIXUPS_H

#include "pe_disc_cache.h"
#include "pe_guest_ram.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

enum { PE_RF_ROWS = 0, PE_RF_BYTES = 1 };
enum { PE_RF_EXE = PE_DISC_SRC_EXE, PE_RF_PEIMG = PE_DISC_SRC_PEIMG };

typedef struct PE_TestRetailFixup {
    unsigned kind;      /* PE_RF_ROWS / PE_RF_BYTES */
    void *p;            /* ROWS: &rows[0][0] (uint32_t[][2]); BYTES: array */
    uint32_t first;     /* ROWS: first row; BYTES: byte offset */
    uint32_t count;     /* ROWS: row count; BYTES: byte count */
    int src;            /* PE_RF_EXE / PE_RF_PEIMG */
    uint32_t src_off;   /* byte offset in the source file */
} PE_TestRetailFixup;

static inline int PE_TestRetail_Apply(const PE_TestRetailFixup *f, size_t n,
                               char *err, size_t err_size)
{
    static uint8_t buf[65536];
    const uint32_t probe = 1u;
    size_t i;
    if (*(const uint8_t *)&probe != 1u) {
        snprintf(err, err_size, "retail fixups assume a little-endian host");
        return -1;
    }
    for (i = 0; i < n; i++) {
        uint32_t done = 0, total = f[i].kind == PE_RF_ROWS ? f[i].count * 4u : f[i].count;
        while (done < total) {
            uint32_t chunk = total - done > sizeof(buf) ? (uint32_t)sizeof(buf) : total - done;
            char why[256];
            why[0] = '\0';
            if (PE_DiscCache_FileRead(f[i].src, f[i].src_off + done, buf, chunk,
                                      why, sizeof(why)) != 0) {
                snprintf(err, err_size, "retail fixup %u: %s disc read failed (%s)",
                         (unsigned)i, f[i].src == PE_RF_EXE ? "EXE" : "PE.IMG",
                         why[0] ? why : "out of range");
                return -1;
            }
            if (f[i].kind == PE_RF_ROWS) {
                uint32_t *rows = (uint32_t *)f[i].p;
                uint32_t k;
                for (k = 0; k < chunk / 4u; k++) {
                    const uint8_t *b = buf + 4u * k;
                    rows[2u * (f[i].first + done / 4u + k) + 1u] =
                        (uint32_t)b[0] | (uint32_t)b[1] << 8 |
                        (uint32_t)b[2] << 16 | (uint32_t)b[3] << 24;
                }
            } else {
                memcpy((uint8_t *)f[i].p + f[i].first + done, buf, chunk);
            }
            done += chunk;
        }
    }
    return 0;
}

/* Copy len bytes of the retail EXE image at guest address vaddr into guest
 * RAM (for tests whose code reads EXE tables that production has in RAM
 * after PE_GuestImage_LoadExe).  0 on success. */
static inline int PE_TestRetail_LoadExeRange(uint32_t vaddr, uint32_t len)
{
    uint8_t b[256];
    uint32_t done = 0;
    while (done < len) {
        uint32_t n = len - done > sizeof(b) ? (uint32_t)sizeof(b) : len - done, k;
        if (PE_DiscCache_ExeRead(vaddr + done, b, n) != 0)
            return -1;
        for (k = 0; k < n; k++)
            PE_StoreU8(vaddr + done + k, b[k]);
        done += n;
    }
    return 0;
}

#define TEST_RETAIL_FIXUPS(tbl) do { \
    char rf_err_[320]; \
    if (PE_TestRetail_Apply((tbl), sizeof(tbl) / sizeof((tbl)[0]), \
                            rf_err_, sizeof(rf_err_)) != 0) { \
        FAIL(rf_err_); \
        return; \
    } \
} while (0)

#endif /* TEST_RETAIL_FIXUPS_H */
