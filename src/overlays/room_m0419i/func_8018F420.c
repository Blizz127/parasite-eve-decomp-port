/* room_m0419i (PE.IMG room m0419i chunk 2, VRAM 0x8018EFE8)
 * func_8018F420 — blob offset 0x438, 0x338 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0075i func_8018F3DC; C re-targeted by symbol address
 * (docs/evidence/room_m0419i-ports-2026-09-23/REPORT.md). */

typedef struct { short m[3][3]; int t[3]; } MATRIX;
typedef struct {
    unsigned char b0, b1, b2, b3;
    short h4, h6, h8, hA, hC, hE;
    unsigned char b10, b11, b12, b13;
    unsigned char pad[0x30];
} SPR;
extern unsigned char D_80194E08, D_80194E09, D_80194E0A, D_80194E0C, D_80194E0D, D_80194E0E, D_80195368, D_80195369, D_8019536A, D_8019536C, D_8019536D, D_8019536E, D_80195378, D_80195379, D_8019537A, D_8019537C, D_8019537D, D_8019537E, D_80195388, D_80195389, D_8019538A, D_8019538C, D_8019538D, D_8019538E, D_80195398, D_80195399, D_8019539A, D_8019539C, D_8019539D, D_8019539E;
extern short D_80194E10, D_80194E12, D_80195370, D_80195372, D_80195380, D_80195382, D_80195390, D_80195392, D_801953A0, D_801953A2;
extern SPR D_80194E18[10];
extern SPR D_801950C0[10];
extern void func_800C2B40();
extern int func_8006DC18();

#define H(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))

void func_8018F420(void *a0, int a1, unsigned char *o)
{
    void *x;
    unsigned int i;
    SPR *p;

    func_800C2B40(o);
    W(o, 0x6C) = func_8006DC18(0xC);
    x = P(a0, 8);
    P(o, 0) = x;
    *(MATRIX *)(o + 4) = *(MATRIX *)P(x, 0x238);
    *(MATRIX *)(o + 0x44) = *(MATRIX *)((char *)P(P(o, 0), 0x238) + 0x120);
    *(MATRIX *)(o + 0x24) = *(MATRIX *)((char *)P(P(o, 0), 0x238) + 0x1A0);
    H(o, 0x64) = 0x1E;
    H(o, 0x66) = 0;
    H(o, 0x68) = 0;
    D_8019537C = 0x2c;
    D_8019537D = 0x1;
    D_80195380 = -0x1e;
    D_80195382 = 0x0;
    D_80195378 = 0x80;
    D_80195379 = 0x80;
    D_8019537A = 0x80;
    D_8019537E = 0x0;
    D_8019538C = 0x2c;
    D_8019538D = 0x1;
    D_80195390 = 0x0;
    D_80195392 = 0x20;
    D_80195388 = 0x80;
    D_80195389 = 0x80;
    D_8019538A = 0x80;
    D_8019538E = 0x0;
    D_8019536C = 0x2c;
    D_8019536D = 0x1;
    D_80195370 = -0x1e;
    D_80195372 = 0x80;
    D_80194E10 = -0x1f;
    D_8019539C = 0x20;
    D_80195368 = 0x80;
    D_80195369 = 0x80;
    D_8019536A = 0x80;
    D_8019536E = 0x0;
    D_80194E0C = 0x0;
    D_80194E0D = 0x1;
    D_80194E12 = 0x80;
    D_80194E08 = 0x80;
    D_80194E09 = 0x80;
    D_80194E0A = 0x80;
    D_80194E0E = 0x0;
    D_8019539D = 0x1;
    D_801953A0 = 0x64;
    D_801953A2 = 0x80;
    D_80195398 = 0x80;
    D_80195399 = 0x80;
    D_8019539A = 0x80;
    D_8019539E = 0x0;
    for (i = 0; i < 10; i++) {
        p = &D_80194E18[i];
        p->b1 = 4;
        p->h4 = 0x80;
        p->h6 = 4;
        p->h8 = 0x1E;
        p->hA = 0;
        if (i == 0) {
            p->hC = -0x40;
        } else {
            p->hC = 0;
        }
        p->b10 = 0xFF;
        p->b11 = 0xFF;
        p->b12 = 0xFF;
        p = &D_801950C0[i];
        p->b1 = 4;
        p->h4 = 0x80;
        p->h6 = 4;
        p->h8 = 0x1E;
        p->hA = 0;
        if (i == 0) {
            p->hC = 0x40;
        } else {
            p->hC = 0;
        }
        p->b10 = 0xFF;
        p->b11 = 0xFF;
        p->b12 = 0xFF;
    }
}
