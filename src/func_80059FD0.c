extern short *D_8009D048;
extern int D_8009D050;
extern unsigned char *D_8009D058;
extern int D_8009D064;
extern short *D_8009D04C;
extern int D_8009D054;
extern int D_8009D090;
extern int D_8009D094;
extern int D_8009D098;
extern int D_8009D09C;
extern unsigned char D_8009D05C[];
extern unsigned char D_800C0E48[];
extern unsigned char D_800A1F84[];
extern unsigned char D_800A204C[];
extern unsigned char D_800A206C[];
extern unsigned char D_800BEEAC[];
extern unsigned char D_8009DE64[];
extern unsigned char *func_8005DB44(int);
extern int func_80052F70(void);

typedef struct { unsigned char b[32]; } Rec;

void func_80059FD0(void)
{
    unsigned char *p;
    unsigned char *res;
    int i;
    int v;
    register int w asm("$5");
    register int m asm("$3");

    if (D_8009D098 != 0 && D_8009D04C != 0) {
        D_8009D048 = D_8009D04C;
        D_8009D058 = D_800A1F84;
        D_8009D064 = 4;
        D_8009D050 = D_8009D054;
    } else {
        D_8009D048 = (short *)D_800C0E48;
        D_8009D050 = func_80052F70();
        D_8009D058 = D_8009D05C;
        D_8009D064 = 2;
    }
    i = D_8009D090;
    if (i >= 0 && i < D_8009D050) {
        v = D_8009D048[i];
        w = v;
        if ((unsigned int)(v - 0x100) < 0x80) {
            res = (unsigned char *)((v << 5) + (int)D_800BEEAC);
            goto e1;
        }
        if ((unsigned int)(v - 1) < 0xFF) {
            res = func_8005DB44(v - 1);
            goto e1;
        }
        if ((unsigned int)(w - 0x200) < 9) {
            m = w << 5;
            res = (unsigned char *)(m + (int)D_8009DE64);
            goto e1;
        }
        res = 0;
    e1:
        p = res;
    } else {
        p = 0;
    }
    *(Rec *)p = *(Rec *)D_800A204C;
    if (D_8009D09C != 0 && D_8009D04C != 0) {
        D_8009D048 = D_8009D04C;
        D_8009D058 = D_800A1F84;
        D_8009D064 = 4;
        D_8009D050 = D_8009D054;
    } else {
        D_8009D048 = (short *)D_800C0E48;
        D_8009D050 = func_80052F70();
        D_8009D058 = D_8009D05C;
        D_8009D064 = 2;
    }
    i = D_8009D094;
    if (i >= 0 && i < D_8009D050) {
        v = D_8009D048[i];
        w = v;
        if ((unsigned int)(v - 0x100) < 0x80) {
            res = (unsigned char *)((v << 5) + (int)D_800BEEAC);
            goto e2;
        }
        if ((unsigned int)(v - 1) < 0xFF) {
            res = func_8005DB44(v - 1);
            goto e2;
        }
        if ((unsigned int)(w - 0x200) < 9) {
            m = w << 5;
            res = (unsigned char *)(m + (int)D_8009DE64);
            goto e2;
        }
        res = 0;
    e2:
        p = res;
    } else {
        p = 0;
    }
    *(Rec *)p = *(Rec *)D_800A206C;
}
