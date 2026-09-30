/* VRAM 0x800C7E50 / file 0xB8650 / size 0x110. */
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
extern int func_80071A54(void);
extern void func_80078C34(unsigned char *a0, unsigned char *a1, unsigned char *a2);

void func_800C7E50(int a0, int a1, unsigned char *a2) {
    unsigned char *s0 = a2;
    unsigned short buf[3];

    buf[0] = -(func_80071A54() % 3 + 9);
    buf[1] = -(func_80071A54() % 3 + 9);
    buf[2] = func_80071A54() % 5 - 2;
    func_80078C34((unsigned char *)D_800E279C->p, (unsigned char *)buf, a2 + 0x10);
    *(unsigned short *)(s0 + 8) = D_800E2348;
    *(unsigned short *)(s0 + 0xA) = D_800E234A;
    *(unsigned short *)(s0 + 0xC) = D_800E234C;
    *(unsigned char *)(s0 + 2) = 0x14;
    *(unsigned char *)(s0 + 1) = 0;
}
