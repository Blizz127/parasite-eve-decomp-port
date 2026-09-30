/* room_m0414i (PE.IMG room m0414i chunk 2, VRAM 0x8018EFE8)
 * func_8018F840 — blob offset 0x858, 0xdc bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0141i func_8018F874; C re-targeted by symbol address
 * (docs/evidence/room_m0414i-ports-2026-09-23/REPORT.md). */

#define B(o, x) (*(unsigned char *)((char *)(o) + (x)))
#define H(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))
typedef struct { short x, y, z, pad; } SVECTOR;
extern void *func_800C2B50();
extern int func_800C6B90();
extern short D_800942EC;
void func_8018F840(int a0, void *q, void *p)
{
    void *r = func_800C2B50();
    SVECTOR v;
    short t;

    if (H(q, 0x2) >= 9) {
        t = H(p, 0x122);
        if (t > 32) {
            H(p, 0x122) = t - 32;
        }
    }
    H(p, 0x120) += H(p, 0x124) >> 4;
    H(p, 0x124) += 800;
    if (H(q, 0x2) == 16) {
        B(q, 0x1) = 2;
    }
    v.x = W(r, 0x18);
    v.y = D_800942EC;
    v.z = W(r, 0x20);
    if (func_800C6B90(&v, H(r, 0x2E))) {
        H(r, 0x2C) = 1;
    }
}
