/* room_m0146i (PE.IMG room m0146i chunk 2, VRAM 0x8018EFE8)
 * func_801905E8 — blob offset 0x1600, 0x8c bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0065i func_8018F09C; C re-targeted by symbol address
 * (docs/evidence/room_m0146i-ports-2026-09-23/REPORT.md). */

extern unsigned int func_800C6CE0();
extern int func_800C251C();
extern int func_800C2758();
extern void func_80190674();
extern char D_80192510[];
extern char D_80192540[];
extern char D_80192558[];
int func_801905E8(void *o)
{
    int r;

    if (func_800C6CE0(o) >= 2) {
        r = func_800C251C(o, D_80192540);
        r |= func_800C2758(o, D_80192510, D_80192558);
    } else {
        r = -1;
    }
    if (r == -1) {
        func_80190674(o);
    }
    return 0;
}
