/* room_m0063i (PE.IMG room m0063i chunk 2, VRAM 0x8018EFE8)
 * func_8018F250 — blob offset 0x268, 0x6c bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0358i func_80192744; C re-targeted by symbol address
 * (docs/evidence/room_m0063i-ports-2026-09-23/REPORT.md). */

#define B(o, x) (*(unsigned char *)((char *)(o) + (x)))
#define SB(o, x) (*(signed char *)((char *)(o) + (x)))
#define H(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))
extern int func_800DFB78();
extern void func_8018FD5C();
int func_8018F250(void *o)
{
    int r = func_800DFB78(o);

    if (r != 1) {
        if (r < 2) {
            if (r != 0) {
                return 0;
            }
            ((void (*)())P(o, 0xC))(o);
            return 0;
        }
    } else {
        func_8018FD5C(o);
    }
    return 0;
}
