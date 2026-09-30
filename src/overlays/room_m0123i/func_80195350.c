/* room_m0123i (PE.IMG room m0123i chunk 2, VRAM 0x8018EFE8)
 * func_80195350 — blob offset 0x6368, 0x2c bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0005i func_80190A6C; C re-targeted by symbol address
 * (docs/evidence/room_m0123i-ports-2026-09-23/REPORT.md). */

#define B(o, x) (*(unsigned char *)((char *)(o) + (x)))
#define H(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))
extern void *D_800E2368;
extern char D_80195684[];
char *func_80195350(int a0)
{
    if (a0 == 1) {
        H(D_800E2368, 0x12) = 1;
    }
    return D_80195684;
}
