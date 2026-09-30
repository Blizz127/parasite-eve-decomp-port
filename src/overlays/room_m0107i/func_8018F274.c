/* room_m0107i (PE.IMG room m0107i chunk 2, VRAM 0x8018EFE8)
 * func_8018F274 — blob offset 0x28c, 0x8c bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0065i func_8018F09C; C re-targeted by symbol address
 * (docs/evidence/room_m0107i-ports-2026-09-23/REPORT.md). */

extern unsigned int func_800C6CE0();
extern int func_800C251C();
extern int func_800C2758();
extern void func_8018F300();
extern char D_80193588[];
extern char D_801935B8[];
extern char D_801935D0[];
int func_8018F274(void *o)
{
    int r;

    if (func_800C6CE0(o) >= 2) {
        r = func_800C251C(o, D_801935B8);
        r |= func_800C2758(o, D_80193588, D_801935D0);
    } else {
        r = -1;
    }
    if (r == -1) {
        func_8018F300(o);
    }
    return 0;
}
