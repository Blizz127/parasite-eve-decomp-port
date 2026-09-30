/* room_m0418i — func_80194090, blob offset 0x50A8, 0x184 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl7 2026-09-27).
 * 3-part model draw (Rot/ScaleMatrix per part, D_80199550 object) + a final D_80199580 draw; lever: a2 + (i*8 + 0x30) keeps retail's separate int offset biv. */

typedef struct { short m[3][3]; int t[3]; } MATRIX;
typedef struct { int vx, vy, vz, pad; } VECTOR;
typedef struct { char pad0[0xA]; short hA; } OBJ;
extern OBJ D_80199550;
extern char D_80199580[];
extern short D_800942EC;
extern char *func_800C2B50();
extern void func_800C2EAC(), func_800C2FF0(), func_800C3098(), func_800C3238();
extern void func_800794C4(), func_80071A44(), func_80078CC4(), func_800C42A4();

#define H(off) *(short *)(a2 + i * 8 + (off))
#define W(off) *(int *)(a2 + i * 16 + (off))

void func_80194090(int a0, int a1, char *a2)
{
    MATRIX m;
    VECTOR s;
    VECTOR v;
    char *r;
    unsigned int i;

    r = func_800C2B50();
    func_800C2EAC(r[0x24]);
    func_800C2FF0(0x40, 0x20);
    func_800C3098(0x10);
    func_800C3238(2);
    for (i = 0; i < 3; i++) {
        func_800794C4(a2 + (i * 8 + 0x30), &m);
        func_80071A44(&v, 0, 0x10);
        v.vx = H(0x48);
        v.vy = H(0x4A);
        v.vz = H(0x4C);
        s = v;
        func_80078CC4(&m, &s);
        m.t[0] = W(0);
        m.t[1] = W(4);
        m.t[2] = W(8);
        D_80199550.hA = 0x80 - (i << 5);
        if (i == 0) {
            D_80199550.hA = 0xC0;
        }
        func_800C42A4(&D_80199550, &m, 0);
    }
    m.t[0] = *(int *)(a2 + 0);
    m.t[1] = D_800942EC;
    m.t[2] = *(int *)(a2 + 8);
    func_800C42A4(D_80199580, &m, 0);
}
