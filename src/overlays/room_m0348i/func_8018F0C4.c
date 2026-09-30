/* room_m0348i (PE.IMG room m0348i chunk 2, VRAM 0x8018EFE8)
 * func_8018F0C4 — blob offset 0xdc, 0x64 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0174i func_801901DC; C re-targeted by symbol address
 * (docs/evidence/room_m0348i-ports-2026-09-23/REPORT.md). */

#define B(o, x) (*(unsigned char *)((char *)(o) + (x)))
#define H(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))
extern int func_800C6CE0();
extern void func_800C2414();
extern char D_80192880[];
int func_8018F0C4(void *o)
{
    int *w;

    if (func_800C6CE0(o) == 3) {
        w = P(P(o, 0x8), 0x0);
        *w &= 0xBFFFFFFF;
        func_800C2414(o, D_80192880);
    }
    return 0;
}
