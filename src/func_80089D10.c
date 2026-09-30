typedef struct Trk {
    unsigned int f0;
    unsigned int f4;
    unsigned int f8;
    unsigned int pad0[10];
    unsigned int f34;
    unsigned int f38;
    unsigned int f3C;
    unsigned int pad1[11];
    unsigned int f6C;
    unsigned int f70;
    unsigned int pad2[10];
    unsigned int f9C;
    unsigned int fA0;
    unsigned int fA4;
} Trk;

extern Trk *D_8009D2C8;
extern unsigned int D_8009D2C4;
extern unsigned int D_800BCD50;
extern unsigned int D_800BCD60;
extern unsigned int D_800BCD74;
extern unsigned int D_800C0DD8;
extern unsigned char D_800BA560[];
extern unsigned char D_800B8AC0[];
extern void func_80089724(unsigned char *a0, unsigned int *a1, unsigned int a2, unsigned int a3);

void func_80089D10(void)
{
    Trk *p;
    unsigned int acc;
    unsigned int mask;
    unsigned int s1v;
    unsigned int s0v;

    p = D_8009D2C8;
    acc = 0;
    mask = ~(D_800BCD50 | D_800BCD60);
    s1v = p->f6C & p->fA4;
    if (s1v & p->f70) {
        D_8009D2C8 = (Trk *)((unsigned char *)p + 0x68);
        func_80089724(D_800BA560, &acc, s1v & p->f70, mask);
        s1v &= ~D_8009D2C8->f8;
        D_8009D2C8 = (Trk *)((unsigned char *)D_8009D2C8 - 0x68);
    }
    s0v = D_8009D2C8->f4 & D_8009D2C8->f3C;
    if (s0v & D_8009D2C8->f8) {
        func_80089724(D_800B8AC0, &acc, s0v & D_8009D2C8->f8, mask);
        s0v &= ~D_8009D2C8->f8;
    }
    if (s1v != 0) {
        D_8009D2C8 = (Trk *)((unsigned char *)D_8009D2C8 + 0x68);
        func_80089724(D_800BA560, &acc, s1v, mask);
        D_8009D2C8 = (Trk *)((unsigned char *)D_8009D2C8 - 0x68);
    }
    if (s0v != 0) {
        func_80089724(D_800B8AC0, &acc, s0v, mask);
    }
    acc |= D_800BCD74;
    D_800C0DD8 = acc;
    D_8009D2C4 |= 0x100;
}
