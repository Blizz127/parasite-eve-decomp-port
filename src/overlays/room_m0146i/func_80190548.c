/* room_m0146i (PE.IMG room m0146i chunk 2, VRAM 0x8018EFE8)
 * func_80190548 — blob offset 0x1560, 0x2c bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0049i func_8018F03C; C re-targeted by symbol address
 * (docs/evidence/room_m0146i-ports-2026-09-23/REPORT.md). */

extern int *func_800C22F8();
extern char D_80192580[];
int func_80190548(void)
{
    *func_800C22F8() = (int)D_80192580;
    return 0;
}
