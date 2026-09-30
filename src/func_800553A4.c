extern unsigned short D_800A1E6E[];
extern unsigned short D_800A1E4E[];
extern unsigned char D_800A1B90[];
extern unsigned char D_800A1B70[];
extern unsigned char D_800A1D79[];
extern int D_8009D038;

void func_800553A4(short *a0, short *a1)
{
    int buf[4];
    int *q;
    int cnt;
    int i;
    int off;
    register int k1 asm("$6");
    register unsigned char *dst1 asm("$7");
    register int k2 asm("$4");
    register unsigned char *dst2 asm("$6");
    unsigned short *a3;
    unsigned int u;
    int h;
    int v;
    int bb;

    if (a0 == 0) {
        return;
    }
    if (a1 == 0) {
        return;
    }
    cnt = 0;
    if (*a0 == 0) {
        i = 0;
        off = 0;
        q = buf;
        do {
            if (*(unsigned short *)((char *)D_800A1E6E + off) != 0) {
                *q = i;
                q++;
                cnt++;
            }
            i++;
            off += 0x20;
        } while (i < 3);
        if (cnt != 0) {
            D_8009D038 = D_8009D038 + 1;
            if (D_8009D038 >= 0x209) {
                k1 = 0;
                dst1 = D_800A1B90;
                do {
                        *dst1 = *dst1 ^ D_800A1D79[k1];
                        k1++;
                        dst1++;
                } while (k1 < 0x20);
                k1 = 0x20;
                dst1 = D_800A1B90 + 0x20;
                do {
                        *dst1 = *dst1 ^ D_800A1B70[k1];
                        k1++;
                        dst1++;
                } while (k1 < 0x209);
                D_8009D038 = 0;
            }
            *a0 = ((cnt * D_800A1B90[D_8009D038]) >> 8) + 1;
        }
    }
    u = *(unsigned short *)a0;
    if ((unsigned int)(u - 1) < 3) {
        a3 = (unsigned short *)((char *)D_800A1E4E + (int)(short)u * 0x20);
        h = *a1;
        if (h == 0) {
            D_8009D038 = D_8009D038 + 1;
            if (D_8009D038 >= 0x209) {
                k2 = 0;
                dst2 = D_800A1B90;
                do {
                        *dst2 = *dst2 ^ D_800A1D79[k2];
                        k2++;
                        dst2++;
                } while (k2 < 0x20);
                k2 = 0x20;
                dst2 = D_800A1B90 + 0x20;
                do {
                        *dst2 = *dst2 ^ D_800A1B70[k2];
                        k2++;
                        dst2++;
                } while (k2 < 0x209);
                D_8009D038 = 0;
            }
            bb = D_800A1B90[D_8009D038];
            v = (*a3 * bb) >> 8;
            *a1 = v;
        } else if (h < 0) {
            v = (*a3 * -h) / 100;
            *a1 = v;
        }
        if (*a1 > *a3) {
            *a1 = *a3;
        }
        *a3 = *a3 - *(unsigned short *)a1;
    }
}
