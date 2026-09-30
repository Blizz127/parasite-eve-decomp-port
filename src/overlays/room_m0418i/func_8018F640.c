/* room_m0418i — func_8018F640, blob offset 0x658, 0x270 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl7 2026-09-27).
 * 4-part rotated draw of D_80199670 + 2 SPR record draws; levers: loop call arg as hp - 0xA (o dies before the loop -> retail s0/s5 reuse), i = 0 hoisted before o. */

typedef struct { short m[3][3]; int t[3]; } MATRIX;
typedef struct { int vx, vy, vz, pad; } VECTOR;
typedef struct { short x, y, z, w; } SV8;
typedef struct { char pad0[0xA]; short hA; } OBJ;
extern OBJ D_80199670;
extern SV8 D_8018EFF4;
extern unsigned char D_801986F8[], D_80198708[];
extern char *func_800C2B50();
extern void func_800C2EAC(), func_800C2FF0(), func_800C3098(), func_800C3238();
extern void func_800794C4(), func_80071A44(), func_80078CC4(), func_800C3134(), func_800C42A4(), func_800C4FC4();

#define H(off) *(short *)(a2 + i * 2 + (off))

void func_8018F640(int a0, char *a1, char *a2)
{
    MATRIX m;
    SV8 rot;
    VECTOR s;
    VECTOR v;
    char *q;
    OBJ *o;
    short *hp;
    unsigned int i;

    q = func_800C2B50() + 4;
    rot = D_8018EFF4;
    i = 0;
    o = &D_80199670;
    hp = &o->hA;
    func_800C2EAC(q[0x20]);
    func_800C2FF0(0x20, 0x20);
    func_800C3098(0x10);
    func_800C3238(2);
    func_800C3134(D_801986F8, *(short *)(a1 + 2), o);
    for (; i < 4; i++) {
        rot.z = H(0x25C);
        func_800794C4(&rot, &m);
        func_80071A44(&v, 0, 0x10);
        v.vx = H(0x254);
        v.vy = H(0x254);
        v.vz = H(0x254);
        s = v;
        func_80078CC4(&m, &s);
        m.t[0] = *(int *)(q + 0x14);
        m.t[1] = *(int *)(q + 0x18);
        m.t[2] = *(int *)(q + 0x1C);
        *hp = *(short *)(a2 + 0x264);
        func_800C42A4((OBJ *)((char *)hp - 0xA), &m, 1);
    }
    for (i = 0; i < 2; i++) {
        m.m[2][2] = 0x1000;
        m.m[1][1] = 0x1000;
        m.m[0][0] = 0x1000;
        m.t[2] = 0;
        m.t[1] = 0;
        m.t[0] = 0;
        m.m[2][1] = 0;
        m.m[2][0] = 0;
        m.m[1][2] = 0;
        m.m[1][0] = 0;
        m.m[0][2] = 0;
        m.m[0][1] = 0;
        func_80071A44(&v, 0, 0x10);
        v.vx = H(0x230);
        v.vy = H(0x230);
        v.vz = H(0x230);
        s = v;
        func_80078CC4(&m, &s);
        m.t[0] = *(int *)(q + 0x14);
        m.t[1] = *(int *)(q + 0x18);
        m.t[2] = *(int *)(q + 0x1C);
        func_800C3134(D_80198708, *(short *)(a1 + 2), a2 + i * 0x18 + 8);
        func_800C4FC4(a2 + i * 0x18, &m, 1);
    }
}
