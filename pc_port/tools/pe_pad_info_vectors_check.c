/* Vector check for pe_plat_pad_info.c:
 *   cc -std=c99 -Wall -Wextra -Werror -Ipc_port/include -Ipc_port/platform pc_port/tools/pe_pad_info_vectors_check.c pc_port/platform/pe_plat_pad_info.c -o check
 *   ./check vectors.txt build/disc1.candidate.exe
 * Each line: seed pattern port term offs RET writes.  Guest RAM = the EXE
 * image, then the records 0x800A5B70..0x800A5D50 and the table zone
 * 0x80150000..0x80151000 filled from `seed` by xorshift32:
 *   pattern 0: every record word is a word-aligned pointer into the zone;
 *   pattern 1: every record word is (x & 0x07070707);
 *   pattern 2: per word, pointer if (x & 1) else (x & 0x07070707);
 *   zone words are (x & 0x0F0F0F0F).
 * The record service returns 0x800A5B70, or 0x800A5C60 if (port & 0xF0). */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "pe_guest_ram.h"
#include "pe_plat/pad.h"
static uint8_t ram[0x200000], exe[0x200000]; static int touched;
uint8_t PE_LoadU8(pe_addr_t a) { return ram[a & 0x1FFFFF]; }
uint16_t PE_LoadU16(pe_addr_t a) { uint16_t v; memcpy(&v, ram + (a & 0x1FFFFF), 2); return v; }
uint32_t PE_LoadU32(pe_addr_t a) { uint32_t v; memcpy(&v, ram + (a & 0x1FFFFF), 4); return v; }
void PE_StoreU8(pe_addr_t a, uint8_t v) { (void)a; (void)v; touched = 1; }
void PE_StoreU16(pe_addr_t a, uint16_t v) { (void)a; (void)v; touched = 1; }
void PE_StoreU32(pe_addr_t a, uint32_t v) { (void)a; (void)v; touched = 1; }
uint32_t pe_plat_pad_record(uint32_t port) { return (port & 0xF0u) ? 0x800A5C60u : 0x800A5B70u; }
static uint32_t xs(uint32_t x) { x ^= x << 13; x ^= x >> 17; x ^= x << 5; return x; }
static void fill(uint32_t seed, int pattern)
{
    uint32_t x = seed ? seed : 1u, a;
    for (a = 0x800A5B70u; a < 0x800A5D50u; a += 4u) {
        uint32_t val;
        x = xs(x);
        if (pattern == 0 || (pattern == 2 && (x & 1u))) val = (0x80150000u + (x % 0x1000u)) & ~3u;
        else val = x & 0x07070707u;
        memcpy(ram + (a & 0x1FFFFF), &val, 4);
    }
    for (a = 0x80150000u; a < 0x80151000u; a += 4u) {
        uint32_t val;
        x = xs(x); val = x & 0x0F0F0F0Fu;
        memcpy(ram + (a & 0x1FFFFF), &val, 4);
    }
}
int main(int c, char **v)
{
    FILE *f, *e; char line[256]; unsigned n = 0, bad = 0, t_addr, t_size;
    if (c != 3 || !(f = fopen(v[1], "r")) || !(e = fopen(v[2], "rb"))) return 2;
    { static uint8_t img[0x200000]; size_t k = fread(img, 1, sizeof img, e); (void)k;
      memcpy(&t_addr, img + 0x18, 4); memcpy(&t_size, img + 0x1C, 4);
      memcpy(exe + (t_addr & 0x1FFFFF), img + 0x800, t_size); }
    while (fgets(line, sizeof line, f)) {
        unsigned seed, port, want, writes; int pattern, term, offs;
        if (sscanf(line, "%u %d %x %d %d %x %u", &seed, &pattern, &port, &term, &offs, &want, &writes) != 7) continue;
        memcpy(ram, exe, sizeof ram); touched = 0;
        fill(seed, pattern);
        n++;
        { uint32_t got = pe_plat_pad_info_mode(port, term, offs);
          if (got != want || touched) { if (bad++ < 8) fprintf(stderr, "MISMATCH got %08X: %s", got, line); } }
    }
    printf("checked %u cases, %u mismatches\n", n, bad);
    return bad != 0;
}
