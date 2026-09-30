/* room_m0418i (PE.IMG room m0418i chunk 2, VRAM 0x8018EFE8)
 * func_80193B6C — blob offset 0x4b84, 0x70 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0049i func_8018F0BC; C re-targeted by symbol address
 * (docs/evidence/room_m0418i-ports-2026-09-23/REPORT.md). */

extern int func_800C251C();
extern int func_800C2758();
extern void func_80193BDC();
extern char D_801989E4[];
extern char D_80198A1C[];
extern char D_80198A38[];
int func_80193B6C(void *o)
{
    int r;

    r = func_800C251C(o, D_80198A1C) | func_800C2758(o, D_801989E4, D_80198A38);
    if (r == -1) {
        func_80193BDC(o);
    }
    return 0;
}
