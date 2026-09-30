/* room_m0418i (PE.IMG room m0418i chunk 2, VRAM 0x8018EFE8)
 * func_8018F2C8 — blob offset 0x2e0, 0x70 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0049i func_8018F0BC; C re-targeted by symbol address
 * (docs/evidence/room_m0418i-ports-2026-09-23/REPORT.md). */

extern int func_800C251C();
extern int func_800C2758();
extern void func_8018F338();
extern char D_80198674[];
extern char D_80198694[];
extern char D_801986A4[];
int func_8018F2C8(void *o)
{
    int r;

    r = func_800C251C(o, D_80198694) | func_800C2758(o, D_80198674, D_801986A4);
    if (r == -1) {
        func_8018F338(o);
    }
    return 0;
}
