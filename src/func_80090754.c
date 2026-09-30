void func_80090754(unsigned char *a0) {
    unsigned char *p;
    int n;
    unsigned char *base;

    p = *(unsigned char **)a0;
    *(unsigned char **)a0 = p + 1;
    n = *p;
    if (n == 0) {
        n = 0x100;
    }
    base = (unsigned char *)(*(unsigned short *)(a0 + 0xCE) * 2 + (unsigned int)a0);
    *(unsigned short *)(base + 0x62) = *(unsigned short *)(base + 0x62) + 1;
    if (*(unsigned short *)(base + 0x62) != n) {
        *(unsigned char **)a0 =
            *(unsigned char **)(*(unsigned short *)(a0 + 0xCE) * 4 + (unsigned int)a0 + 4);
    } else {
        *(unsigned short *)(a0 + 0xCE) = (*(unsigned short *)(a0 + 0xCE) - 1) & 3;
    }
}
