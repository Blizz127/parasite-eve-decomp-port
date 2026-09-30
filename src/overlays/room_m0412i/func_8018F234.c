/* room_m0412i (PE.IMG room m0412i chunk 2, VRAM 0x8018EFE8)
 * func_8018F234 — blob offset 0x24c, 0x2c bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0049i func_8018F03C; C re-targeted by symbol address
 * (docs/evidence/room_m0412i-ports-2026-09-23/REPORT.md). */

extern int *func_800C22F8();
extern char D_801954EC[];
int func_8018F234(void)
{
    *func_800C22F8() = (int)D_801954EC;
    return 0;
}
