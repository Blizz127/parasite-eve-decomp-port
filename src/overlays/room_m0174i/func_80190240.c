/* room_m0174i (PE.IMG room m0174i chunk 2, VRAM 0x8018EFE8)
 * func_80190240 — blob offset 0x1258, 0x8c bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0034i func_8018F12C; C re-targeted by symbol address
 * (docs/evidence/room_m0174i-ports-2026-09-23/REPORT.md). */

extern int func_800C6CE0();
extern int func_800C251C();
extern int func_800C2758();
extern void func_801902CC();
extern char D_80196D6C[];
extern char D_80196DDC[];
extern char D_80196E14[];
int func_80190240(void *o)
{
    int r;

    if (func_800C6CE0(o) == 3) {
        r = func_800C251C(o, D_80196DDC);
        r |= func_800C2758(o, D_80196D6C, D_80196E14);
    } else {
        r = -1;
    }
    if (r == -1) {
        func_801902CC(o);
    }
    return 0;
}
