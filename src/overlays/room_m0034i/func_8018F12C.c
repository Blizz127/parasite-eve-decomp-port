/* room_m0034i (PE.IMG room m0034i chunk 2, VRAM 0x8018EFE8)
 * func_8018F12C — blob offset 0x144, 0x8c bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0186i func_8018FF30; C re-targeted by symbol address
 * (docs/evidence/room_m0034i-ports-2026-09-23/REPORT.md). */

extern int func_800C6CE0();
extern int func_800C251C();
extern int func_800C2758();
extern void func_8018F1B8();
extern char D_8018FF34[];
extern char D_8018FF5C[];
extern char D_8018FF70[];
int func_8018F12C(void *o)
{
    int r;

    if (func_800C6CE0(o) == 3) {
        r = func_800C251C(o, D_8018FF5C);
        r |= func_800C2758(o, D_8018FF34, D_8018FF70);
    } else {
        r = -1;
    }
    if (r == -1) {
        func_8018F1B8(o);
    }
    return 0;
}
