extern short *D_8009D048;
extern int D_8009D050;
extern unsigned char *D_8009D084;
extern int D_8009D088;
extern int D_8009D08C;
extern unsigned char D_800A1FE8[];
extern unsigned char D_800A204C[];
extern unsigned char D_800A206C[];
extern unsigned char D_800BEEAC[];
extern unsigned char D_8009DE64[];
extern unsigned char *func_8005DB44(int);
extern int func_80059F08(int);

typedef struct { unsigned char b[32]; } Rec;

void func_80059C44(void)
{
    unsigned char *p;
    unsigned char *res;
    int i;
    int v;
    register int w asm("$5");
    register int m asm("$3");

    D_8009D084 = D_800A1FE8;
    D_8009D08C = 0;
    D_8009D088 = 0;
    i = func_80059F08(0);
    if (i >= 0 && i < D_8009D050) {
        v = D_8009D048[i];
        w = v;
        if ((unsigned int)(v - 0x100) < 0x80) {
            res = (unsigned char *)((v << 5) + (int)D_800BEEAC);
            goto a1;
        }
        if ((unsigned int)(v - 1) < 0xFF) {
            res = func_8005DB44(v - 1);
            goto a1;
        }
        if ((unsigned int)(w - 0x200) < 9) {
            m = w << 5;
            res = (unsigned char *)(m + (int)D_8009DE64);
            goto a1;
        }
        res = 0;
    a1:
        p = res;
    } else {
        p = 0;
    }
e1:
    *(Rec *)D_800A204C = *(Rec *)p;
    i = func_80059F08(1);
    if (i >= 0 && i < D_8009D050) {
        v = D_8009D048[i];
        w = v;
        if ((unsigned int)(v - 0x100) < 0x80) {
            res = (unsigned char *)((v << 5) + (int)D_800BEEAC);
            goto a2;
        }
        if ((unsigned int)(v - 1) < 0xFF) {
            res = func_8005DB44(v - 1);
            goto a2;
        }
        if ((unsigned int)(w - 0x200) < 9) {
            m = w << 5;
            res = (unsigned char *)(m + (int)D_8009DE64);
            goto a2;
        }
        res = 0;
    a2:
        p = res;
    } else {
        p = 0;
    }
e2:
    *(Rec *)D_800A206C = *(Rec *)p;
}
