extern short *D_8009D07C;
extern volatile int D_8009D080;
extern short *D_8009D04C;
extern int D_8009D054;
extern int D_8009D050;
extern short *D_8009D048;
extern short D_800C1F80[];
extern short D_800C1EB8[];
extern unsigned char D_800BEEAC[];
extern unsigned char D_8009DE64[];
extern unsigned char *func_8005DB44(int);

void func_800588EC(int a0)
{
    int unused[2];
    int i;
    int key;
    register int found asm("$17");
    register int n asm("$2");
    int cnt;
    int v;
    int w;
    unsigned char *p;
    short *t;
    short *base;
    register short *r1 asm("$3");
    int r2;

    if (a0 == 0) {
        goto zero;
    }
    D_8009D07C = D_800C1F80;
    i = 0;
    key = 0x204;
    t = D_800C1F80;
    while (i < 0x52 && *t != key) {
        i++;
        t++;
    }
    found = i < 0x52;
    if (found) {
        goto ret_found;
    }
    i = 0;
    cnt = D_8009D050;
    if (cnt <= 0) {
        goto ret_found;
    }
    do {
        if (i >= 0 && i < cnt) {
            v = D_8009D048[i];
            w = v;
            if ((unsigned int)(v - 0x100) < 0x80) {
                p = (unsigned char *)((v << 5) + (int)D_800BEEAC);
                goto got;
            }
            if ((unsigned int)(v - 1) < 0xFF) {
                p = func_8005DB44(v - 1);
                goto got;
            }
            if ((unsigned int)(w - 0x200) < 9) {
                p = (unsigned char *)((w << 5) + (int)D_8009DE64);
                goto got;
            }
        }
        p = 0;
    got:
        if (p != 0 && p[6] == 6) {
            break;
        }
        cnt = D_8009D050;
        i++;
    } while (i < cnt);
    if (i < D_8009D050) {
        i = 0;
        base = D_8009D07C;
        t = base;
        while (i < 0x51 && *t != 0) {
            i++;
            t++;
        }
        if (i < 0x52) {
            base[i] = 0x204;
            found = 1;
        }
    }
ret_found:
    n = found + 0x50;
    goto done;
zero:
    D_8009D07C = D_800C1EB8;
    n = 0x64;
done:
    D_8009D080 = n;
    asm volatile("" ::: "memory");
    r1 = D_8009D07C;
    r2 = D_8009D080;
    D_8009D04C = r1;
    D_8009D054 = r2;
}
