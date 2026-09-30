/* room_m0075i — func_80190410, blob offset 0x1428, 0x2C0 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl5 2026-09-27).
 * Beam segment draw; explicit (i << 2) index keeps retail's base-first addu (i*4: 1w). */

typedef struct { short m[3][3]; int t[3]; } MATRIX;
typedef struct { int vx, vy, vz, pad; } VECTOR;
typedef struct { unsigned char pad0[4]; unsigned char b4; unsigned char pad5[3]; short h8; short hA; } OBJ;
extern OBJ D_801940B8;
extern VECTOR D_8018EFFC;
extern unsigned char D_80194008[];
extern unsigned char **func_800C2B50();
extern void func_800C2EAC();
extern void func_800C2FF0();
extern void func_800C3098();
extern void func_800C3238();
extern void func_80078CC4();
extern void func_800C42A4();

#define SH(o, x) (*(short *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))

void func_80190410(int a0, unsigned char *a1, unsigned char *o)
{
    MATRIX m;
    VECTOR s;
    unsigned char **x;
    MATRIX *A;
    MATRIX *B;
    unsigned int i;

    x = func_800C2B50();
    func_800C2EAC(((unsigned char *)x)[0x6C]);
    func_800C2FF0(0x20, 0x20);
    func_800C3098(0x10);
    func_800C3238(2);
    m.m[0][0] = m.m[1][1] = m.m[2][2] = 0x1000;
    m.t[0] = m.t[1] = m.t[2] = 0;
    m.m[0][1] = m.m[0][2] = m.m[1][0] = m.m[1][2] = m.m[2][0] = m.m[2][1] = 0;
    s = D_8018EFFC;
    func_80078CC4(&m, &s);
    if (SH(a1, 2) == 0x1E) {
        *(MATRIX *)(o + 0x20) = *(MATRIX *)((char *)P(*x, 0x238) + 0x1A0);
        *(MATRIX *)o = *(MATRIX *)((char *)P(*x, 0x238) + 0x120);
    }
    if (SH(a1, 2) >= 0x1F) {
        D_801940B8.h8 = 0;
        A = (MATRIX *)o;
        B = (MATRIX *)(o + 0x20);
    } else {
        A = (MATRIX *)((char *)P(*x, 0x238) + 0x120);
        B = (MATRIX *)((char *)P(*x, 0x238) + 0x1A0);
    }
    for (i = 0; i < SH(o, 0x182); i++) {
        D_801940B8.hA = SH(o, 0x180);
        D_801940B8.b4 = D_80194008[SH(o + (i << 2), 0x160)];
        m.t[0] = A->t[0] + SH(o + i * 16, 0x40);
        m.t[1] = A->t[1] + SH(o + i * 16, 0x42);
        m.t[2] = A->t[2] + SH(o + i * 16, 0x44);
        func_800C42A4(&D_801940B8, &m, 1);
        m.t[0] = B->t[0] + SH(o + i * 16, 0x48);
        m.t[1] = B->t[1] + SH(o + i * 16, 0x4A);
        m.t[2] = B->t[2] + SH(o + i * 16, 0x4C);
        func_800C42A4(&D_801940B8, &m, 1);
    }
}
