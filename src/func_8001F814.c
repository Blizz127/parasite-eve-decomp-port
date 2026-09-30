typedef struct {
    unsigned char pad0[0xE];
    unsigned char fE;
    unsigned char fF;
    unsigned char pad10[4];
    unsigned int f14;
    unsigned char pad18[0x12];
    short f2A;
    unsigned char pad2C[2];
    short f2E;
    unsigned char pad30[2];
    short f32;
    unsigned char pad34[0x64];
    unsigned int f98;
} Ent;

typedef struct {
    unsigned char pad0[0x4C];
    unsigned int f4C;
} St;

extern Ent *D_8009D254;
extern St *D_8009D278;
extern short D_8009D298[1];
extern unsigned char D_8009D29A[];
extern unsigned char D_8009D29B[];
extern unsigned int D_8009D29C[];

extern int func_800305C8(Ent *, Ent *);
extern void func_8001A680(Ent *, int);
extern void func_8006DE80(int, int, int, int, int);

short func_8001F814(Ent *a)
{
    int r;
    int k;
    register int cmd asm("$2");

    r = 0;
    switch (D_8009D254->fE) {
    case 6:
    case 8:
    case 10:
    case 12:
    case 13:
    case 14:
    case 15:
        D_8009D29A[0] = D_8009D254->fE;
        D_8009D29B[0] = D_8009D254->fF;
        D_8009D298[0] = 1;
        D_8009D29C[0] = D_8009D254->f14;
        break;
    case 7:
    case 9:
    case 11:
        D_8009D29A[0] = D_8009D254->fE;
        D_8009D29B[0] = D_8009D254->fF;
        D_8009D298[0] = 1;
        D_8009D29C[0] = D_8009D254->fF << 16;
        break;
    }
    if (D_8009D278->f4C & 0x12000) {
        return r;
    }
    r = func_800305C8(a, D_8009D254);
    k = (short)r;
    if (k < 0x200) {
        cmd = 0;
    } else if (k < 0x600) {
        cmd = 2;
    } else if (k < 0xA00) {
        cmd = 1;
    } else {
        cmd = (k < 0xE00) ? 3 : 0;
    }
    func_8001A680(D_8009D254, cmd);
    func_8006DE80(0x46A, 0, D_8009D254->f2A, D_8009D254->f2E, D_8009D254->f32);
    if (D_8009D254->f98 & 0x100) {
        D_8009D254->f98 &= ~0x100;
        D_8009D298[0] = 2;
    }
    return r;
}
