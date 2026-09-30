struct S {
    int w0;
    char pad04[13];
    char b11;
    char b12;
    char pad13[205];
    char bE0;
    char bE1;
    char padE2[20];
    char bF6;
    char bF7;
    short hF8;
    short hFA;
    char padFC[40];
    int w124;
    int w128;
    int w12C;
    char pad130[28];
    int w14C;
    int w150;
    char pad154[68];
    int a198[10];
    int b1C0[10][48];
    int c940[1];
    int d944[1];
    int e948[1];
    int f94C[1];
    int g950[2];
    int h958[1];
};

extern struct S D_800B0CD8;
extern unsigned char D_80094488[];
extern char D_8009448C[];
extern int func_8006E498(int a0, int a1);

void func_8006B35C(void)
{
    struct S *b;
    int i;
    int j;
    int k;
    unsigned char *p;
    int v;
    int x;

    b = &D_800B0CD8;
    for (i = 9; i >= 0; i--) {
        b->a198[i] = 0;
    }
    for (i = 0; i < 10; i++) {
        for (j = 47; j >= 0; j--) {
            b->b1C0[i][j] = 0;
        }
    }
    for (i = 0; i < 1; i++) {
        b->c940[i] = 0;
    }
    for (i = 0; i < 1; i++) {
        b->d944[i] = 0;
    }
    for (i = 0; i < 1; i++) {
        b->e948[i] = 0;
    }
    for (i = 0; i < 1; i++) {
        b->f94C[i] = 0;
    }
    for (i = 1; i >= 0; i--) {
        b->g950[i] = 0;
    }
    for (i = 0; i < 1; i++) {
        b->h958[i] = 0;
    }
    v = b->w150;
    b->w128 = v;
    b->w12C = v + 0x1400;
    b->w124 = func_8006E498(b->w14C, 0x5EAF6804);
    x = b->w0;
    b->bE0 = 0x27;
    b->bE1 = 0xD;
    b->b11 = 0;
    b->b12 = 0;
    b->w0 = x & 0xFFBFFFFF;
    p = D_80094488;
    for (k = 0; k < 0x20; k += 8) {
        *(short *)(p + 6) = 0;
        *(short *)(D_8009448C + k) = 0;
        p += 8;
    }
    b->bF6 = 0x30;
    b->bF7 = 0x7F;
    b->hF8 = 0x100;
    b->hFA = 0x800;
}
