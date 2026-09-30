/* VRAM 0x800C7F60 / file 0xB8760 / size 0x104. */
extern unsigned char *D_8009D254;
extern unsigned char D_800E08A8[];
extern unsigned short D_800E2348;
extern unsigned short D_800E234A;
extern unsigned short D_800E234C;

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

extern struct Outer *D_800E279C;
extern void func_80078C34(unsigned char *a0, unsigned char *a1, unsigned char *a2);

void func_800C7F60(int a0, int a1, unsigned char *a2) {
    unsigned char *p = *(unsigned char **)D_8009D254;
    unsigned char *q = *(unsigned char **)(p + 0x68);
    unsigned short n = *(unsigned short *)(q + 6);
    unsigned short buf[3];
    struct Outer *op;

    func_80078C34((unsigned char *)D_800E279C->p + 0x260,
                  D_800E08A8 + (short)(n - 1) * 8, (unsigned char *)buf);
    *(unsigned short *)(a2 + 8) = D_800E2348 + buf[0];
    *(unsigned short *)(a2 + 0xA) = D_800E234A + buf[1];
    op = D_800E279C;
    *(unsigned short *)(a2 + 0xC) = D_800E234C + buf[2];
    *(struct Eight *)(a2 + 0x10) = op->p->eight;
    *(unsigned short *)(a2 + 4) = 0x7F;
}
