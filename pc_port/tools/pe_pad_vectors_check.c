/* Vector check for pe_plat_pad.c:
 *   cc -std=c99 -Wall -Wextra -Werror -Ipc_port/include -Ipc_port/platform pc_port/tools/pe_pad_vectors_check.c pc_port/platform/pe_plat_pad.c -o check && ./check vectors.txt
 * Each line: CASE port field... -> RET value WRITES n.  A field is
 * ADDR/size=VALUE (guest RAM set before the call).  The record service
 * returns 0x800A5B70, or 0x800A5C60 when (port & 0xF0) != 0. */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "pe_guest_ram.h"
#include "pe_plat/pad.h"
static uint8_t ram[0x200000], touched;
uint8_t PE_LoadU8(pe_addr_t a) { return ram[a & 0x1FFFFF]; }
uint16_t PE_LoadU16(pe_addr_t a) { uint16_t v; memcpy(&v, ram + (a & 0x1FFFFF), 2); return v; }
uint32_t PE_LoadU32(pe_addr_t a) { uint32_t v; memcpy(&v, ram + (a & 0x1FFFFF), 4); return v; }
void PE_StoreU8(pe_addr_t a, uint8_t v) { (void)a; (void)v; touched = 1; }
void PE_StoreU16(pe_addr_t a, uint16_t v) { (void)a; (void)v; touched = 1; }
void PE_StoreU32(pe_addr_t a, uint32_t v) { (void)a; (void)v; touched = 1; }
uint32_t pe_plat_pad_record(uint32_t port) { return (port & 0xF0u) ? 0x800A5C60u : 0x800A5B70u; }
int main(int c, char **v)
{
    FILE *f; char line[512]; unsigned n = 0, bad = 0;
    if (c != 2 || !(f = fopen(v[1], "r"))) return 2;
    while (fgets(line, sizeof line, f)) {
        unsigned port, want, writes; char *p = line + 5, *arrow = strstr(line, "->");
        if (!arrow || sscanf(line, "CASE %x", &port) != 1) continue;
        if (sscanf(arrow, "-> RET %x WRITES %u", &want, &writes) != 2) continue;
        memset(ram, 0, sizeof ram); touched = 0;
        p = strchr(p, ' ');
        while (p && p < arrow) {
            unsigned addr, size, val;
            if (sscanf(p, " %x/%u=%x", &addr, &size, &val) == 3) {
                if (size == 1) ram[addr & 0x1FFFFF] = (uint8_t)val;
                else memcpy(ram + (addr & 0x1FFFFF), &val, 4);
            }
            p = strchr(p + 1, ' ');
        }
        n++;
        { int got = pe_plat_pad_get_state(port);
          if ((unsigned)got != want || touched) {
              if (bad++ < 8) fprintf(stderr, "MISMATCH got %08X: %s", (unsigned)got, line);
          } }
    }
    printf("checked %u cases, %u mismatches\n", n, bad);
    return bad != 0;
}
