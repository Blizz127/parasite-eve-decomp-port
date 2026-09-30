extern void func_800C2EAC(int a0);
extern void func_800C3098(int a0);
extern void func_800C2FF0(int a0, int a1);
extern void func_800C3238(int a0);
extern void func_800C3B04(void *a0);

typedef struct {
    short f0;
    short f2;
    short f4;
    short pad6;
    int pad8;
    int padC;
    int f10;
    int f14;
    int f18;
    int pad1C;
    int pad20;
    int pad24;
    short f28;
    short pad2A;
    int pad2C;
    int pad30;
    int pad34;
    int pad38;
    int pad3C;
} P;

extern P D_800E27B0;
extern P D_800E2770;

void func_800CDD0C(int a0, int a1, unsigned char *a2) {
    P *q;
    P *r;
    unsigned short i;
    unsigned char c;

    func_800C2EAC(3);
    func_800C3098(0x100);
    func_800C2FF0(0x20, 0x20);
    func_800C3238(2);
    q = &D_800E27B0;
    q->f0 = *(unsigned short *)(a2 + 4) + *(unsigned short *)(a2 + 0xA);
    D_800E27B0.f2 = *(unsigned short *)(a2 + 6) + *(unsigned short *)(a2 + 0xC);
    D_800E27B0.f4 = *(unsigned short *)(a2 + 8) + *(unsigned short *)(a2 + 0xE);
    D_800E27B0.f10 = (*(signed char *)(a2 + 3) << 3) + 0x20C;
    D_800E27B0.f14 = (*(signed char *)(a2 + 3) << 3) + 0x20C;
    D_800E27B0.f18 = (*(signed char *)(a2 + 3) << 3) + 0x20C;
    c = a2[1];
    D_800E27B0.f28 = (signed char)c;
    func_800C3B04(q);
    func_800C3098(0x10);
    i = 0;
    r = &D_800E2770;
    do {
        r->f0 = *(unsigned short *)(a2 + i * 2 + 0x10) + *(unsigned short *)(a2 + 0xA);
        r->f2 = *(unsigned short *)(a2 + i * 2 + 0x20) + *(unsigned short *)(a2 + 0xC);
        r->f4 = *(unsigned short *)(a2 + i * 2 + 0x30) + *(unsigned short *)(a2 + 0xE);
        c = a2[1];
        r->f28 = (signed char)c;
        i++;
        func_800C3B04(r);
    } while (i < 8);
}
