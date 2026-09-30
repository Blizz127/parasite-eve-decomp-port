typedef struct {
    short vx, vy, vz;
} Vec3;

extern int D_8009D280;
extern int D_8009D224;
extern short D_8009D308;
extern int D_8009CDA4;
extern unsigned int D_8009D1A0;
extern unsigned int D_8009D2E8;
extern short D_800942EC;
extern Vec3 D_800BEA40[];
extern int D_800B8A18;
extern unsigned char D_800BCFEE[];
extern signed char D_800B0CE4;
extern unsigned int D_800B0CD8[];
extern int D_800B162C;
extern int D_800B1628;

extern void func_8006B35C(void);
extern void func_8006B4F8(int);
extern void func_8006BD68(void);
extern void func_80034FC4(void);
extern void func_8001266C(void);
extern void func_8003F758(Vec3 *, int, int, int);
extern void func_8003F798(Vec3 *, int, int, int, int);
extern void func_80068B94(void);
extern void func_80079024(int);
extern void func_8006BE4C(void);
extern int func_8006BECC(void);
extern void func_8006C4C4(int);
extern int func_8006C5BC(void);
extern void func_8001A918(void);
extern void func_800371B0(int);
extern void func_800125E0(void);
extern void func_800E0060(void);
extern void func_80074DC0(int);
extern void func_80074D28(int);

#define CLAMP(v)                   \
    if ((v) > 0x7FFF) {            \
        (v) = 0x7FFF;              \
    } else if ((v) < -0x7FFF) {    \
        (v) = -0x7FFF;             \
    }

#define SET_VEC(n, X, Y, Z)        \
    {                              \
    Vec3 *p;                       \
    int x, y, z;                   \
    x = (X);                       \
    y = (Y);                       \
    z = (Z);                       \
    CLAMP(x);                      \
    CLAMP(y);                      \
    CLAMP(z);                      \
    p = &D_800BEA40[n];            \
    p->vx = x;                     \
    p->vy = y;                     \
    p->vz = z;                     \
    }

void func_8003F074(void)
{
    unsigned char *b;
    unsigned int *f;

    func_8006B35C();
    func_8006B4F8(D_8009D280);
    func_8006BD68();
    D_8009D224 = 1;
    D_8009D308 = 1;
    func_80034FC4();
    func_8001266C();
    func_8003F758(D_800BEA40, 0x28, 0x28, 0x28);
    SET_VEC(0, 0x10000, 0, 0);
    func_8003F798(D_800BEA40, 0, 0xFF, 0xFF, 0xFF);
    SET_VEC(1, 0, 0x10000, 0);
    func_8003F798(D_800BEA40, 1, 0x80, 0x80, 0x80);
    SET_VEC(2, 0, 0, 0x10000);
    func_8003F798(D_800BEA40, 2, 0x60, 0x60, 0x60);
    func_80068B94();
    func_80079024(D_800B8A18);
    b = D_800BCFEE;
    *b |= 0x40;
    func_8006BE4C();
    while (func_8006BECC() == 1) {
    }
    func_8006C4C4(D_800B0CE4);
    while (func_8006C5BC() == 1) {
    }
    func_8001A918();
    func_800371B0((D_800B0CD8[0] & 0x40000000) ? D_800B162C : D_800B1628);
    func_800125E0();
    func_800E0060();
    D_8009CDA4 = 0;
    D_800942EC = 0;
    func_80074DC0(0);
    func_80074D28(1);
    f = D_800B0CD8;
    D_8009D1A0 &= ~0x40;
    D_8009D2E8 &= ~0xC;
    *f &= ~0x402;
}
