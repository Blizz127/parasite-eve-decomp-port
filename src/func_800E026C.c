extern short D_800E21A4;
extern int D_800BCFA4;
extern void func_800E051C();

void func_800E026C(unsigned char *a0)
{
    unsigned char c;

    if (a0[0] == 0) {
        return;
    }
    *(short *)0x1F80001E = 0x77D3;
    *(short *)0x1F800022 = 0x34;
    *(unsigned char *)0x1F800018 = 0x80;
    *(unsigned char *)0x1F800019 = 0x80;
    *(unsigned char *)0x1F80001A = 0x80;
    *(unsigned char *)0x1F80001C = a0[3];
    *(unsigned char *)0x1F80001D = 0xD0;
    *(unsigned short *)0x1F800024 = *(unsigned short *)(a0 + 4);
    *(unsigned short *)0x1F800026 = *(unsigned short *)(a0 + 6);
    *(unsigned short *)0x1F800028 = *(unsigned short *)(a0 + 8);
    *(int *)0x1F80002C = *(short *)(a0 + 0xE);
    *(int *)0x1F800034 = D_800BCFA4;
    if (a0[0] != 1) {
        return;
    }
    if (a0[2] == 0) {
        c = a0[3] + 0x10;
        a0[3] = c;
        a0[2] = a0[1];
        if (c > 0x30) {
            a0[0] = 0;
            a0[3] = 0;
            D_800E21A4--;
            return;
        }
    } else {
        a0[2]--;
    }
    if (a0[0] == 1) {
        func_800E051C();
    }
}
