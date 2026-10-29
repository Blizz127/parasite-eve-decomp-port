/* room_m0049i (PE.IMG room m0049i chunk 2, VRAM 0x8018EFE8)
 * func_8018F03C — blob offset 0x54, 0x2c bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0167i func_8018F1EC; C re-targeted by symbol address
 * (docs/evidence/room_m0049i-ports-2026-09-23/REPORT.md). */

extern int *func_800C22F8(int slot);
extern char D_80192BE8[];
/* Init callback (func_8006F39C passes the effect slot in a0, which retail
 * hands straight on to func_800C22F8): explicit here, same code. */
int func_8018F03C(int slot)
{
    *func_800C22F8(slot) = (int)D_80192BE8;
    return 0;
}
