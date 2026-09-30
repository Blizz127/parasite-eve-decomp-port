/* room_m0418i (PE.IMG room m0418i chunk 2, VRAM 0x8018EFE8)
 * func_8018FF3C — blob offset 0xf54, 0x28 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0049i func_8018F094; C re-targeted by symbol address
 * (docs/evidence/room_m0418i-ports-2026-09-23/REPORT.md). */

extern void func_800C2414();
extern char D_8019872C[];
int func_8018FF3C(void *o)
{
    func_800C2414(o, D_8019872C);
    return 0;
}
