typedef struct {
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char pad0;
    unsigned char f4;
    unsigned char f5;
    unsigned char f6;
    unsigned char pad1;
    short f8;
    short fA;
} S;

extern char D_800E0B38;
extern S D_800E22F8;
extern S D_800F34B8;
extern int *func_800C22F8(void);

int func_800C9A70(void)
{
    int *p;

    p = func_800C22F8();
    *p = (int)&D_800E0B38;
    D_800E22F8.f4 = 0xBD;
    D_800E22F8.f5 = 9;
    D_800E22F8.f8 = 0;
    D_800E22F8.fA = 0x80;
    D_800E22F8.r = 0x80;
    D_800E22F8.g = 0x80;
    D_800E22F8.b = 0x80;
    D_800E22F8.f6 = 0;
    D_800F34B8.f4 = 0xAC;
    D_800F34B8.f5 = 6;
    D_800F34B8.f8 = -0x32;
    D_800F34B8.fA = 0x80;
    D_800F34B8.r = 0x50;
    D_800F34B8.g = 0x50;
    D_800F34B8.b = 0x50;
    D_800F34B8.f6 = 0;
    return 0;
}
