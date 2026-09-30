typedef struct ItemDef {
    unsigned char pad0[6];
    unsigned char b6;
} ItemDef;

extern short *D_8009D048;
extern int D_8009D050;
extern unsigned int *D_8009D058;
extern int D_8009D064;
extern unsigned char D_800C0E48[];
extern unsigned int D_8009D05C[];
extern short D_800A1FD4[];
extern signed char D_800C0E20;
extern signed char D_800C0E22;
extern int func_80052F70(void);
extern int func_80057654(int);
extern ItemDef *func_8005DB44(int);
extern int func_8005833C(int);
extern void func_80055760(void);

static inline void swap16(short *p, short *q)
{
    *p ^= *q;
    *q ^= *p;
    *p ^= *q;
}

int func_80058030(int a, int i, int b, int j)
{
    int r;
    int busy;
    short *p;
    short *st;

    r = 1;
    D_8009D048 = (short *)D_800C0E48;
    D_8009D050 = func_80052F70();
    D_8009D058 = D_8009D05C;
    D_8009D064 = 2;
    if (a == 13 && b == a) {
        st = D_800A1FD4;
        swap16(&st[i], &st[j]);
    } else if (a == 14 && b == a) {
        swap16(&D_8009D048[i], &D_8009D048[j]);
        if (D_800C0E20 == i) {
            D_800C0E20 = j;
        } else if (D_800C0E20 == j) {
            D_800C0E20 = i;
        }
        if (D_800C0E22 == i) {
            D_800C0E22 = j;
        } else if (D_800C0E22 == j) {
            D_800C0E22 = i;
        }
    } else if (a == 13) {
        if (func_80057654(j) == 0) {
            goto fail;
        }
        busy = D_8009D048 == (short *)D_800C0E48 && (D_800C0E20 == j || D_800C0E22 == j);
        if (busy) {
            goto fail;
        }
        {
            register short *bs asm("$3") = D_800A1FD4;
            p = &bs[i];
        }
        if (func_8005DB44(*p - 1)->b6 >= 16 && func_8005DB44(*p - 1)->b6 < 19) {
            r = func_8005833C(i) == 0;
        } else {
            st = D_800A1FD4;
            swap16(&st[i], &D_8009D048[j]);
        }
    } else {
        if (func_80057654(i) == 0) {
            goto fail;
        }
        busy = D_8009D048 == (short *)D_800C0E48 && (D_800C0E20 == i || D_800C0E22 == i);
        if (busy) {
            goto fail;
        }
        swap16(&D_8009D048[i], &D_800A1FD4[j]);
    }
    func_80055760();
    goto out;
fail:
    r = 0;
out:
    return r;
}
