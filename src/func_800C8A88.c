typedef struct { short m[3][3]; int t[3]; } MATRIX;
typedef struct { int vx; int vy; int vz; int pad; } VECTOR;

extern unsigned short D_800F34E2;
extern void func_800C2EAC(unsigned int a0);
extern void func_800C3098(int a0);
extern void func_800C2FF0(int a0, int a1);
extern void func_800C3238(int a0);
extern void func_80071A44(VECTOR *a0, int a1, int a2);
extern void func_80078CC4(MATRIX *a0, VECTOR *a1);
extern void func_800C42A4(unsigned char *a0, MATRIX *a1, int a2);

void func_800C8A88(int a0, int a1, unsigned char *a2) {
    MATRIX m;
    VECTOR v1;
    VECTOR v2;
    unsigned short *g;

    func_800C2EAC(3);
    func_800C3098(0x100);
    func_800C2FF0(0x20, 0x20);
    func_800C3238(2);
    g = &D_800F34E2;
    *g = *(unsigned short *)(a2 + 4);
    m = *(MATRIX *)(a2 + 0x10);
    m.t[0] = *(short *)(a2 + 8);
    m.t[1] = *(short *)(a2 + 0xA);
    m.t[2] = *(short *)(a2 + 0xC);
    func_80071A44(&v2, 0, 0x10);
    v2.vx = *(short *)(a2 + 6);
    v2.vy = *(short *)(a2 + 6);
    v2.vz = *(short *)(a2 + 6);
    v1 = v2;
    func_80078CC4(&m, &v1);
    func_800C42A4((unsigned char *)g - 0xA, &m, 0);
}
