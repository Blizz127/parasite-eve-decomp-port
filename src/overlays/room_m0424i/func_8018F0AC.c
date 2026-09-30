/* room_m0424i (PE.IMG room m0424i chunk 2, VRAM 0x8018EFE8)
 * func_8018F0AC — blob offset 0xc4, 0x8c bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0065i func_8018F09C; C re-targeted by symbol address
 * (docs/evidence/room_m0424i-ports-2026-09-23/REPORT.md). */

extern unsigned int func_800C6CE0();
extern int func_800C251C();
extern int func_800C2758();
extern void func_8018F138();
extern char D_80190D18[];
extern char D_80190D48[];
extern char D_80190D60[];
int func_8018F0AC(void *o)
{
    int r;

    if (func_800C6CE0(o) >= 2) {
        r = func_800C251C(o, D_80190D48);
        r |= func_800C2758(o, D_80190D18, D_80190D60);
    } else {
        r = -1;
    }
    if (r == -1) {
        func_8018F138(o);
    }
    return 0;
}
