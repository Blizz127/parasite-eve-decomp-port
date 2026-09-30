/* room_m0418i — func_8018FBA4, blob offset 0xBBC, 0x204 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl7 2026-09-27).
 * 30-particle draw (companion of func_8018FDA8): identity+ScaleMatrix, per-kind colour/size of D_801994A8; t = b; t += k; t -= c >> 1 keeps retail association. */

typedef struct { short m[3][3]; int t[3]; } MATRIX;
typedef struct { int vx, vy, vz, pad; } VECTOR;
typedef struct { char pad0[4]; unsigned char b4, b5; char pad6[4]; short hA; } OBJ;
extern OBJ D_801994A8;
extern char *func_800C2B50();
extern void func_800C2EAC(), func_800C2FF0(), func_800C3098(), func_800C3238();
extern void func_80071A44(), func_80078CC4(), func_800C42A4();

#define B(off) (a2 + i)[off]
#define H(off) *(short *)(a2 + i * 2 + (off))
#define V(off) *(short *)(a2 + i * 8 + (off))

void func_8018FBA4(int a0, int a1, unsigned char *a2)
{
    MATRIX m;
    VECTOR s;
    VECTOR v;
    char *r;
    unsigned int i;
    int t;

    r = func_800C2B50();
    func_800C2EAC(r[0x24]);
    func_800C3098(0x10);
    func_800C3238(2);
    for (i = 0; i < 30; i++) {
        if (B(0x21C) == 1) {
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
            v.vx = H(0xF0);
            v.vy = H(0xF0);
            v.vz = H(0xF0);
            s = v;
            func_80078CC4(&m, &s);
            m.t[0] = V(0);
            m.t[1] = V(2);
            m.t[2] = V(4);
            D_801994A8.hA = H(0x168);
            if (B(0x1A4) == 0x44) {
                func_800C2FF0(0x20, 0x20);
                D_801994A8.b4 = B(0x1A4);
            }
            if (B(0x1A4) == 0x4C) {
                func_800C2FF0(0x10, 0x10);
                t = B(0x1A4);
                t += 3;
                t -= B(0x1E0) >> 1;
                D_801994A8.b4 = t;
            }
            if (B(0x1A4) == 0x58) {
                func_800C2FF0(0x10, 0x10);
                t = B(0x1A4);
                t += 7;
                t -= B(0x1E0) >> 1;
                D_801994A8.b4 = t;
            }
            D_801994A8.b5 = B(0x1C2);
            func_800C42A4(&D_801994A8, &m, 1);
        }
    }
}
