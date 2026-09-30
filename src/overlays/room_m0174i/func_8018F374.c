/* room_m0174i (PE.IMG room m0174i chunk 2, VRAM 0x8018EFE8)
 * func_8018F374 — blob offset 0x38c, 0x8c bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0034i func_8018F12C; C re-targeted by symbol address
 * (docs/evidence/room_m0174i-ports-2026-09-23/REPORT.md). */

extern int func_800C6CE0();
extern int func_800C251C();
extern int func_800C2758();
extern void func_8018F400();
extern char D_80196C7C[];
extern char D_80196CA4[];
extern char D_80196CB8[];
int func_8018F374(void *o)
{
    int r;

    if (func_800C6CE0(o) == 3) {
        r = func_800C251C(o, D_80196CA4);
        r |= func_800C2758(o, D_80196C7C, D_80196CB8);
    } else {
        r = -1;
    }
    if (r == -1) {
        func_8018F400(o);
    }
    return 0;
}
