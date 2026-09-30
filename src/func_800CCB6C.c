void func_800CCB6C(int a0, unsigned char *a1, unsigned char *a2) {
    *(unsigned char *)(a2 + 3) -= 8;
    *(unsigned short *)(a2 + 4) += 0x1A4;
    if (*(signed char *)(a2 + 3) < 0x14) {
        *(unsigned char *)(a2 + 3) = 0;
        a1[1] = 2;
    }
}
