void func_800CCA40(int a0, unsigned char *a1, unsigned char *a2) {
    *(unsigned char *)(a2 + 3) -= 2;
    *(unsigned short *)(a2 + 4) += 0x28;
    if (*(signed char *)(a2 + 3) < 0x1E)
        a1[1] = 2;
}
