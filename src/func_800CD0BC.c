extern short D_800E27F8[];
extern short D_800E27FA;
extern short D_800E27FC;
extern int func_80071A54(void);

void func_800CD0BC(int a0, int a1, unsigned char *a2) {
    unsigned short i;
    unsigned char *p;

    *(short *)(a2 + 8) = D_800E27F8[0];
    *(short *)(a2 + 0xA) = D_800E27FA;
    *(short *)(a2 + 0xC) = D_800E27FC;
    *(short *)(a2 + 4) = 0x7F;
    *(short *)(a2 + 6) = 0x3B4;
    a2[3] = 0;
    i = 0;
    do {
        p = a2 + i * 8;
        *(short *)(p + 0x10) = D_800E27F8[0];
        *(short *)(p + 0x12) = D_800E27F8[1];
        *(short *)(p + 0x14) = D_800E27F8[2];
        *(short *)(p + 0x20) = func_80071A54() % 50 - 0x19;
        *(short *)(p + 0x22) = func_80071A54() % 50 - 0x19;
        *(short *)(p + 0x24) = 0;
        i++;
    } while (i < 2);
}
