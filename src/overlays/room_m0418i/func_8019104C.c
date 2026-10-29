/* room_m0418i — func_8019104C, blob offset 0x2064, 0x2FC bytes. Flags -O2 -G0.
 * Sprite/model pass: when p->bC < 12 draw D_801995A8 (frame byte from bC % 12),
 * then always draw D_801995B8, then a uniformly scaled (p->h8) matrix for the
 * two D_801995C8[0]/[1] draws via func_800C4FC4. */

typedef struct { short m[3][3]; int t[3]; } MATRIX;
typedef struct { int vx, vy, vz, pad; } VECTOR;
typedef struct { unsigned char b[8]; } CV;
typedef struct { short h0, h2, h4, h6, h8; unsigned short hA; unsigned char bC; } P;
typedef struct { unsigned char pad0[4]; unsigned char b4; unsigned char b5; unsigned char pad1[4]; short hA; } OBJ;
typedef struct { unsigned char pad0[0x14]; short h14; short pad16; } OBJ2;
extern CV D_8018F018;
extern VECTOR D_8018F020, D_8018F030;
extern OBJ D_801995A8, D_801995B8;
extern OBJ2 D_801995C8[2];
extern unsigned char *func_800C2B50();
extern void func_800C2EAC(), func_800C3098(), func_800C3238(), func_800C2FF0();
extern void func_80078CC4(), func_800C42A4(), func_800794C4(), func_80071A44();
extern void func_800C4FC4();

void func_8019104C(int a0, int a1, P *p)
{
    P *q = p;
    unsigned char *r = func_800C2B50();
    CV cv;
    MATRIX m;
    VECTOR sa;
    VECTOR sb;
    VECTOR s;

    cv = D_8018F018;
    func_800C2EAC(r[0x24]);
    func_800C3098(0x10);
    func_800C3238(2);
    func_800C2FF0(0x20, 0x20);
    if (p->bC < 12) {
        m.m[0][0] = m.m[1][1] = m.m[2][2] = 0x1000;
        m.t[0] = m.t[1] = m.t[2] = 0;
        m.m[0][1] = m.m[0][2] = m.m[1][0] = m.m[1][2] = m.m[2][0] = m.m[2][1] = 0;
        sa = D_8018F020;
        func_80078CC4(&m, &sa);
        m.t[0] = p->h0;
        m.t[1] = p->h2;
        m.t[2] = p->h4;
        D_801995A8.hA = p->hA;
        D_801995A8.b4 = (p->bC % 12 & 0xFE) - 0x5C;
        func_800C42A4(&D_801995A8, &m, 1);
    }
    m.m[0][0] = m.m[1][1] = m.m[2][2] = 0x1000;
    m.t[0] = m.t[1] = m.t[2] = 0;
    m.m[0][1] = m.m[0][2] = m.m[1][0] = m.m[1][2] = m.m[2][0] = m.m[2][1] = 0;
    sa = D_8018F030;
    func_80078CC4(&m, &sa);
    m.t[0] = q->h0;
    m.t[1] = q->h2;
    m.t[2] = q->h4;
    D_801995B8.hA = (short)q->hA >> 2;
    func_800C42A4(&D_801995B8, &m, 1);
    func_800794C4(&cv, &m);
    func_80071A44(&s, 0, 16);
    s.vx = q->h8;
    s.vy = q->h8;
    s.vz = q->h8;
    sb = s;
    func_80078CC4(&m, &sb);
    m.t[0] = q->h0;
    m.t[1] = q->h2;
    m.t[2] = q->h4;
    D_801995C8[0].h14 = q->hA;
    func_800C4FC4(&D_801995C8[0], &m, 0);
    D_801995C8[1].h14 = q->hA;
    func_800C4FC4(&D_801995C8[1], &m, 0);
}
