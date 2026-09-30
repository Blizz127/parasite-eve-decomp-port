void func_800CBC68(int a0, unsigned char *a1, unsigned char *a2) {
    *(unsigned short *)(a2 + 4) -= 8;
    *(unsigned short *)(a2 + 6) += 0xA;
    if (*(short *)(a2 + 4) < 0x14) {
        *(unsigned short *)(a2 + 4) = 0;
        a1[1] = 2;
    }
}
