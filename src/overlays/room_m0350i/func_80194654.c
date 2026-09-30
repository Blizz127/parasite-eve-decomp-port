/* room_m0350i — func_80194654, blob offset 0x566C, 0x168 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl13 2026-09-28).
 * CE560/CE610 controller: pp = D_800F32D0 read between a1[1] = 1 and a1[0]-- (fills the lhu delay; keeps store order) (park 30w -> 0). */

extern unsigned char *D_800F33E0;
extern unsigned char *D_800F32D0;
extern char D_8019A7A0[];
extern void func_801944F0();
extern int func_800CE560();
extern short **func_800CE610();
extern void func_8006DCE4();

int func_80194654(int a0, short *a1)
{
    short **e;
    unsigned char *p;
    short *q;
    int a;
    int b;
    unsigned char *r;
    short c;
    unsigned char *pp;

    switch (a0) {
    case 0:
        r = D_800F33E0;
        a1[0] = 0;
        return func_800CE560(((void **)r)[2], 4, 4, func_801944F0);
    case 1:
        p = ((unsigned char **)D_800F32D0)[2];
        if (p[0xE] == 0xD) {
            a = *(unsigned short *)(p + 0x16);
            if (a >= 0x1A) {
                return 2;
            }
            b = *(unsigned short *)(p + 0x1A);
            if (a >= 8 && b < 8) {
                a1[0] = 4;
                a1[1] = 0;
            }
        }
        if (a1[0] == 0) {
            return 0;
        }
        c = a1[1];
        a1[1] = c - 1;
        if (c > 0) {
            break;
        }
        e = func_800CE610(((void **)D_800F33E0)[2]);
        if (e == 0) {
            return 0;
        }
        *e = (short *)D_8019A7A0;
        a1[1] = 1;
        pp = D_800F32D0;
        a1[0]--;
        q = *e;
        func_8006DCE4(0x5C7, ((void **)*(void **)(((unsigned char **)pp)[2]))[2], q[0], q[1], q[2]);
        break;
    case 2:
        break;
    }
    return 0;
}
