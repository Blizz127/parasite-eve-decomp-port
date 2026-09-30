void func_800CBBF0(int a0, unsigned char *a1, unsigned char *a2) {
    *(unsigned short *)(a2 + 4) -= 8;
    *(unsigned short *)(a2 + 6) += 0x3C;
    if (*(short *)(a2 + 4) < 0x14) {
        *(unsigned short *)(a2 + 4) = 0;
        a1[1] = 2;
    }
}
