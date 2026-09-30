extern unsigned short D_800E27F0;
extern unsigned short D_800E27F2;
extern unsigned short D_800E27F4;
extern int func_80071A54();

void func_800CDA5C(int a0, int a1, unsigned char *a2)
{
    unsigned short *src;
    unsigned short i;
    register int v asm("$2");
    unsigned short *q;

    src = &D_800E27F0;
    *(unsigned short *)(a2 + 4) = *src;
    *(unsigned short *)(a2 + 6) = D_800E27F2;
    *(unsigned short *)(a2 + 8) = D_800E27F4;
    *(short *)(a2 + 0xA) = func_80071A54() % 201 - 100;
    v = func_80071A54();
    a2[1] = 0x7F;
    *(short *)(a2 + 0xE) = 0;
    a2[3] = 0;
    *(short *)(a2 + 0xC) = v % 101 - 50;
    i = 0;
    q = src;
    for (; i < 8; i++) {
        *(short *)(a2 + i * 2 + 0x10) = q[0];
        *(short *)(a2 + i * 2 + 0x20) = q[1];
        *(short *)(a2 + i * 2 + 0x30) = q[2];
        *(short *)(a2 + i * 2 + 0x40) = func_80071A54() % 50 - 25;
        v = func_80071A54();
        *(short *)(a2 + i * 2 + 0x60) = 0;
        *(short *)(a2 + i * 2 + 0x50) = v % 50 - 25;
    }
}
