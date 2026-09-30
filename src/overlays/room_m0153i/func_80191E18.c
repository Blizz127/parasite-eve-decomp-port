/* room_m0153i (PE.IMG room m0153i chunk 2, VRAM 0x8018EFE8)
 * func_80191E18 — blob offset 0x2e30, 0x84 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0022i func_801908FC; C re-targeted by symbol address
 * (docs/evidence/room_m0153i-ports-2026-09-23/REPORT.md). */

typedef struct Src {
    unsigned char pad[0x28];
    int f28;
    int pad2C;
    int f30;
} Src;
typedef struct Q {
    unsigned char pad[0x50];
    int f50;
    int pad54;
    int f58;
    unsigned char pad5C[8];
    Src *f64;
    unsigned char pad68[6];
    short f6E;
} Q;
typedef struct P {
    unsigned char pad[0x28];
    int f28[4];
    unsigned char pad38[2];
    short f3A;
} P;
extern int func_800DFF80();
extern short func_800DFFB8();
void func_80191E18(P *p, Q *q)
{
    Src *s;

    if (q->f6E > 0) {
        s = q->f64;
        if (s != 0) {
            q->f50 = s->f28;
            q->f58 = s->f30;
        }
        p->f3A = func_800DFFB8(p->f3A, (short)func_800DFF80(&q->f50, p->f28), q->f6E);
    }
}
