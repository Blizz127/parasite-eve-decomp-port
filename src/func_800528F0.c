extern unsigned int D_800A76A4[];
extern unsigned char D_800A1B90[];
extern unsigned char D_800A1B70[];
extern unsigned char D_800A1D79[];
extern int D_8009D038;

void func_800528F0(void)
{
    unsigned int buf[0x209];
    unsigned int s;
    unsigned int m;
    int i;
    int j;
    register int k asm("$4");
    register unsigned char *dst asm("$5");

    s = D_800A76A4[0] / 60;
    m = 0;
    for (i = 0; i < 0x11; i++) {
        for (j = 0x1F; j >= 0; j--) {
            s = s * 0x5D588B65 + 1;
            m = (m >> 1) | (s & 0x80000000);
        }
        buf[i] = m;
    }
    buf[0x10] = (buf[0x10] << 23) ^ (buf[0] >> 9) ^ buf[0x0F];
    for (i = 0x11; i < 0x209; i++) {
        buf[i] = (buf[i - 0x11] << 23) ^ (buf[i - 0x10] >> 9) ^ buf[i - 1];
    }
    for (i = 0; i < 0x209; i++) {
        D_800A1B90[i] = buf[i];
    }
        k = 0;
        dst = D_800A1B90;
        do {
            *dst = *dst ^ D_800A1D79[k];
            k++;
            dst++;
        } while (k < 0x20);
        k = 0x20;
        dst = D_800A1B90 + 0x20;
        do {
            *dst = *dst ^ D_800A1B70[k];
            k++;
            dst++;
        } while (k < 0x209);
        k = 0;
        dst = D_800A1B90;
        do {
            *dst = *dst ^ D_800A1D79[k];
            k++;
            dst++;
        } while (k < 0x20);
        k = 0x20;
        dst = D_800A1B90 + 0x20;
        do {
            *dst = *dst ^ D_800A1B70[k];
            k++;
            dst++;
        } while (k < 0x209);
        k = 0;
        dst = D_800A1B90;
        do {
            *dst = *dst ^ D_800A1D79[k];
            k++;
            dst++;
        } while (k < 0x20);
        k = 0x20;
        dst = D_800A1B90 + 0x20;
        do {
            *dst = *dst ^ D_800A1B70[k];
            k++;
            dst++;
        } while (k < 0x209);
    D_8009D038 = 0x208;
}
