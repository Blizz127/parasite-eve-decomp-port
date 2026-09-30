/* VRAM 0x800C22F8 / file 0xB2AF8 / size 0x11C. */
extern int D_800E2248;
extern unsigned char *D_800F34F4;
extern int D_800F32A8;
extern int D_800F3330;

int func_800C6CE0();

int func_800C22F8(int slot) {
    unsigned char *p;
    unsigned char *e;
    unsigned int k;
    int o;
    int *q;

    p = (unsigned char *)(slot + 0xC);
    e = (unsigned char *)(slot + 0xA0C);
    do {
        *p = 0;
        p++;
    } while (p < e);
    *(unsigned char *)(slot + 2) = 0;
    *(unsigned char *)(slot + 3) = 0;
    D_800E2248 = slot + 0xC;
    D_800F34F4 = (unsigned char *)(slot + 0x80);
    D_800F32A8 = slot;
    D_800F3330 = slot + 0x200;
    *(short *)(slot + 0xC) = 0;
    *(short *)(slot + 0xE) = 0;
    *(short *)(slot + 0x10) = 0;
    k = 0;
    o = 0;
    for (; k < 0x40; k++) {
        *(unsigned char *)(o + (int)D_800F34F4) = 0;
        *(unsigned char *)(o + (int)D_800F34F4 + 1) = 0;
        *(short *)(o + (int)D_800F34F4 + 2) = 0;
        *(short *)(o + (int)D_800F34F4 + 4) = 0;
        o += 6;
    }
    if (func_800C6CE0(slot) == 3) {
        q = *(int **)(*(int **)(slot + 8));
        *q = (*q & 0xC0FFFFFF) | 0x1000000;
    }
    return D_800E2248 + 0x6C;
}
