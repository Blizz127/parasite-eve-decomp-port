typedef struct {
    unsigned char active;      /* 0x00 */
    unsigned char pad1[7];
    unsigned char mode;        /* 0x08 */
    unsigned char b9;          /* 0x09 */
    unsigned char pad2[2];
    unsigned int lo : 20;      /* 0x0C flags */
    unsigned int f20 : 1;
    unsigned int f21 : 1;
    unsigned int hi : 10;
    short x;                   /* 0x10 */
    short r[4];                /* 0x12 */
    unsigned char dig[5][6];   /* 0x1A: 5 digits + count at [5] */
} Slot;                        /* 0x38 */

extern Slot D_800BCEA8[];
extern short D_8009CE98[4];
extern unsigned char D_8009CEA0;
extern signed char D_8009CEA4;
extern unsigned char D_8009CED0;

void func_800375E0(short x0, int mode, unsigned short *nums) {
    short x = x0;
    unsigned char i;
    unsigned char j;
    unsigned char k;
    unsigned short v;
    unsigned short q;
    short t;
    unsigned char *p;

    for (i = 0; i < 4; i++) {
        if (D_800BCEA8[i].active == 0) {
            D_800BCEA8[i].active = 1;
            D_800BCEA8[i].b9 = 0;
            D_800BCEA8[i].x = x;
            D_8009CEA0 = 0;
            D_8009CEA4 = -1;
            D_800BCEA8[i].mode = mode;
            D_800BCEA8[i].f20 = 0;
            D_800BCEA8[i].f21 = 0;
            if ((unsigned char)mode != 0) {
                D_800BCEA8[i].r[0] = D_8009CE98[0];
                D_800BCEA8[i].r[1] = D_8009CE98[1];
                D_800BCEA8[i].r[2] = D_8009CE98[2];
                D_800BCEA8[i].r[3] = D_8009CE98[3];
                if (D_800BCEA8[i].mode == 3) {
                    D_800BCEA8[i].f20 = 1;
                }
            } else if (D_8009CED0 != 0) {
                D_800BCEA8[i].b9 = 1;
            }
            for (j = 0; j < 5; j++) {
                v = *nums++;
                if ((short)v == -1) {
                    return;
                }
                k = 0;
                while (t = (short)v / 10, p = D_800BCEA8[0].dig[0] + (j * 6 + i * 56), p[k] = v - (q = t) * 10, v = q, (short)t != 0) {
                    k++;
                }
                D_800BCEA8[i].dig[j][5] = k + 1;
            }
            return;
        }
    }
}
