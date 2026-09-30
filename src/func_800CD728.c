extern int D_800F33F8;
extern int D_800F33FC;
extern int D_800F3400;
extern char D_800F340C;
extern short D_800F340E;
extern char D_800F3408;
extern char D_800E27D4;
extern char D_800E27D5;
extern short D_800E27D6;
extern char D_800E27D0;
extern char D_800E27D1;
extern char D_800E27D2;
extern int D_800E2780;
extern int D_800E2784;
extern int D_800E2788;
extern char D_800E2794;
extern char D_800E2795;
extern short D_800E2796;
extern short D_800F33F0;
extern short D_800F33F2;
extern short D_800F33F4;
extern char D_800F340D;
extern char D_800F3409;
extern char D_800F340A;
extern short D_800E27B8;
extern short D_800E27BA;
extern short D_800E27BC;
extern short D_800E2778;
extern short D_800E277A;
extern short D_800E277C;
extern char D_800E2790;
extern char D_800E2791;
extern char D_800E2792;
extern char D_800E0F6C;

int func_800C22F8();

int func_800CD728(void) {

    *(int *)func_800C22F8() = (int)&D_800E0F6C;
    D_800F33F0 = 0;
    D_800F33F2 = 0;
    D_800F33F4 = 0;
    D_800F33F8 = 0x5F4;
    D_800F33FC = 0x5F4;
    D_800F3400 = 0x5F4;
    D_800F340C = 0x40;
    {
        register int z asm("$4");
        z = 0x20;
        D_800F340D = z;
        D_800F3409 = z;
        D_800F340A = z;
    }
    D_800F340E = -0x64;
    D_800F3408 = 0x15;
    D_800E27D4 = 0x46;
    D_800E27D5 = 0x30;
    D_800E27D6 = -0x6E;
    D_800E27D0 = 0xFF;
    D_800E27D1 = 0xB0;
    D_800E27D2 = 0xB0;
    D_800E2780 = 0xA4;
    D_800E2784 = 0xA4;
    D_800E2788 = 0x108;
    D_800E2794 = 0x6E;
    D_800E2795 = 3;
    D_800E2796 = -0x96;
    D_800E27B8 = 0;
    D_800E27BA = 0;
    D_800E27BC = 0;
    D_800E2778 = 0;
    D_800E277A = 0;
    D_800E277C = 0;
    D_800E2790 = 0x80;
    D_800E2791 = 0x80;
    D_800E2792 = 0x80;
    return 0;
}
