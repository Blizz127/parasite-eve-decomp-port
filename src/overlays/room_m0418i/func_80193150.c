/* room_m0418i — func_80193150, blob offset 0x4168, 0x2B0 bytes. Flags -O2 -G0.
 * Model draw: build the object matrix from p, set the TPage/CLUT by mode word
 * r+0x24, draw D_80199528, then (when a1->[1] > 10) an extra scaled draw of
 * D_801996B0 with a clamped (n-10)<<9 scale. */

typedef struct { short m[3][3]; int t[3]; } MATRIX;
typedef struct { int vx, vy, vz, pad; } VECTOR;
typedef struct { unsigned char b[8]; } CV;
extern void *D_80199528;
extern int D_8019956C, D_8019957C;
extern short D_800942EC;
extern unsigned char D_80198860[], D_801996B0[];
extern char *func_800C2B50();
extern void func_800794C4(), func_80071A44(), func_80078CC4(), func_800C6D5C();
extern int func_80077A64(), func_80077AA4();
extern void func_800C6EC0(unsigned short, unsigned short);
extern void func_800C3134(), func_800C6ED8(), func_800C6EF8(), func_800C6FA0();
extern void func_800C71E4(), func_800C6F4C();
extern void func_800C2EAC(), func_800C2FF0(), func_800C3098(), func_800C3238();
extern void func_800C42A4();

void func_80193150(int a0, short *a1, short *p)
{
    short *q = p;
    char *r = func_800C2B50();
    MATRIX m;
    CV cv;
    VECTOR s2;
    VECTOR s;
    int n;
    int v;

    func_800794C4(p + 4, &m);
    func_80071A44(&s, 0, 16);
    s.vx = p[8];
    s.vy = p[9];
    s.vz = p[10];
    s2 = s;
    func_80078CC4(&m, &s2);
    m.t[0] = D_8019956C;
    m.t[1] = D_800942EC + p[1];
    m.t[2] = D_8019957C;
    func_800C6D5C(D_80199528, 0, 0);
    if (*(int *)(r + 0x24) == 0) {
        func_800C6EC0(func_80077A64(0, 1, 0x340, 0x100), func_80077AA4(0, 0x1D7));
    }
    if (*(int *)(r + 0x24) == 1) {
        func_800C6EC0(func_80077A64(0, 1, 0x340, 0x160), func_80077AA4(0, 0x1DB));
    }
    func_800C3134(D_80198860, a1[1], &cv);
    func_800C6ED8(1);
    func_800C6EF8(D_80199528);
    func_800C6FA0(D_80199528, (unsigned short)q[12]);
    func_800C71E4(D_80199528, &m);
    func_800C6F4C(D_80199528);
    n = a1[1];
    if (n > 10) {
        v = (n - 10) << 9;
        if (v > 0x7D00) {
            v = 0x7D00;
        }
        func_800C2EAC(3);
        func_800C2FF0(0x20, 0x20);
        func_800C3098(0x100);
        func_800C3238(2);
        m.m[0][0] = m.m[1][1] = m.m[2][2] = 0x1000;
        m.t[0] = m.t[1] = m.t[2] = 0;
        m.m[0][1] = m.m[0][2] = m.m[1][0] = m.m[1][2] = m.m[2][0] = m.m[2][1] = 0;
        func_80071A44(&s, 0, 16);
        s.vx = v;
        s.vy = v;
        s.vz = v;
        func_80078CC4(&m, &s);
        m.t[1] = 0;
        m.t[0] = D_8019956C;
        m.t[2] = D_8019957C;
        func_800C42A4(D_801996B0, &m, 1);
    }
}
