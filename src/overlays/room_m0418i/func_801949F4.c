/* room_m0418i — func_801949F4, blob offset 0x5A0C, 0x150 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl7 2026-09-27).
 * Model draw setup: RotMatrix/ScaleMatrix on a local MATRIX, sprite object D_80199570 register + draw; lever: p = a2 copy made before the first a2 read keeps the two callee-saved homes. */

typedef struct { short m[3][3]; int t[3]; } MATRIX;
typedef struct { int vx, vy, vz, pad; } VECTOR;
typedef struct { char pad0[4]; unsigned char b4; char pad5[5]; short hA; } OBJ;
extern OBJ D_80199570;
extern char D_80198AD4[];
extern char *func_800C2B50();
extern void func_800C2EAC(), func_800C2FF0(), func_800C3098(), func_800C3238();
extern void func_800794C4(), func_80071A44(), func_80078CC4(), func_800C3134(), func_800C42A4();

void func_801949F4(int a0, char *a1, char *a2)
{
    MATRIX m;
    VECTOR s;
    VECTOR v;
    char *r;
    unsigned int n;
    char *p;
    OBJ *o;

    p = a2;
    r = func_800C2B50();
    n = ((unsigned char)a2[0x22] >> 4) << 2;
    if (n > 12) {
        n = 12;
    }
    func_800C2EAC(r[0x24]);
    func_800C2FF0(0x40, 0x40);
    func_800C3098(0x10);
    func_800C3238(2);
    func_800794C4(p + 0x10, &m);
    func_80071A44(&v, 0, 0x10);
    v.vx = *(short *)(p + 0x1C);
    v.vy = *(short *)(p + 0x1C);
    v.vz = 0x1000;
    s = v;
    func_80078CC4(&m, &s);
    o = &D_80199570;
    func_800C3134(D_80198AD4, *(short *)(a1 + 2), o);
    m.t[0] = *(int *)(p + 0);
    m.t[1] = *(int *)(p + 4);
    m.t[2] = *(int *)(p + 8);
    D_80199570.b4 = n;
    D_80199570.hA = *(short *)(p + 0x20);
    func_800C42A4(o, &m, 1);
}
