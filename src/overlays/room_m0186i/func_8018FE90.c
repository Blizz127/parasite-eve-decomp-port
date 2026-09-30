/* room_m0186i (PE.IMG room m0186i chunk 2, VRAM 0x8018EFE8)
 * func_8018FE90 — blob offset 0xea8, 0x2c bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0049i func_8018F03C; C re-targeted by symbol address
 * (docs/evidence/room_m0186i-ports-2026-09-23/REPORT.md). */

extern int *func_800C22F8();
extern char D_80194230[];
int func_8018FE90(void)
{
    *func_800C22F8() = (int)D_80194230;
    return 0;
}
