typedef struct {
    unsigned char pad0[6];
    unsigned char b6;
    unsigned char pad7[3];
    unsigned short fA;
} Item;

extern int D_8009CF3C;
extern signed char D_800C0E20[];
extern unsigned short D_800C0E06[];
extern unsigned short D_800C0E08[];
extern int func_800515F8(int *);
extern unsigned char *func_8005DB44(int);
extern int func_8005B89C(void);
extern int func_800579D4(int, int);
extern int func_800524D0(void);
extern Item *func_8005332C(int);
extern int func_80056C14(int);

int func_8004324C(int id)
{
    int max;
    int cur;
    int bits;
    int ok;
    int f;
    int k;
    Item *q;

    cur = func_800515F8(&max);
    bits = func_8005DB44(id + 0xEB)[5] >> func_8005B89C();
    if (id == 5) {
        ok = cur >= max / 3 && (bits & 1);
    } else {
        switch (id) {
        default:
            ok = (cur >= func_800579D4(id, 0)) ? bits & 1 : 0;
            break;
        case 0x13:
            ok = cur >= max && (bits & 1);
            break;
        case 6:
            ok = cur > 0 && (bits & 1);
            break;
        }
    }
    f = 0;
    if (D_8009CF3C == 0 || ((unsigned int)(id - 6) >= 3 && id != 10 && id != 0x13)) {
        f = 1;
    }
    ok &= f;
    if (id == 5) {
        if (func_800524D0() != 0) {
            ok = 0;
        }
    }
    if (id == 6) {
        q = func_8005332C(D_800C0E20[0]);
        if (q->b6 == 8) {
            ok = 0;
        } else {
            if (q->b6 != 0 && q->b6 < 8) {
                k = q->b6 - 4;
                if (k <= 0) {
                    k = 1;
                }
            } else {
                k = (q->b6 >= 19) ? q->b6 - 18 : 0;
            }
            if (k == 1 || k == 3) {
                ok = (q->fA + func_80056C14(k - 1) > 0) ? ok & 1 : 0;
            } else {
                ok = 0;
            }
        }
    }
    if (id < 3 || id == 0x12) {
        if (D_800C0E08[0] >= D_800C0E06[0]) {
            ok = 0;
        }
    }
    return ok;
}
