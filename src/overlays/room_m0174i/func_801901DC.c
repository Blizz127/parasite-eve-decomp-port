/* room_m0174i (PE.IMG room m0174i chunk 2, VRAM 0x8018EFE8)
 * func_801901DC — blob offset 0x11F4, 0x64 bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the room VMA (docs/evidence/room-lane-2026-09-23/REPORT.md).
 * When func_800C6CE0(o) == 3, clear bit 30 of the ctx word and func_800C2414(o, D_80196DA4). */

#define B(o, x) (*(unsigned char *)((char *)(o) + (x)))
#define H(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))
extern int func_800C6CE0();
extern void func_800C2414();
extern char D_80196DA4[];
int func_801901DC(void *o)
{
    int *w;

    if (func_800C6CE0(o) == 3) {
        w = P(P(o, 0x8), 0x0);
        *w &= 0xBFFFFFFF;
        func_800C2414(o, D_80196DA4);
    }
    return 0;
}
