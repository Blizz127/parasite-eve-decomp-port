extern unsigned char D_800BCFEE;

int func_80018F54(void) {
    unsigned char *p = &D_800BCFEE;

    *p &= ~0x40;
    return 1;
}
