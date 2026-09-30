extern int D_8009CDDC;

void func_8003CCB0(unsigned char *a0, int a1) {
    register int saved asm("$11");
    unsigned char *cursor;
    unsigned char *base;
    int db;
    int i;
    int v;
    unsigned char *p;
    int s2;

    saved = a1;
    db = D_8009CDDC;
    if (*(unsigned char **)a0 == 0) {
        return;
    }
    if (*(short *)(a0 + 0xBA) == 0) {
        return;
    }
    asm volatile("" : "=r"(s2) : "0"(saved));
    cursor = *(unsigned char **)(a0 + 0x10);
    base = *(unsigned char **)(a0 + 0x54);
    for (i = 0; i < *(unsigned short *)(*(unsigned char **)a0 + 8);) {
        if (cursor[3] == 0xB) {
            v = 1;
        } else {
            v = (short)a1;
        }
        p = base + db * 0x34;
        if (v) {
            p[7] |= 2;
        } else {
            p[7] &= ~2;
        }
        base += 0x68;
        i++;
        cursor += 0xC;
    }
    for (i = 0; i < *(unsigned short *)(*(unsigned char **)a0 + 0xA);) {
        if (cursor[3] == 0x10) {
            v = 1;
        } else {
            v = (short)s2;
        }
        p = base + db * 0x28;
        if (v) {
            p[7] |= 2;
        } else {
            p[7] &= ~2;
        }
        base += 0x50;
        i++;
        cursor += 0xC;
    }
    for (i = 0; i < *(unsigned short *)(*(unsigned char **)a0 + 0xC);) {
        if (cursor[3] == 0x15) {
            v = 1;
        } else {
            v = (short)s2;
        }
        p = base + db * 0x24;
        if (v) {
            p[7] |= 2;
        } else {
            p[7] &= ~2;
        }
        base += 0x48;
        i++;
        cursor += 0xC;
    }
    for (i = 0; i < *(unsigned short *)(*(unsigned char **)a0 + 0xE);) {
        if (cursor[3] == 0x1A) {
            v = 1;
        } else {
            v = (short)s2;
        }
        p = base + db * 0x1C;
        if (v) {
            p[7] |= 2;
        } else {
            p[7] &= ~2;
        }
        base += 0x38;
        i++;
        cursor += 0xC;
    }
    asm volatile("" : : "r"(s2));
}
