/* room_m0401i (PE.IMG room m0401i chunk 2, VRAM 0x8018EFE8)
 * func_801902B8 — blob offset 0x12d0, 0x8c bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0065i func_8018F09C; C re-targeted by symbol address
 * (docs/evidence/room_m0401i-ports-2026-09-23/REPORT.md). */

extern unsigned int func_800C6CE0();
extern int func_800C251C();
extern int func_800C2758();
extern void func_80190344();
extern char D_801953A4[];
extern char D_801953D4[];
extern char D_801953EC[];
int func_801902B8(void *o)
{
    int r;

    if (func_800C6CE0(o) >= 2) {
        r = func_800C251C(o, D_801953D4);
        r |= func_800C2758(o, D_801953A4, D_801953EC);
    } else {
        r = -1;
    }
    if (r == -1) {
        func_80190344(o);
    }
    return 0;
}
