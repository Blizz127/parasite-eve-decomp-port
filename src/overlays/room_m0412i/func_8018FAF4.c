/* room_m0412i (PE.IMG room m0412i chunk 2, VRAM 0x8018EFE8)
 * func_8018FAF4 — blob offset 0xb0c, 0x54 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0141i func_8018F950; C re-targeted by symbol address
 * (docs/evidence/room_m0412i-ports-2026-09-23/REPORT.md). */

#define B(o, x) (*(unsigned char *)((char *)(o) + (x)))
#define H(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))
extern void *func_800C2B50();
void func_8018FAF4(int a0, int a1, void *p)
{
    void *r = func_800C2B50();

    H(p, 0x0) = W(r, 0x18);
    H(p, 0x2) = W(r, 0x1C);
    H(p, 0x4) = W(r, 0x20);
    H(p, 0xA) = 0;
    H(p, 0x8) = 1024;
}
