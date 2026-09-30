void func_800CCA78(int a0, unsigned char *a1, unsigned char *a2) {
    *(unsigned char *)(a2 + 3) -= 6;
    *(unsigned short *)(a2 + 4) += 0xB4;
    if (*(signed char *)(a2 + 3) < 0x1E)
        a1[1] = 2;
}
