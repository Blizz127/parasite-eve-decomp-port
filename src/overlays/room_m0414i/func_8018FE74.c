/* room_m0414i (PE.IMG room m0414i chunk 2, VRAM 0x8018EFE8)
 * func_8018FE74 — blob offset 0xe8c, 0x74 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0141i func_8018FEA8; C re-targeted by symbol address
 * (docs/evidence/room_m0414i-ports-2026-09-23/REPORT.md). */

#define B(o, x) (*(unsigned char *)((char *)(o) + (x)))
#define H(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))
extern void *func_800C2B50();
void func_8018FE74(int a0, int a1, void *p)
{
    void *r = func_800C2B50();
    unsigned int i;
    struct { short x, y, z, pad; } *v = p;

    for (i = 0; i < 16; i++) {
        v[i].x = W(r, 0x18);
        v[i].y = W(r, 0x1C);
        v[i].z = W(r, 0x20);
    }
    H(p, 0x82) = 128;
    H(p, 0x80) = 2048;
}
