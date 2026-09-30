/* room_m0418i (PE.IMG room m0418i chunk 2, VRAM 0x8018EFE8)
 * func_8018FF64 — blob offset 0xf7c, 0x70 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0049i func_8018F0BC; C re-targeted by symbol address
 * (docs/evidence/room_m0418i-ports-2026-09-23/REPORT.md). */

extern int func_800C251C();
extern int func_800C2758();
extern void func_8018FFD4();
extern char D_80198718[];
extern char D_80198740[];
extern char D_80198754[];
int func_8018FF64(void *o)
{
    int r;

    r = func_800C251C(o, D_80198740) | func_800C2758(o, D_80198718, D_80198754);
    if (r == -1) {
        func_8018FFD4(o);
    }
    return 0;
}
