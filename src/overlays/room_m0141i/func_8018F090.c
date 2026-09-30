/* room_m0141i (PE.IMG room m0141i chunk 2, VRAM 0x8018EFE8)
 * func_8018F090 — blob offset 0xa8, 0x2c bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0049i func_8018F03C; C re-targeted by symbol address
 * (docs/evidence/room_m0141i-ports-2026-09-23/REPORT.md). */

extern int *func_800C22F8();
extern char D_8019205C[];
int func_8018F090(void)
{
    *func_800C22F8() = (int)D_8019205C;
    return 0;
}
