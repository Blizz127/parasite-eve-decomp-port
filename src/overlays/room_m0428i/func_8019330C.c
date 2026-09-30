/* room_m0428i — func_8019330C, blob offset 0x4324, 0x17C bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl13 2026-09-28).
 * CE560/CE610 controller: switch with empty case 2; r = D_800F33E0 before the init stores; a1[0]++ then a1[1] = 0 then n = a1[0] (store-order invalidation gives retail's lh reload) (ovl7 park 90w -> 0). */

extern unsigned char *D_800F33E0;
extern unsigned char *D_800F32D0;
extern void func_80193018();
extern int func_800CE560();
extern short *func_800CE610();
extern void func_8006DCE4();

#define P(o, x) (*(void **)((char *)(o) + (x)))

int func_8019330C(int a0, short *a1)
{
    short *e;
    unsigned char *o;
    short *d;
    unsigned char *r;
    int n;

    switch (a0) {
    case 0:
        r = D_800F33E0;
        a1[0] = 0;
        a1[1] = 0;
        return func_800CE560(P(r, 8), 8, 4, func_80193018);
    case 1:
        if (a1[0] >= 4) {
            return 2;
        }
        if (++a1[1] < 2) {
            return 0;
        }
        e = func_800CE610(P(D_800F33E0, 8));
        if (e == 0) {
            return 0;
        }
        if (a1[0] == 0) {
            o = P(D_800F32D0, 8);
            d = P(o, 0x238);
            func_8006DCE4(0x5C3, P(P(o, 0), 8), d[10], d[12], d[14]);
        }
        a1[0]++;
        a1[1] = 0;
        n = a1[0];
        if (n == 4) {
            e[0] = 0x100;
            e[1] = 8;
            ((unsigned char *)e)[6] = 1;
            ((unsigned char *)e)[7] = 1;
        } else {
            e[0] = (n << 5) + 0x40;
            e[1] = 8;
            ((unsigned char *)e)[6] = 0;
            ((unsigned char *)e)[7] = 0;
        }
        break;
    case 2:
        break;
    }
    return 0;
}
