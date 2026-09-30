/* room_m0418i — func_80192D90, blob offset 0x3DA8, 0x1C0 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl7 2026-09-27).
 * 32-slot ring draw: rsin/rcos offsets, identity+ScaleMatrix, D_801996A0 object via func_800C3134/func_800C42A4. */

typedef struct { short m[3][3]; int t[3]; } MATRIX;
typedef struct { int vx, vy, vz, pad; } VECTOR;
typedef struct { short x, y, z; } SV;
extern char D_801996A0[];
extern unsigned char D_80198848[];
extern int D_8019956C;
extern int D_8019957C;
extern char *func_800C2B50();
extern int func_80077CF4(), func_80077DC4();
extern void func_800C2EAC(), func_800C2FF0(), func_800C3098(), func_800C3238();
extern void func_80071A44(), func_80078CC4(), func_800C3134(), func_800C42A4();

#define B(off) (a2 + i)[off]
#define H(off) *(short *)(a2 + i * 2 + (off))

void func_80192D90(int a0, int a1, unsigned char *a2)
{
    MATRIX m;
    SV d;
    VECTOR s;
    VECTOR v;
    char *r;
    unsigned int i;

    r = func_800C2B50();
    func_800C2EAC(r[0x24]);
    func_800C2FF0(0x20, 0x20);
    func_800C3098(0x10);
    func_800C3238(2);
    for (i = 0; i < 32; i++) {
        if (B(0) == 1) {
            d.x = (func_80077CF4(H(0x40)) * H(0xC0)) >> 12;
            d.y = 0;
            d.z = (func_80077DC4(H(0x40)) * H(0xC0)) >> 12;
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
            v.vx = H(0x80);
            v.vy = H(0x80);
            v.vz = 0x1000;
            s = v;
            func_80078CC4(&m, &s);
            m.t[0] = d.x + D_8019956C;
            m.t[1] = d.y;
            m.t[2] = d.z + D_8019957C;
            func_800C3134(D_80198848, B(0x20), D_801996A0);
            func_800C42A4(D_801996A0, &m, 1);
        }
    }
}
