void func_8009071C(unsigned char *a0) {
    unsigned short h = *(unsigned short *)(a0 + 0xCE);
    unsigned int v = *(unsigned int *)a0;
    unsigned int t;
    unsigned short h2;
    h = (h + 1) & 3;
    *(unsigned short *)(a0 + 0xCE) = h;
    *(unsigned int *)(a0 + 4 + (h << 2)) = v;
    h2 = *(unsigned short *)(a0 + 0xCE);
    t = (h2 << 1) + (unsigned int)a0;
    *(unsigned short *)(t + 0x62) = 0;
}
