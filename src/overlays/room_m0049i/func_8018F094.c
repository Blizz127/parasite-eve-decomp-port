/* room_m0049i (PE.IMG room m0049i chunk 2, VRAM 0x8018EFE8)
 * func_8018F094 — blob offset 0xAC, 0x28 bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the room VMA (docs/evidence/room-lane-2026-09-23/REPORT.md).
 * func_800C2414(o, D_80192B7C); return 0. */

extern void func_800C2414();
extern char D_80192B7C[];
int func_8018F094(void *o)
{
    func_800C2414(o, D_80192B7C);
    return 0;
}
