extern void *func_800C22F8(void);
extern unsigned char D_800E0928[];

typedef struct { unsigned char r, g, b, pad0; unsigned char f4, f5, f6, pad1; short f8, fA; } S;

extern S D_800E22D8;
extern S D_800F3498;
extern S D_800E2318;
extern S D_800F34D8;

int func_800C7BA0(void) {
    void **p;

    p = func_800C22F8();
    *p = D_800E0928;
    D_800E22D8.f4 = 0xBD;
    D_800E22D8.f5 = 0x9;
    D_800E22D8.f8 = 0x0;
    D_800E22D8.fA = 0x80;
    D_800E22D8.r = 0x80;
    D_800E22D8.g = 0x80;
    D_800E22D8.b = 0x80;
    D_800E22D8.f6 = 0;
    D_800F3498.f4 = 0xAE;
    D_800F3498.f5 = 0x7;
    D_800F3498.f8 = -0x32;
    D_800F3498.fA = 0x80;
    D_800F3498.r = 0x50;
    D_800F3498.g = 0x50;
    D_800F3498.b = 0x50;
    D_800F3498.f6 = 0;
    D_800E2318.f4 = 0x68;
    D_800E2318.f5 = 0x0;
    D_800E2318.f8 = -0x3C;
    D_800E2318.fA = 0x80;
    D_800E2318.r = 0x50;
    D_800E2318.g = 0x50;
    D_800E2318.b = 0x50;
    D_800E2318.f6 = 0;
    D_800F34D8.f4 = 0x42;
    D_800F34D8.f5 = 0x20;
    D_800F34D8.f8 = 0x32;
    D_800F34D8.fA = 0x80;
    D_800F34D8.r = 0x80;
    D_800F34D8.g = 0x80;
    D_800F34D8.b = 0x80;
    D_800F34D8.f6 = 0;
    return 0;
}
