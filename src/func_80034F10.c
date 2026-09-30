extern int D_800A77F0[];
extern int D_800B6A80[];
extern int D_800BEA90[];
extern int D_800B0CD8;
extern int D_8009D2E8;
extern int D_8009D2AC;
extern int D_8009D20C;
extern int D_8009D2F0;
extern int D_8009D254;
extern int D_8009D224;
extern short D_8009D2A6;

void func_80034F10(void) {
    int *p;
    int *r;
    int *b;
    int v;
    int m;
    unsigned int i;
    unsigned int j;
    unsigned int off;
    unsigned int base;

    D_8009D2E8 = 0;
    i = 0;
    p = D_800A77F0;
    do {
        *p = 0;
        i++;
        p++;
    } while (i < 0x200);
    i = 0;
    p = D_800B6A80;
    do {
        i++;
        *p = 0;
    } while (i < 0x40);
    i = 0;
    b = D_800BEA90;
    base = 0;
    do {
        j = 0;
        off = base;
        do {
            *(int *)(off + (int)b) = 0;
            j++;
            off += 4;
        } while (j < 0xA0);
        i++;
        base += 0x280;
    } while (i < 0xE);
    r = &D_800B0CD8;
    __asm__ __volatile__("" : "=r"(r) : "0"(r));
    v = *r;
    m = ~0x3000;
    D_8009D2AC = 0;
    D_8009D20C = 0;
    D_8009D2F0 = 0;
    D_8009D2A6 = 0;
    D_8009D254 = 0;
    D_8009D224 = 0;
    *r = v & m;
}
