/* VRAM 0x80042D40 / file 0x33540 / size 0x190. */
extern int D_8009CED8;
extern int D_8009CEDC;
extern int D_8009CEE0;
extern int D_8009CEE4;
extern int D_8009CEE8;
extern int D_8009CEEC;
extern unsigned char D_800A1878[];
extern unsigned short *D_800B0E50[];
extern unsigned short *D_800B0E54[];
extern unsigned char D_800BCE3F[];
extern unsigned char D_800BCE3E[];
extern unsigned char D_800BCE3D[];
extern unsigned char D_800BCDE3[];
extern unsigned char D_800BCDE2[];
extern unsigned char D_800BCDE1[];

void func_80042D40(void) {
    int t;
    int m;
    int w;
    int i;
    unsigned short *src;
    register unsigned short *dst asm("$8");
    int v;
    int r;
    int g;
    int b;
    int x;

    D_8009CEE8 += D_8009CEE4;
    if (D_8009CEE8 < 0) {
        D_8009CEE8 = 0;
        D_8009CEE4 = 0;
        D_8009CED8 = 0;
    } else if (D_8009CEE8 >= D_8009CEE0) {
        D_8009CEE8 = D_8009CEE0 - 1;
        D_8009CEE4 = 0;
        D_8009CED8 = 7;
    }
    t = D_800A1878[D_8009CEE8];
    w = (t * D_8009CEEC) << 5;
    m = t * (D_8009CEEC + 0x100);
    src = D_800B0E50[0];
    dst = D_800B0E54[0];
    for (i = 0; i < D_8009CEDC << 8; i++) {
        v = *src++;
        if (v != 0) {
            r = v & 0x1F;
            g = (v >> 5) & 0x1F;
            b = (v >> 10) & 0x1F;
            v &= 0x8000;
            r = (w + (r << 16) - m * r) >> 16;
            g = (w + (g << 16) - m * g) >> 16;
            b = (w + (b << 16) - m * b) >> 16;
            v = v | r | (g << 5) | (b << 10);
            *dst = v;
        } else {
            *dst = 0;
        }
        dst++;
    }
    x = (unsigned int)w >> 13;
    D_800BCE3F[0] = x;
    D_800BCE3E[0] = x;
    D_800BCE3D[0] = x;
    D_800BCDE3[0] = x;
    D_800BCDE2[0] = x;
    D_800BCDE1[0] = x;
}
