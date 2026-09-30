typedef struct Blob {
    char b[32];
} Blob;

extern short *D_8009D048;
extern short *D_8009D04C;
extern int D_8009D050;
extern int D_8009D054;
extern void *D_8009D058;
extern int D_8009D064;
extern void *D_8009D070;
extern void *D_8009D074;

extern short D_800A1F84;
extern short D_800C0E48;
extern char D_8009D05C;
extern unsigned char D_800BEEAC[];
extern unsigned char D_8009DE64[];
extern Blob D_800A1F94;
extern Blob D_800A1FB4;
extern char D_800A1FD3;
extern char D_800A1FB3;
extern short D_800A1FC0;
extern short D_800A1FA0;

extern int func_80052F70(void);
extern unsigned char *func_8005DB44(int);

void func_80056C40(int a0, int a1, int a2, int a3)
{
    unsigned char *res;
    int v;
    register int w asm("$5");
    register int m asm("$3");

    if (a0 != 0 && D_8009D04C != 0) {
        D_8009D048 = D_8009D04C;
        D_8009D058 = &D_800A1F84;
        D_8009D064 = 4;
        D_8009D050 = D_8009D054;
    } else {
        D_8009D048 = &D_800C0E48;
        D_8009D050 = func_80052F70();
        D_8009D058 = &D_8009D05C;
        D_8009D064 = 2;
    }
    if (a1 >= 0 && a1 < D_8009D050) {
        v = D_8009D048[a1];
        w = v;
        if ((unsigned int)(v - 0x100) < 0x80) {
            res = (unsigned char *)((v << 5) + (int)D_800BEEAC);
            goto done0;
        }
        if ((unsigned int)(v - 1) < 0xFF) {
            res = func_8005DB44(v - 1);
            goto done0;
        }
        if ((unsigned int)(w - 0x200) < 9) {
            m = w << 5;
            res = (unsigned char *)(m + (int)D_8009DE64);
            goto done0;
        }
    }
    res = 0;
done0:
    D_8009D070 = res;
    if (a2 != 0 && D_8009D04C != 0) {
        D_8009D048 = D_8009D04C;
        D_8009D058 = &D_800A1F84;
        D_8009D064 = 4;
        D_8009D050 = D_8009D054;
    } else {
        D_8009D048 = &D_800C0E48;
        D_8009D050 = func_80052F70();
        D_8009D058 = &D_8009D05C;
        D_8009D064 = 2;
    }
    if (a3 >= 0 && a3 < D_8009D050) {
        v = D_8009D048[a3];
        w = v;
        if ((unsigned int)(v - 0x100) < 0x80) {
            res = (unsigned char *)((v << 5) + (int)D_800BEEAC);
            goto done1;
        }
        if ((unsigned int)(v - 1) < 0xFF) {
            res = func_8005DB44(v - 1);
            goto done1;
        }
        if ((unsigned int)(w - 0x200) < 9) {
            m = w << 5;
            res = (unsigned char *)(m + (int)D_8009DE64);
            goto done1;
        }
    }
    res = 0;
done1:
    D_8009D074 = res;
    D_800A1F94 = *(Blob *)D_8009D070;
    D_800A1FB4 = *(Blob *)D_8009D074;
    D_800A1FD3 = 0;
    D_800A1FB3 = 0;
    D_800A1FC0 = 0;
    D_800A1FA0 = 0;
}
