extern short D_800E21A4;
extern int D_800BCFA4;
extern void func_800E051C();

void func_800E03A0(unsigned char *a0)
{
    int r;
    int g;
    int b;
    short lv;

    if (a0[0] == 0) {
        return;
    }
    *(short *)0x1F80001E = 0x7713;
    *(short *)0x1F800022 = 0x34;
    *(unsigned char *)0x1F80001C = 0xC0;
    *(unsigned char *)0x1F80001D = 0xCA;
    *(unsigned short *)0x1F800024 = *(unsigned short *)(a0 + 4);
    *(unsigned short *)0x1F800026 = *(unsigned short *)(a0 + 6);
    *(unsigned short *)0x1F800028 = *(unsigned short *)(a0 + 8);
    *(int *)0x1F80002C = *(short *)(a0 + 0xE);
    *(int *)0x1F800034 = D_800BCFA4;
    lv = *(short *)(a0 + 0xC);
    r = a0[0x10] - lv;
    g = a0[0x11] - lv;
    b = a0[0x12] - lv;
    if (r < 0) {
        r = 0;
    }
    if (g < 0) {
        g = 0;
    }
    if (b < 0) {
        b = 0;
    }
    *(unsigned char *)0x1F800018 = r;
    *(unsigned char *)0x1F800019 = g;
    *(unsigned char *)0x1F80001A = b;
    if (a0[0] != 1) {
        *(short *)(a0 + 0xC) -= a0[1];
        if (*(short *)(a0 + 0xC) < 0) {
            a0[0] = 0;
            lv = D_800E21A4;
            *(short *)(a0 + 0xC) = 0;
            D_800E21A4 = lv - 1;
            return;
        }
    } else {
        *(short *)(a0 + 0xC) += a0[1];
        if (*(short *)(a0 + 0xC) >= 0x100) {
            *(short *)(a0 + 0xC) = 0xFF;
            a0[0] = 2;
        }
    }
    func_800E051C();
}
