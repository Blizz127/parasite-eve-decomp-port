/* room_m0186i (PE.IMG room m0186i chunk 2, VRAM 0x8018EFE8)
 * func_8018F2AC — blob offset 0x2c4, 0x8c bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0065i func_8018F09C; C re-targeted by symbol address
 * (docs/evidence/room_m0186i-ports-2026-09-23/REPORT.md). */

extern unsigned int func_800C6CE0();
extern int func_800C251C();
extern int func_800C2758();
extern void func_8018F338();
extern char D_80194138[];
extern char D_80194150[];
extern char D_8019415C[];
int func_8018F2AC(void *o)
{
    int r;

    if (func_800C6CE0(o) >= 2) {
        r = func_800C251C(o, D_80194150);
        r |= func_800C2758(o, D_80194138, D_8019415C);
    } else {
        r = -1;
    }
    if (r == -1) {
        func_8018F338(o);
    }
    return 0;
}
