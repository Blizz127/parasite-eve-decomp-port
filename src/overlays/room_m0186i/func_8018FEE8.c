/* room_m0186i (PE.IMG room m0186i chunk 2, VRAM 0x8018EFE8)
 * func_8018FEE8 — blob offset 0xF00, 0x48 bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the room VMA (docs/evidence/room-lane-2026-09-23/REPORT.md).
 * If func_800C6CE0(o) == 3, func_800C2414(o, D_801941E0); return 0. */

extern int func_800C6CE0();
extern void func_800C2414();
extern char D_801941E0[];
int func_8018FEE8(void *o)
{
    if (func_800C6CE0(o) == 3) {
        func_800C2414(o, D_801941E0);
    }
    return 0;
}
