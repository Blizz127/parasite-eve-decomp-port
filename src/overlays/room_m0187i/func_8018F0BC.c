/* room_m0187i (PE.IMG room m0187i chunk 2, VRAM 0x8018EFE8)
 * func_8018F0BC — blob offset 0xd4, 0x8c bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0034i func_8018F12C; C re-targeted by symbol address
 * (docs/evidence/room_m0187i-ports-2026-09-23/REPORT.md). */

extern int func_800C6CE0();
extern int func_800C251C();
extern int func_800C2758();
extern void func_8018F148();
extern char D_8018FFA8[];
extern char D_8018FFD0[];
extern char D_8018FFE4[];
int func_8018F0BC(void *o)
{
    int r;

    if (func_800C6CE0(o) == 3) {
        r = func_800C251C(o, D_8018FFD0);
        r |= func_800C2758(o, D_8018FFA8, D_8018FFE4);
    } else {
        r = -1;
    }
    if (r == -1) {
        func_8018F148(o);
    }
    return 0;
}
