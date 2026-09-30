extern short *D_8009D048;
extern int D_8009D050;
extern unsigned char *func_8005DB44(int);
extern int func_80053968(int);
extern int func_80053B48();

int func_80053D2C(int id)
{
    short *p;
    int t;
    int slot;
    register int r asm("$17");
    register unsigned char *rec asm("$4");

    r = 0;
    p = D_8009D048;
    while (p < D_8009D048 + D_8009D050) {
        if (*p == 0) {
            break;
        }
        p++;
    }
    if (p < D_8009D048 + D_8009D050) {
        t = p - D_8009D048;
    } else {
        t = -1;
    }
    slot = t;
    if (id < 0x100) {
        rec = func_8005DB44(id - 1);
        if (rec == 0) {
            return r;
        }
        switch (rec[6]) {
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
            r = func_80053968(id) == 0;
            break;
        case 10:
        case 12:
        case 13:
        case 14:
        case 15:
            if (slot < 0) {
                r = 1;
            } else {
                D_8009D048[slot] = id;
            }
            break;
        case 16:
        case 17:
        case 18:
            r = func_80053B48();
            break;
        }
    } else {
        if (slot >= 0) {
            D_8009D048[slot] = id;
        } else {
            r = 1;
        }
    }
    return r;
}
