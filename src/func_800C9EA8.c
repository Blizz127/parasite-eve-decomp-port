typedef struct { short m[3][3]; int t[3]; } MATRIX;
typedef struct { short vx; short vy; short vz; short pad; } SVECTOR;
typedef struct { int vx; int vy; int vz; int pad; } VECTOR;

extern unsigned char *D_8009D254;
extern short D_800E0AD8[];
extern unsigned char D_800E22F8[];
extern void func_800C2EAC(unsigned int a0);
extern void func_800C3098(int a0);
extern void func_800C2FF0(int a0, int a1);
extern void func_800C3238(int a0);
extern void func_800794C4(SVECTOR *a0, MATRIX *a1);
extern void func_80071A44(VECTOR *a0, int a1, int a2);
extern void func_80078CC4(MATRIX *a0, VECTOR *a1);
extern void func_800C42A4(unsigned char *a0, MATRIX *a1, int a2);

void func_800C9EA8(int a0, int a1, unsigned char *a2) {
    MATRIX m;
    SVECTOR rot;
    VECTOR v1;
    VECTOR v2;
    short n;
    short *tab;
    int ni;
    short *base;

    n = *(unsigned short *)(*(unsigned char **)(*(unsigned char **)D_8009D254 + 0x68) + 6);
    rot.vx = 0;
    rot.vy = 0;
    n = n - 1;
    rot.vz = *(signed char *)(a2 + 1) << 6;
    func_800C2EAC(3);
    func_800C3098(0x10);
    func_800C2FF0(0x10, 0x10);
    func_800C3238(0);
    func_800794C4(&rot, &m);
    m.t[0] = *(short *)(a2 + 8);
    m.t[1] = *(short *)(a2 + 0xA);
    m.t[2] = *(short *)(a2 + 0xC);
    func_80071A44(&v2, 0, 0x10);
    ni = n;
    base = D_800E0AD8;
    tab = (short *)(ni * 2 + (unsigned int)base);
    v2.vx = *tab;
    v2.vy = *tab;
    v2.vz = *tab;
    v1 = v2;
    func_80078CC4(&m, &v1);
    func_800C42A4(D_800E22F8, &m, 1);
}
