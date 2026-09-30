extern short *D_8009D048;
extern int D_8009D050;
extern unsigned int *D_8009D058;
extern int D_8009D064;
extern short *D_8009D04C;
extern int D_8009D054;
extern unsigned char D_8009D05C[];
extern unsigned char D_800C0E48[];
extern unsigned char D_800A1F84[];
extern int func_80052F70(void);

void func_8005600C(int *a0, int *a1)
{
    int unused[4];
    int s2;
    int i;
    int n;
    int nt;
    register int one asm("$7");
    int t;
    register unsigned int *bits asm("$6");

    s2 = (D_8009D048 != (short *)D_800C0E48);
    if (*a0 != 0 && D_8009D04C != 0) {
        D_8009D048 = D_8009D04C;
        D_8009D058 = (unsigned int *)D_800A1F84;
        D_8009D064 = 4;
        D_8009D050 = D_8009D054;
    } else {
        D_8009D048 = (short *)D_800C0E48;
        D_8009D050 = func_80052F70();
        D_8009D058 = (unsigned int *)D_8009D05C;
        D_8009D064 = 2;
    }
    nt = D_8009D050;
    if (nt > 0) {
        i = 0;
        bits = D_8009D058;
        one = 1;
        n = nt;
        do {
            if ((bits[i >> 5] & (one << (i & 0x1F))) != 0) {
                if (i != *a1) {
                    goto f1;
                }
            }
            i++;
        } while (i < n);
    f1:
        if (i < D_8009D050) {
            *a1 = i;
            goto out;
        }
    }
    if (*a0 == 0 && D_8009D04C != 0) {
        D_8009D048 = D_8009D04C;
        D_8009D058 = (unsigned int *)D_800A1F84;
        D_8009D064 = 4;
        D_8009D050 = D_8009D054;
    } else {
        D_8009D048 = (short *)D_800C0E48;
        D_8009D050 = func_80052F70();
        D_8009D058 = (unsigned int *)D_8009D05C;
        D_8009D064 = 2;
    }
    n = D_8009D050;
    if (n > 0) {
        i = 0;
        bits = D_8009D058;
        one = 1;
        do {
            if ((bits[i >> 5] & (one << (i & 0x1F))) != 0) {
                goto f2;
            }
            i++;
        } while (i < n);
    f2:
        if (i < D_8009D050) {
            *a0 = (*a0 == 0);
            *a1 = i;
            goto out;
        }
        t = -1;
    } else {
        t = -1;
    }
    *a0 = t;
out:
    if (s2 != 0 && D_8009D04C != 0) {
        D_8009D048 = D_8009D04C;
        D_8009D058 = (unsigned int *)D_800A1F84;
        D_8009D064 = 4;
        D_8009D050 = D_8009D054;
    } else {
        D_8009D048 = (short *)D_800C0E48;
        D_8009D050 = func_80052F70();
        D_8009D058 = (unsigned int *)D_8009D05C;
        D_8009D064 = 2;
    }
}
