/* VRAM 0x800C9D9C / file 0xBA59C / size 0x104. */
extern unsigned char *D_8009D254;
extern unsigned char D_800E0AF8[];
extern unsigned short D_800E2358;
extern unsigned short D_800E235A;
extern unsigned short D_800E235C;

struct Eight {
    unsigned int w[8];
};

struct Inner {
    unsigned char pad[0x260];
    struct Eight eight;
};

struct Outer {
    unsigned char pad[0x238];
    struct Inner *p;
};

extern struct Outer *D_800E27A4;
extern void func_80078C34(unsigned char *a0, unsigned char *a1, unsigned char *a2);

void func_800C9D9C(int a0, int a1, unsigned char *a2) {
    unsigned char *p = *(unsigned char **)D_8009D254;
    unsigned char *q = *(unsigned char **)(p + 0x68);
    unsigned short n = *(unsigned short *)(q + 6);
    unsigned short buf[3];
    struct Outer *op;

    func_80078C34((unsigned char *)D_800E27A4->p + 0x260,
                  D_800E0AF8 + (short)(n - 1) * 8, (unsigned char *)buf);
    *(unsigned short *)(a2 + 8) = D_800E2358 + buf[0];
    *(unsigned short *)(a2 + 0xA) = D_800E235A + buf[1];
    op = D_800E27A4;
    *(unsigned short *)(a2 + 0xC) = D_800E235C + buf[2];
    *(struct Eight *)(a2 + 0x10) = op->p->eight;
    *(unsigned short *)(a2 + 4) = 0x7F;
}
