/* room_m0421i (PE.IMG room m0421i chunk 2, VRAM 0x8018EFE8)
 * func_8018FFF4 — blob offset 0x100c, 0xf4 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0107i func_8018F9D8; C re-targeted by symbol address
 * (docs/evidence/room_m0421i-ports-2026-09-23/REPORT.md). */

#define B(o, x) (*(unsigned char *)((char *)(o) + (x)))
#define H(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))
typedef struct { short x, y, z, pad; } SVECTOR;
extern void *func_800C2B50();
void func_8018FFF4(void *o, void *q, void *p)
{
    void *r = func_800C2B50();
    SVECTOR v;
    short t;
    int a;

    a = H(P(o, 0x8), 0x2A);
    v.x = a;
    a = H(P(o, 0x8), 0x32);
    v.z = a;
    if (H(q, 0x2) < 32) {
        H(p, 0x138) += H(p, 0x13C);
        H(p, 0x13C) += 7;
    }
    if (H(q, 0x2) < 32) {
        t = H(p, 0x13A);
        if (t > 4) {
            H(p, 0x13A) = t - 4;
        }
    }
    if (H(q, 0x2) == 16) {
        H(r, 0x2C) = 1;
    }
    if (H(q, 0x2) == 32) {
        B(q, 0x1) = 2;
    }
}
