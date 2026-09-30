/* room_m0418i — func_80192544, blob offset 0x355C, 0x1C0 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl7 2026-09-27).
 * Two-stage record draw (RotMatrix+ScaleMatrix, D_801995F8 SPR records via func_800C4FC4); second stage after a1+2 >= 0x3D. */

typedef struct { short m[3][3]; int t[3]; } MATRIX;
typedef struct { int vx, vy, vz, pad; } VECTOR;
typedef struct { unsigned char *p; unsigned char a[4]; unsigned char b[4]; short h[5]; } SPR;
extern SPR D_801995F8[3];
extern int D_8019956C;
extern int D_8019957C;
extern void func_800C3238(), func_80079754(), func_80071A44(), func_80078CC4(), func_800C4FC4();

void func_80192544(int a0, char *a1, char *a2)
{
    MATRIX m;
    VECTOR s;
    VECTOR v;
    VECTOR w;

    func_800C3238(3);
    *(short *)(a2 + 8) = 0;
    *(short *)(a2 + 0xA) = 0;
    *(short *)(a2 + 0xC) = 0;
    func_80079754(a2 + 8, &m);
    func_80071A44(&v, 0, 0x10);
    v.vx = *(short *)(a2 + 0x10);
    v.vy = *(short *)(a2 + 0x10);
    v.vz = *(short *)(a2 + 0x10);
    s = v;
    func_80078CC4(&m, &s);
    m.t[0] = D_8019956C;
    m.t[1] = 0;
    m.t[2] = D_8019957C;
    func_800C4FC4(&D_801995F8[0], &m, 1);
    if (*(short *)(a1 + 2) >= 0x3D) {
        func_800C3238(2);
        *(short *)(a2 + 8) = 0x400;
        *(short *)(a2 + 0xA) = 0x400;
        *(short *)(a2 + 0xC) = 0;
        func_80079754(a2 + 8, &m);
        func_80071A44(&w, 0, 0x10);
        w.vx = *(short *)(a2 + 0x12);
        w.vy = *(short *)(a2 + 0x12);
        w.vz = *(short *)(a2 + 0x12);
        v = w;
        func_80078CC4(&m, &v);
        m.t[0] = D_8019956C;
        m.t[1] = 0;
        m.t[2] = D_8019957C;
        func_800C4FC4(&D_801995F8[1], &m, 0);
    }
}
