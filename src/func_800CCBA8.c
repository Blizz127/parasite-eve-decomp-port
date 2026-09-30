extern void *func_800C22F8(void);
extern unsigned char D_800E0EB8[];
extern void func_80078CC4(void *a0, void *a1);

typedef struct { short m[3][3]; short pad; int t[3]; } MTX;
typedef struct { int w[4]; } V16;
typedef struct { unsigned char r, g, b, pad0; unsigned char f4, f5, f6, pad1; short f8, fA; } S;

extern V16 D_800C220C;
extern V16 D_800C221C;
extern V16 D_800C222C;
extern MTX D_800F3478;
extern MTX D_800F33C0;
extern MTX D_800F32B0;
extern S D_800E2298;
extern S D_800E2250;
extern S D_800E27E0;
extern S D_800F3460;

int func_800CCBA8(void) {
    void **p;
    V16 a;
    V16 b;
    V16 c;
    MTX *mp;

    p = func_800C22F8();
    *p = D_800E0EB8;

    a = D_800C220C;
    mp = &D_800F3478;
    D_800F3478.m[2][2] = 0x1000;
    D_800F3478.m[1][1] = 0x1000;
    mp->m[0][0] = 0x1000;
    D_800F3478.t[2] = 0;
    D_800F3478.t[1] = 0;
    D_800F3478.t[0] = 0;
    D_800F3478.m[2][1] = 0;
    D_800F3478.m[2][0] = 0;
    D_800F3478.m[1][2] = 0;
    D_800F3478.m[1][0] = 0;
    D_800F3478.m[0][2] = 0;
    D_800F3478.m[0][1] = 0;
    func_80078CC4(mp, &a);

    b = D_800C221C;
    mp = &D_800F33C0;
    D_800F33C0.m[2][2] = 0x1000;
    D_800F33C0.m[1][1] = 0x1000;
    mp->m[0][0] = 0x1000;
    D_800F33C0.t[2] = 0;
    D_800F33C0.t[1] = 0;
    D_800F33C0.t[0] = 0;
    D_800F33C0.m[2][1] = 0;
    D_800F33C0.m[2][0] = 0;
    D_800F33C0.m[1][2] = 0;
    D_800F33C0.m[1][0] = 0;
    D_800F33C0.m[0][2] = 0;
    D_800F33C0.m[0][1] = 0;
    func_80078CC4(mp, &b);

    c = D_800C222C;
    mp = &D_800F32B0;
    D_800F32B0.m[2][2] = 0x1000;
    D_800F32B0.m[1][1] = 0x1000;
    mp->m[0][0] = 0x1000;
    D_800F32B0.t[2] = 0;
    D_800F32B0.t[1] = 0;
    D_800F32B0.t[0] = 0;
    D_800F32B0.m[2][1] = 0;
    D_800F32B0.m[2][0] = 0;
    D_800F32B0.m[1][2] = 0;
    D_800F32B0.m[1][0] = 0;
    D_800F32B0.m[0][2] = 0;
    D_800F32B0.m[0][1] = 0;
    func_80078CC4(mp, &c);

    D_800E2298.f4 = 0x42;
    D_800E2298.f5 = 0x20;
    D_800E2298.f8 = 0x32;
    D_800E2298.fA = 0x7F;
    D_800E2298.r = 0x80;
    D_800E2298.g = 0x80;
    D_800E2298.b = 0x80;
    D_800E2250.f4 = 0x80;
    D_800E2250.f5 = 0x4;
    D_800E2250.f8 = -0x32;
    D_800E2250.fA = 0x7F;
    D_800E2250.r = 0x80;
    D_800E2250.g = 0x80;
    D_800E2250.b = 0x80;
    D_800E27E0.f4 = 0x68;
    D_800E27E0.f5 = 0x0;
    D_800E27E0.f8 = -0x63;
    D_800E27E0.fA = 0x7F;
    D_800E27E0.r = 0x40;
    D_800E27E0.g = 0x80;
    D_800E27E0.b = 0x40;
    D_800F3460.f4 = 0x6E;
    D_800F3460.f5 = 0x3;
    D_800F3460.f8 = -0x64;
    D_800F3460.fA = 0x7F;
    D_800F3460.r = 0x80;
    D_800F3460.g = 0x80;
    D_800F3460.b = 0x80;
    return 0;
}
