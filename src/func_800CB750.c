extern void func_800C2EAC(int a0);
extern void func_800C3098(int a0);
extern void func_800C2FF0(int a0, int a1);
extern void func_800C3238(int a0);
extern void func_80071A44(void *a0, int a1, int a2);
extern void func_80078CC4(void *a0, void *a1);
extern void func_800C42A4(void *a0, void *a1, int a2);

typedef struct { short m[3][3]; short pad; int t[3]; } MTX;
typedef struct { int x, y, z, pad; } VEC;
typedef struct {
    unsigned char r, g, b, pad0;
    unsigned char f4b, f5b, f6b, pad1;
    short f8s, fAs;
} S;
typedef struct {
    short pad0;
    short pad2;
    unsigned short f4;
    short f6;
    short f8;
    short fA;
    short fC;
    short padE;
    short f10;
    short f12;
    short f14;
} A;

extern S D_800E2338;

void func_800CB750(int a0, int a1, A *a2) {
    MTX m;
    VEC v1;
    VEC v2;
    int d;
    unsigned short e;

    func_800C2EAC(3);
    func_800C3098(0x10);
    func_800C2FF0(0x20, 0x20);
    func_800C3238(2);

    d = a2->f6 + 0x42C;
    e = a2->f4;
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
    D_800E2338.fAs = e;
    m.t[0] = a2->f8;
    m.t[1] = a2->fA;
    m.t[2] = a2->fC;
    func_80071A44(&v1, 0, 0x10);
    v1.x = d;
    v1.y = d;
    v1.z = d;
    func_80078CC4(&m, &v1);
    func_800C42A4(&D_800E2338, &m, 1);

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
    m.t[0] = a2->f10;
    m.t[1] = a2->f12;
    m.t[2] = a2->f14;
    func_80071A44(&v2, 0, 0x10);
    v2.x = d;
    v2.y = d;
    v2.z = d;
    func_80078CC4(&m, &v2);
    func_800C42A4(&D_800E2338, &m, 1);
}
