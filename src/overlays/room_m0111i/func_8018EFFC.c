/* room_m0111i (PE.IMG room m0111i chunk 2, VRAM 0x8018EFE8)
 * func_8018EFFC — blob offset 0x14, 0x2c bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0049i func_8018F03C; C re-targeted by symbol address
 * (docs/evidence/room_m0111i-ports-2026-09-23/REPORT.md). */

extern int *func_800C22F8();
extern char D_80190104[];
int func_8018EFFC(void)
{
    *func_800C22F8() = (int)D_80190104;
    return 0;
}
