/* room_m0418i (PE.IMG room m0418i chunk 2, VRAM 0x8018EFE8)
 * func_8018FEE4 — blob offset 0xefc, 0x2c bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0049i func_8018F03C; C re-targeted by symbol address
 * (docs/evidence/room_m0418i-ports-2026-09-23/REPORT.md). */

extern int *func_800C22F8();
extern char D_8019877C[];
int func_8018FEE4(void)
{
    *func_800C22F8() = (int)D_8019877C;
    return 0;
}
