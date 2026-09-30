extern char D_800E0FFC;
extern int D_800E22B8;
extern int D_800E22BC;
extern int D_800E22C0;
extern unsigned char D_800E22CD;
extern short D_800E22CE;
extern short D_800E22B0;
extern short D_800E22B2;
extern short D_800E22B4;
extern unsigned char D_800E22C8;
extern unsigned char D_800E22C9;
extern unsigned char D_800E22CA;
extern int *func_800C22F8(void);

int func_800CE084(void)
{
    int *p;

    p = func_800C22F8();
    *p = (int)&D_800E0FFC;
    D_800E22B8 = 0x300;
    D_800E22BC = 0x300;
    D_800E22C0 = 0x300;
    D_800E22CD = 5;
    D_800E22CE = -0x64;
    D_800E22B0 = 0;
    D_800E22B2 = 0;
    D_800E22B4 = 0;
    D_800E22C8 = 0x80;
    D_800E22C9 = 0x80;
    D_800E22CA = 0x80;
    return 0;
}
