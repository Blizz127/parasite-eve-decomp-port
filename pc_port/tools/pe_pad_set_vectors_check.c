/* Vector check for pe_plat_pad_set.c:
 *   cc -std=c99 -Wall -Wextra -Werror -Ipc_port/include -Ipc_port/platform pc_port/tools/pe_pad_set_vectors_check.c pc_port/platform/pe_plat_pad_set.c -o check
 *   ./check vectors.txt build/disc1.candidate.exe
 * Line: seed pattern KIND args... BUSY b RET r W addr=value... | W -
 * KIND ACT (port data count), ALIGN (port data), MAIN (port mode lock).
 * Guest RAM = the EXE image, then records 0x800A5B70..0x800A5D50 and zone
 * 0x80150000..0x80151000 from `seed` by xorshift32 (x ^= x<<13; x ^= x>>17;
 * x ^= x<<5; seed 0 -> 1), one generator step per record word, then per zone
 * word:
 *   pattern 0: record word = (0x80150000 + x % 0x1000) & ~3 (zone pointer)
 *   pattern 1: record word = x & 0x07070707
 *   pattern 2: pointer as in 0 if (x & 1), else x & 0x07070707
 *   pattern 3: byte i = 0xFF if bit 8*i of x is set, else 0x00
 *   pattern 4: as pattern 3; after the zone, byte +0xE4 of both records =
 *              (0,1,2,3,4,0x80,0xFF)[seed % 7]
 *   zone word = x & 0x0F0F0F0F.
 * pe_plat_pad_record: 0x800A5B70, or 0x800A5C60 if (port & 0xF0).
 * pe_plat_pad_busy returns the case's recorded BUSY value.  The written words
 * must be exactly those listed (final values); RET is compared for ALIGN and
 * MAIN (PadSetAct returns nothing in the port). */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "pe_guest_ram.h"
