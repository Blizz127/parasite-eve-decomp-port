typedef struct Blk {
    char b[32];
} Blk;

extern int D_8009D03C;
extern int D_8009D04C;
extern short D_800C0E48[];
extern short D_800C1EB8[];
extern short D_800C1F80[];
extern Blk D_800A1E64[];
extern unsigned char D_800A1E6D[];
extern short D_800A1E76[];
extern void func_80052E30(int);
extern unsigned char *func_8005DB44(int);

void func_80052C6C(void)
{
    int i;
    int j;
    int k;
    int off;
    Blk *dst;
    unsigned char *p;

    func_80052E30(0);
    for (i = 0x31; i >= 0; i--) {
        D_800C0E48[i] = 0;
    }
    i = 0;
    do {
        p = func_8005DB44(i++);
    } while (p != 0 && p[6] != 0x13);
    D_8009D03C = i;
    for (j = 0; j < 9; j++) {
        p = func_8005DB44(D_8009D03C + (j % 3) - 1);
        dst = &D_800A1E64[j];
        off = j * 0x20;
        *dst = *(Blk *)p;
        *(unsigned char *)((char *)D_800A1E6D + off) = 0;
        *(short *)((char *)D_800A1E76 + off) = 0x3E7;
    }
    for (i = 0x63; i >= 0; i--) {
        D_800C1EB8[i] = 0;
    }
    for (i = 0x51; i >= 0; i--) {
        D_800C1F80[i] = 0;
    }
    D_8009D04C = 0;
}
