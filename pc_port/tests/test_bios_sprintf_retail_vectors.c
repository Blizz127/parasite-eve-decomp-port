/* Host formatter (PE_Bios_SprintfHost) against black-box retail vectors of
 * the EXE formatter func_80071A84: bios_sprintf_retail_vectors.tsv, generated
 * by pc_port/tools/bios_sprintf_oracle.py from the user's own retail EXE.
 * Synthetic formats and integers only: no game strings, saves or devices.
 *
 * Retail is not host C for alternate-form zero: "%#x" of 0 is "0x0" and
 * "%#.0o" of 0 is empty.  The snprintf-comparison tests skip those rows; this
 * test owns them.  Each row also checks truncation at small capacities.
 *
 * Usage: test_bios_sprintf_retail_vectors [vectors.tsv]   (cwd: repository root) */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pe_bios_string.h"

static const char *guest_format;
uint8_t PE_LoadU8(pe_addr_t address)
{
    size_t length = strlen(guest_format);
    if (address < 1 || (size_t)(address - 1) > length) abort();
    return (uint8_t)guest_format[address - 1];
}
void PE_StoreU16(pe_addr_t a, uint16_t v) { (void)a; (void)v; abort(); }
void PE_StoreU32(pe_addr_t a, uint32_t v) { (void)a; (void)v; abort(); }

int main(int argc, char **argv)
{
    const char *path = argc > 1 ? argv[1] : "pc_port/tests/bios_sprintf_retail_vectors.tsv";
    static const uint32_t caps[] = {1, 2, 8};
    char line[1024];
    unsigned rows = 0, failed = 0;
    FILE *f = fopen(path, "r");
    if (!f) { fprintf(stderr, "cannot open %s (run from the repository root)\n", path); return 2; }
    while (fgets(line, sizeof line, f)) {
        char *fmt = line, *words, *ret_text, *want, *end;
        uint32_t args[8];
        int nargs = 0, want_ret, got;
        char actual[512];
        size_t want_len;
        if (line[0] == '#') continue;
        line[strcspn(line, "\n")] = 0;
        if (!(words = strchr(fmt, '\t'))) goto malformed;
        *words++ = 0;
        if (!(ret_text = strchr(words, '\t'))) goto malformed;
        *ret_text++ = 0;
        if (!(want = strchr(ret_text, '\t'))) goto malformed;
        *want++ = 0;
        want_len = strlen(want);
        if (want_len < 2 || want[0] != '[' || want[want_len - 1] != ']') goto malformed;
        want[want_len - 1] = 0; want++; want_len -= 2;
        want_ret = (int)strtol(ret_text, &end, 10);
        if (*end) goto malformed;
        for (char *p = words; *p && nargs < 8;) {
            args[nargs++] = (uint32_t)strtoul(p, &end, 16);
            if (end == p) goto malformed;
            p = *end == ',' ? end + 1 : end;
        }
        rows++;
        guest_format = fmt;
        got = PE_Bios_SprintfHost(actual, sizeof actual, 1, args, nargs);
        if (got != want_ret || strcmp(actual, want) != 0) {
            fprintf(stderr, "row %u \"%s\" (%s): host \"%s\" (%d), retail \"%s\" (%d)\n",
                    rows, fmt, words, actual, got, want, want_ret);
            failed++;
            continue;
        }
        for (unsigned k = 0; k < sizeof caps / sizeof caps[0]; k++) {
            size_t keep = want_len < caps[k] - 1u ? want_len : caps[k] - 1u;
            memset(actual, 0x7F, sizeof actual);
            got = PE_Bios_SprintfHost(actual, caps[k], 1, args, nargs);
            if (got != want_ret || strlen(actual) != keep || memcmp(actual, want, keep) != 0 ||
                (unsigned char)actual[caps[k]] != 0x7F) {
                fprintf(stderr, "row %u \"%s\" (%s): truncation at capacity %u\n",
                        rows, fmt, words, (unsigned)caps[k]);
                failed++;
                break;
            }
        }
        continue;
malformed:
        fprintf(stderr, "malformed vector line: %s\n", line);
        fclose(f);
        return 2;
    }
    fclose(f);
    if (!rows) { fputs("no vectors read\n", stderr); return 2; }
    if (failed) { fprintf(stderr, "%u of %u retail formatter vectors differ\n", failed, rows); return 1; }
    printf("%u retail formatter vectors match, each also truncated at 3 capacities\n", rows);
    return 0;
}