#include "pe_plat/pad.h"
static uint8_t ram[0x200000], exe[0x200000], wrote[0x200000 / 4];
static uint32_t cur_busy; static int busy_calls;
uint8_t PE_LoadU8(pe_addr_t a) { return ram[a & 0x1FFFFF]; }
uint16_t PE_LoadU16(pe_addr_t a) { uint16_t v; memcpy(&v, ram + (a & 0x1FFFFF), 2); return v; }
uint32_t PE_LoadU32(pe_addr_t a) { uint32_t v; memcpy(&v, ram + (a & 0x1FFFFF), 4); return v; }
void PE_StoreU8(pe_addr_t a, uint8_t v) { ram[a & 0x1FFFFF] = v; wrote[(a & 0x1FFFFF) >> 2] = 1; }
void PE_StoreU16(pe_addr_t a, uint16_t v) { memcpy(ram + (a & 0x1FFFFF), &v, 2); wrote[(a & 0x1FFFFF) >> 2] = 1; }
void PE_StoreU32(pe_addr_t a, uint32_t v) { memcpy(ram + (a & 0x1FFFFF), &v, 4); wrote[(a & 0x1FFFFF) >> 2] = 1; }
uint32_t pe_plat_pad_record(uint32_t port) { return (port & 0xF0u) ? 0x800A5C60u : 0x800A5B70u; }
int pe_plat_pad_busy(uint32_t record) { (void)record; busy_calls++; return (int)cur_busy; }
static uint32_t xs(uint32_t x) { x ^= x << 13; x ^= x >> 17; x ^= x << 5; return x; }
static void fill(uint32_t seed, int pattern)
{
    uint32_t x = seed ? seed : 1u, a;
    for (a = 0x800A5B70u; a < 0x800A5D50u; a += 4u) {
        uint32_t val; int i;
        x = xs(x);
        if (pattern == 3 || pattern == 4) { val = 0; for (i = 0; i < 4; i++) if ((x >> (8 * i)) & 1u) val |= 0xFFu << (8 * i); }
        else if (pattern == 0 || (pattern == 2 && (x & 1u))) val = (0x80150000u + (x % 0x1000u)) & ~3u;
        else val = x & 0x07070707u;
        memcpy(ram + (a & 0x1FFFFF), &val, 4);
    }
    for (a = 0x80150000u; a < 0x80151000u; a += 4u) { uint32_t val; x = xs(x); val = x & 0x0F0F0F0Fu; memcpy(ram + (a & 0x1FFFFF), &val, 4); }
    if (pattern == 4) {
        static const uint8_t e4[7] = {0, 1, 2, 3, 4, 0x80, 0xFF};
        ram[(0x800A5B70u + 0xE4u) & 0x1FFFFF] = e4[(seed ? seed : 1u) % 7u];
        ram[(0x800A5C60u + 0xE4u) & 0x1FFFFF] = e4[(seed ? seed : 1u) % 7u];
    }
}
int main(int c, char **v)
{
    FILE *f, *e; static char line[4096]; unsigned n = 0, bad = 0, t_addr, t_size;
    if (c != 3 || !(f = fopen(v[1], "r")) || !(e = fopen(v[2], "rb"))) return 2;
    { static uint8_t img[0x200000]; size_t k = fread(img, 1, sizeof img, e); (void)k;
      memcpy(&t_addr, img + 0x18, 4); memcpy(&t_size, img + 0x1C, 4);
      memcpy(exe + (t_addr & 0x1FFFFF), img + 0x800, t_size); }
    while (fgets(line, sizeof line, f)) {
        char *tok[64]; int nt = 0, i, kind, ok = 1; unsigned seed, pattern, a0, a1, a2 = 0, busy, want; char *s = line;
        while (nt < 64 && (tok[nt] = strtok(nt ? NULL : s, " \n"))) nt++;
        if (nt < 8) continue;
        seed = (unsigned)strtoul(tok[0], 0, 10); pattern = (unsigned)strtoul(tok[1], 0, 10);
        kind = !strcmp(tok[2], "ACT") ? 0 : !strcmp(tok[2], "ALIGN") ? 1 : 2;
        a0 = (unsigned)strtoul(tok[3], 0, 16); a1 = (unsigned)strtoul(tok[4], 0, 16);
        i = 5; if (kind != 1) a2 = (unsigned)strtoul(tok[i++], 0, 16);
        busy = (unsigned)strtoul(tok[i + 1], 0, 16); want = (unsigned)strtoul(tok[i + 3], 0, 16); i += 5;
        memcpy(ram, exe, sizeof ram); memset(wrote, 0, sizeof wrote); fill(seed, (int)pattern);
        cur_busy = busy; busy_calls = 0; n++;
        { uint32_t got = 0;
          if (kind == 0) pe_plat_pad_set_act(a0, a1, a2);
          else if (kind == 1) got = (uint32_t)pe_plat_pad_set_act_align(a0, a1);
          else got = (uint32_t)pe_plat_pad_set_main_mode(a0, a1, a2);
          if (kind != 0 && got != want) ok = 0;
          { unsigned k, listed = 0;
            for (; i < nt; i++) { unsigned addr, val, have;
                if (!strcmp(tok[i], "-")) break;
                if (sscanf(tok[i], "%x=%x", &addr, &val) != 2) continue;
                memcpy(&have, ram + (addr & 0x1FFFFF), 4); listed++;
                if (have != val || !wrote[(addr & 0x1FFFFF) >> 2]) ok = 0;
                wrote[(addr & 0x1FFFFF) >> 2] = 0; }
            for (k = 0; k < sizeof wrote; k++) if (wrote[k]) { ok = 0; break; }
            (void)listed; } }
        if (!ok && bad++ < 8) fprintf(stderr, "MISMATCH line %u: %.180s\n", n, line);
    }
    printf("checked %u cases, %u mismatches\n", n, bad);
    return bad != 0;
}
