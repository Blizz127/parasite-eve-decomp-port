typedef struct { short m[3][3]; int t[3]; } MATRIX;
typedef struct { int vx; int vy; int vz; int pad; } VECTOR;

extern short D_800E2318[];
extern void func_800C2EAC(unsigned int a0);
extern void func_800C3098(int a0);
extern void func_800C2FF0(int a0, int a1);
extern void func_800C3238(int a0);
extern void func_80071A44(VECTOR *a0, int a1, int a2);
extern void func_80078CC4(MATRIX *a0, VECTOR *a1);
extern void func_800C42A4(short *a0, MATRIX *a1, int a2);

void func_800C8870(int a0, int a1, unsigned char *a2)
{
    MATRIX m;
    VECTOR v;
    int s;
    unsigned short u;

    func_800C2EAC(3);
    func_800C3098(0x10);
    func_800C2FF0(0x20, 0x20);
    func_800C3238(2);
    s = *(short *)(a2 + 6) + 0x170;
    u = *(unsigned short *)(a2 + 4);
    m.m[0][0] = m.m[1][1] = m.m[2][2] = 0x1000;
    m.t[0] = m.t[1] = m.t[2] = 0;
    m.m[0][1] = m.m[0][2] = m.m[1][0] = m.m[1][2] = m.m[2][0] = m.m[2][1] = 0;
    D_800E2318[5] = u;
    m.t[0] = *(short *)(a2 + 8);
    m.t[1] = *(short *)(a2 + 0xA);
    m.t[2] = *(short *)(a2 + 0xC);
    func_80071A44(&v, 0, 0x10);
    v.vx = s;
    v.vy = s;
    v.vz = s;
    func_80078CC4(&m, &v);
    func_800C42A4(D_800E2318, &m, 1);
}
