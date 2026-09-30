/* room_m0167i (PE.IMG room m0167i chunk 2, VRAM 0x8018EFE8)
 * func_8018F244 — blob offset 0x25C, 0x48 bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the room VMA (docs/evidence/room-lane-2026-09-23/REPORT.md).
 * If func_800C6CE0(o) >= 2 (unsigned), func_800C2414(o, D_80193190); return 0. */

extern unsigned int func_800C6CE0();
extern void func_800C2414();
extern char D_80193190[];
int func_8018F244(void *o)
{
    if (func_800C6CE0(o) >= 2) {
        func_800C2414(o, D_80193190);
    }
    return 0;
}
