/* room_m0358i (PE.IMG room m0358i chunk 2, VRAM 0x8018EFE8)
 * func_80192744 — blob offset 0x375C, 0x6C bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the room VMA (docs/evidence/room-lane-2026-09-23/REPORT.md).
 * func_800DFB78 dispatch (0: o->fn(o); 1: func_80193250). */

#define B(o, x) (*(unsigned char *)((char *)(o) + (x)))
#define SB(o, x) (*(signed char *)((char *)(o) + (x)))
#define H(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))
extern int func_800DFB78();
extern void func_80193250();
int func_80192744(void *o)
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
        func_80193250(o);
    }
    return 0;
}
