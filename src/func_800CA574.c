extern void *func_800C22F8(void);
extern unsigned char D_800E0C88[];

typedef struct { unsigned char r, g, b, pad0; unsigned char f4, f5, f6, pad1; short f8, fA; } S;

extern S D_800E2308;
extern S D_800F34C8;
extern S D_800E2338;
extern S D_800F34E8;

int func_800CA574(void) {
    void **p;

    p = func_800C22F8();
    *p = D_800E0C88;
    D_800E2308.f4 = 0xBD;
    D_800E2308.f5 = 0x9;
    D_800E2308.f8 = 0x0;
    D_800E2308.fA = 0x80;
    D_800E2308.r = 0x80;
    D_800E2308.g = 0x80;
    D_800E2308.b = 0x80;
    D_800E2308.f6 = 0;
    D_800F34C8.f4 = 0xAE;
    D_800F34C8.f5 = 0x7;
    D_800F34C8.f8 = -0x32;
    D_800F34C8.fA = 0x80;
    D_800F34C8.r = 0x50;
    D_800F34C8.g = 0x50;
    D_800F34C8.b = 0x50;
    D_800F34C8.f6 = 0;
    D_800E2338.f4 = 0x68;
    D_800E2338.f5 = 0x0;
    D_800E2338.f8 = -0x3C;
    D_800E2338.fA = 0x80;
    D_800E2338.r = 0x50;
    D_800E2338.g = 0x50;
    D_800E2338.b = 0x50;
    D_800E2338.f6 = 0;
    D_800F34E8.f4 = 0x42;
    D_800F34E8.f5 = 0x20;
    D_800F34E8.f8 = 0x32;
    D_800F34E8.fA = 0x80;
    D_800F34E8.r = 0x80;
    D_800F34E8.g = 0x80;
    D_800F34E8.b = 0x80;
    D_800F34E8.f6 = 0;
    return 0;
}
