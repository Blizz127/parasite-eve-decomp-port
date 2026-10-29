/* room_m0418i — func_80190914, blob offset 0x192C, 0x284 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (focus lane f3 2026-10-01).
 * Two stretched sprites via func_800D3114 then 8 func_800C42A4 draws at +0x28 position triples
 * (gated on byte +0xA9). Sibling of func_801919B0.
 * Levers: locals declared o, m, v (frame 0x38/0x40/0x60); D_8018F004 copy after the
 * func_800C2B50 call; &o passed twice (CSE'd into $fp); identity/zero matrix fills as
 * assignment chains (reverse store order); loop indexes a2 + i * 8 + 0x28.. rather than
 * walking a pointer, so the giv is a2 itself and keeps the 0x28/0x2A/0x2C displacements. */

typedef struct { unsigned char b[4]; } CV;
typedef struct { short m[3][3]; short pad; int t[3]; } MAT;
typedef struct { int x, y, z, w; } VEC;
extern CV D_8018F004;
extern VEC D_8018F008;
extern unsigned char D_801987E4[];
extern unsigned char D_80199690[];
extern char *func_800C2B50();
extern void func_800C2EAC(), func_800C3098(), func_800C3238(), func_800C2FF0();
extern void func_800C3134(), func_800D3114(), func_800C42A4(), func_80078CC4();
extern unsigned short func_80077A64();
extern unsigned short func_80077AA4();

void func_80190914(int a0, int a1, char *a2)
{
    CV o;
    MAT m;
    VEC v;
    char *r;
    unsigned int i;
    char *q1;
    char *q2;

    r = func_800C2B50();
    o = D_8018F004;
    func_800C2EAC(r[0x24]);
    func_800C3098(0x10);
    func_800C3238(2);
    func_800C2FF0(0x10, 0x10);
    if (a2[0xA9]) {
        q1 = a2 + 0x98;
        func_800C3134(D_801987E4, *(int *)(a2 + 0xA4), q1);
        q2 = a2 + 0x9C;
        func_800C3134(D_801987E4, *(int *)(a2 + 0xA4), q2);
        func_800D3114(a2 + 0x68, a2[0xAA] - 1, 0x3C, 0x20, 0x90, 0x51, 0x10,
                      func_80077A64(1, 0, 0x340, 0x100), func_80077AA4(0, 0x1D9),
                      *(int *)(a2 + 0xA0), q1, q2, 1);
        func_800D3114(a2 + 0x80, a2[0xAA] - 1, 0x3C, 0x20, 0x90, 0x51, 0x10,
                      func_80077A64(1, 0, 0x340, 0x100), func_80077AA4(0, 0x1D9),
                      *(int *)(a2 + 0xA0), &o, &o, 2);
        m.m[0][0] = m.m[1][1] = m.m[2][2] = 0x1000;
        m.t[0] = m.t[1] = m.t[2] = 0;
        m.m[0][1] = m.m[0][2] = m.m[1][0] = m.m[1][2] = m.m[2][0] = m.m[2][1] = 0;
        v = D_8018F008;
        func_80078CC4(&m, &v);
        for (i = 0; i < 8; i++) {
            m.t[0] = *(short *)(a2 + i * 8 + 0x28);
            m.t[1] = *(short *)(a2 + i * 8 + 0x2A);
            m.t[2] = *(short *)(a2 + i * 8 + 0x2C);
            func_800C42A4(D_80199690, &m, 1);
        }
    }
}
