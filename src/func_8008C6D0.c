extern unsigned char D_800B8BB4[];
extern unsigned int D_8009D2B8;

void func_8008C6D0(unsigned char *a0) {
    unsigned int i = 0;
    unsigned char *p = D_800B8BB4;

    D_8009D2B8 = *(unsigned int *)(a0 + 4);
    do {
        *(unsigned int *)p |= 3;
        i++;
        p += 0x11C;
    } while (i < 0x18);
}
