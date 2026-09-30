extern unsigned char *volatile D_800B1624;
extern unsigned char D_800BCFFD;
extern unsigned short D_800BCFAC;
extern unsigned short D_800BCFAE;
extern unsigned short D_800BCFB0;
extern unsigned short D_800BCFB2;

int func_80065DCC(void)
{
    unsigned char *p = D_800B1624;
    unsigned char *q = D_800B1624;

    q += *(unsigned int *)(p + 0x1C);
    q += D_800BCFFD * 52;
    *(unsigned short *)(q + 0x2C) = D_800BCFAC;
    *(unsigned short *)(q + 0x2E) = D_800BCFAE;
    *(unsigned short *)(q + 0x30) = D_800BCFB0;
    *(unsigned short *)(q + 0x32) = D_800BCFB2;
    return 0;
}
