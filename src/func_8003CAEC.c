extern int D_8009CDDC;

void func_8003CAEC(unsigned char *a0, int a1, int a2, int a3) {
    unsigned char *q;
    register unsigned int col asm("$3");
    int i;
    int db;

    db = D_8009CDDC;
    if (*(unsigned char **)a0 == 0) {
        return;
    }
    if (*(short *)(a0 + 0xBA) == 0) {
        return;
    }
    col = ((a3 & 0xFF) << 16) | ((a2 & 0xFF) << 8) | (a1 & 0xFF);
    a0[0x90] = a1;
    a0[0x91] = a2;
    a0[0x92] = a3;
    q = *(unsigned char **)(a0 + 0x54);
    for (i = 0; i < *(unsigned short *)(*(unsigned char **)a0 + 8); i++) {
        unsigned char *p = q + db * 0x34;
        unsigned char c = p[7];
        *(unsigned int *)(p + 4) = col;
        *(unsigned int *)(p + 0x10) = col;
        *(unsigned int *)(p + 0x1C) = col;
        *(unsigned int *)(p + 0x28) = col;
        p[7] = c;
        q += 0x68;
    }
    for (i = 0; i < *(unsigned short *)(*(unsigned char **)a0 + 0xA); i++) {
        unsigned char *p = q + db * 0x28;
        unsigned char c = p[7];
        *(unsigned int *)(p + 4) = col;
        *(unsigned int *)(p + 0x10) = col;
        *(unsigned int *)(p + 0x1C) = col;
        p[7] = c;
        q += 0x50;
    }
    for (i = 0; i < *(unsigned short *)(*(unsigned char **)a0 + 0xC); i++) {
        unsigned char *p = q + db * 0x24;
        unsigned char c = p[7];
        *(unsigned int *)(p + 4) = col;
        *(unsigned int *)(p + 0xC) = col;
        *(unsigned int *)(p + 0x14) = col;
        *(unsigned int *)(p + 0x1C) = col;
        p[7] = c;
        q += 0x48;
    }
    for (i = 0; i < *(unsigned short *)(*(unsigned char **)a0 + 0xE); i++) {
        unsigned char *p = q + db * 0x1C;
        unsigned char c = p[7];
        *(unsigned int *)(p + 4) = col;
        *(unsigned int *)(p + 0xC) = col;
        *(unsigned int *)(p + 0x14) = col;
        p[7] = c;
        q += 0x38;
    }
}
