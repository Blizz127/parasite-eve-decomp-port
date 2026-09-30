/* room_m0022i (PE.IMG room m0022i chunk 2, VRAM 0x8018EFE8)
 * func_80192138 — blob offset 0x3150, 0x98 bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the room VMA (docs/evidence/room_m0022i-batch-2026-09-23/REPORT.md). */

#define B(o, x) (*(unsigned char *)((char *)(o) + (x)))
#define SB(o, x) (*(signed char *)((char *)(o) + (x)))
#define H(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))
extern void func_80192200();
int func_80192138(void *o, int a1, unsigned int a2, int a3, int a4, int a5)
{
    switch (a2) {
    case 25:
        if (a1 == 1) {
            W(o, 0x10) = a3;
            *(int *)a3 = a1;
        }
        break;
    case 4:
        W(o, 0x3C) = a3;
        W(o, 0x40) = a4;
        H(o, 0x44) = a5;
        break;
    case 28:
        H(o, 0x48) = a3;
        H(o, 0x46) = a4;
        break;
    case 10:
        SB(o, 0x16) = a3;
        P(o, 0xC) = func_80192200;
        break;
    }
    return 0;
}
