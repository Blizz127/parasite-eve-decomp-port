/* room_m0419i (PE.IMG room m0419i chunk 2, VRAM 0x8018EFE8)
 * func_8018F2C8 — blob offset 0x2e0, 0xc4 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0075i func_8018F284; C re-targeted by symbol address
 * (docs/evidence/room_m0419i-ports-2026-09-23/REPORT.md). */

#define B(o, x) (*(unsigned char *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))
extern unsigned int func_800C6CE0();
extern int func_800C251C();
extern int func_800C2758();
extern void func_8018F38C();
extern char D_80194C08[];
extern char D_80194C38[];
extern char D_80194C50[];
int func_8018F2C8(void *o)
{
    int r = 0;
    void *c;

    if (func_800C6CE0(o) >= 2) {
        c = P(o, 0x8);
        if (B(c, 0xE) >= 2 || B(P(c, 0x0), 0xAC) != 0) {
            r = func_800C251C(o, D_80194C38);
            r |= func_800C2758(o, D_80194C08, D_80194C50);
        }
    } else {
        r = -1;
    }
    if (r == -1) {
        func_8018F38C(o);
    }
    return 0;
}
