typedef struct Trk {
    unsigned int f0;
    unsigned int f4;
    unsigned int f8;
    unsigned int fC;
    unsigned int f10;
    unsigned int f14;
    unsigned int f18;
    unsigned int pad1[20];
    unsigned int f6C;
    unsigned int f70;
    unsigned int pad2[3];
    unsigned int f80;
} Trk;

extern Trk *D_8009D2C8;
extern unsigned int D_800BCD50;
extern unsigned int D_800BCD5C;
extern unsigned int D_800BCD60;
extern unsigned char D_800BA560[];
extern unsigned char D_800B8AC0[];
extern void func_80089724(unsigned char *a0, unsigned int *a1, unsigned int a2, unsigned int a3);
extern void func_80087728(unsigned int a0);

void func_80089784(void)
{
    Trk *p;
    Trk *q;
    unsigned int acc;
    unsigned int mask;
    unsigned int s1v;
    unsigned int s0v;

    p = D_8009D2C8;
    acc = 0;
    mask = ~(D_800BCD50 | D_800BCD60);
    s1v = p->f6C & p->f80;
    if (s1v & p->f70) {
        D_8009D2C8 = (Trk *)((unsigned char *)p + 0x68);
        func_80089724(D_800BA560, &acc, s1v & p->f70, mask);
        q = D_8009D2C8;
        s1v &= ~q->f8;
        D_8009D2C8 = (Trk *)((unsigned char *)q - 0x68);
        q->f18 &= ~q->f8;
    }
    s0v = D_8009D2C8->f4 & D_8009D2C8->f18;
    if (s0v & D_8009D2C8->f8) {
        func_80089724(D_800B8AC0, &acc, s0v & D_8009D2C8->f8, mask);
        q = D_8009D2C8;
        q->f18 &= ~q->f8;
        s0v &= ~q->f8;
    }
    if (s1v != 0) {
        D_8009D2C8 = (Trk *)((unsigned char *)D_8009D2C8 + 0x68);
        func_80089724(D_800BA560, &acc, s1v, mask);
        D_8009D2C8->f18 = 0;
        D_8009D2C8 = (Trk *)((unsigned char *)D_8009D2C8 - 0x68);
    }
    if (s0v != 0) {
        func_80089724(D_800B8AC0, &acc, s0v, mask);
        D_8009D2C8->f18 = 0;
    }
    acc |= D_800BCD5C;
    D_800BCD5C = 0;
    if (acc != 0) {
        func_80087728(acc);
    }
}
