typedef struct Rec {
    int w[9];
} Rec;

extern short *D_8009D048;
extern int D_8009D050;
extern int D_8009D028;
extern unsigned char D_800BEEAC[];
extern unsigned char D_8009DE64[];
extern unsigned char *func_8005DB44(int);
extern Rec *func_80051098(void);
extern void func_800512AC(int, int *);
extern void func_80057D30(int);
extern void func_800516B4(int);
extern void func_80055760(void);
extern void func_8004F23C(void);
extern void func_80048918(int, int, int);

void func_80057834(int i)
{
    unsigned char *p;
    unsigned char *res;
    Rec *r;
    int local;
    int v;
    int w;

    if (i >= 0 && i < D_8009D050) {
        v = D_8009D048[i];
        w = v;
        if ((unsigned int)(v - 0x100) < 0x80) {
            res = (unsigned char *)((v << 5) + (int)D_800BEEAC);
            goto done;
        }
        if ((unsigned int)(v - 1) < 0xFF) {
            res = func_8005DB44(v - 1);
            goto done;
        }
        if ((unsigned int)(w - 0x200) < 9) {
            res = (unsigned char *)((w << 5) + (int)D_8009DE64);
            goto done;
        }
        res = 0;
    done:
        p = res;
    } else {
        p = 0;
    }
    if (D_8009D028 != 0) {
        r = func_80051098();
        r->w[0] = 0;
        r->w[1] = D_8009D048[i];
        r->w[2] = i;
        local = D_8009D048[i];
        func_800512AC(0, &local);
        func_80057D30(i);
        return;
    }
    switch (p[0xE]) {
    case 1:
        func_800516B4(D_8009D048[i]);
        func_80057D30(i);
        func_80055760();
        break;
    case 2:
        func_8004F23C();
        break;
    case 4:
    case 5:
    case 6:
        func_80048918(0xFE, i, -1);
        break;
    case 12:
    case 13:
    case 14:
        func_80048918(0x200, i, -1);
        break;
    }
}
