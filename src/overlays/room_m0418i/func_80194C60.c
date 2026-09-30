/* room_m0418i — func_80194C60, blob offset 0x5C78, 0x114 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl 2026-09-27).
 * sprite/billboard draw setup: identity MATRIX scaled by a2+0x10, translate, func_800C42A4; lever: chained-assignment direction */

typedef struct { short m[3][3]; short pad; int t[3]; } MATRIX;
typedef struct { int vx, vy, vz, pad; } VECTOR;
typedef struct { unsigned char pad[0xA]; short v; } S94C8;
extern S94C8 D_801994C8;
extern unsigned char *func_800C2B50();
extern void func_800C2EAC();
extern void func_800C2FF0();
extern void func_800C3098();
extern void func_800C3238();
extern void func_80071A44();
extern void func_80078CC4();
extern void func_800C42A4();

void func_80194C60(void *a0, void *a1, int *a2)
{
    MATRIX m;
    VECTOR t;
    VECTOR v;

    func_800C2EAC(func_800C2B50()[0x24]);
    func_800C2FF0(0x20, 0x20);
    func_800C3098(0x10);
    func_800C3238(2);
    m.m[0][0] = m.m[1][1] = m.m[2][2] = 0x1000;
    m.t[0] = m.t[1] = m.t[2] = 0;
    m.m[0][1] = m.m[0][2] = m.m[1][0] = m.m[1][2] = m.m[2][0] = m.m[2][1] = 0;
    func_80071A44(&v, 0, 0x10);
    v.vx = *(short *)(a2 + 4);
    v.vy = *(short *)(a2 + 4);
    v.vz = 0x1000;
    t = v;
    func_80078CC4(&m, &t);
    m.t[0] = a2[0];
    m.t[1] = a2[1];
    m.t[2] = a2[2];
    D_801994C8.v = *(short *)(a2 + 5);
    func_800C42A4(&D_801994C8, &m, 1);
}
