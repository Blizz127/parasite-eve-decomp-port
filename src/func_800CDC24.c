extern unsigned short D_800E27F0;
extern unsigned short D_800E27F2;
extern unsigned short D_800E27F4;
extern int func_80071A54();

void func_800CDC24(int a0, int a1, unsigned char *a2)
{
    int v;

    *(unsigned short *)(a2 + 4) = D_800E27F0;
    *(unsigned short *)(a2 + 6) = D_800E27F2;
    *(unsigned short *)(a2 + 8) = D_800E27F4;
    a2[1] = 0x7F;
    *(short *)(a2 + 0xA) = func_80071A54() % 201 - 100;
    v = func_80071A54();
    *(short *)(a2 + 0xE) = 0;
    *(short *)(a2 + 0xC) = v % 101 - 50;
}
