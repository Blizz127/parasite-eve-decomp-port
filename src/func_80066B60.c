extern unsigned short D_800BCFE8;
extern unsigned short D_800BCFEA;
extern unsigned short D_800BCFEC;
extern unsigned char D_800BCFEE;
extern unsigned char D_800BCFEF;
extern unsigned short D_800BCFF0;
extern unsigned short D_800BCFF2;
extern unsigned short D_800BCFF4;
extern unsigned short D_800BCFF6;
extern unsigned short D_800BCFF8;

int func_80066B60(int a0)
{
    unsigned short x;
    unsigned short y;
    unsigned short z;

    x = D_800BCFE8;
    y = D_800BCFEA;
    z = D_800BCFEC;
    D_800BCFE8 = 0xFF;
    D_800BCFEA = 0xFF;
    D_800BCFEC = 0xFF;
    D_800BCFEE = 2;
    D_800BCFEF = 2;
    D_800BCFF6 = a0;
    D_800BCFF8 = 0;
    D_800BCFF0 = x;
    D_800BCFF2 = y;
    D_800BCFF4 = z;
    return 0;
}
