/* room_m0350i — func_80199F2C, blob offset 0xAF44, 0xE8 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl 2026-09-27).
 * event handler (spawn / copy vector into slot); lever: scalar *q stores */

extern unsigned char *D_800F33E0;
extern unsigned char D_8019A86E;
extern unsigned char D_8019A856;
extern short D_8019A860, D_8019A862;
extern short D_800942EC;
extern int func_800CE560();
extern short *func_800CE610();
extern void func_8019A21C();

int func_80199F2C(int a0)
{
    short *p;
    short *q;

    if (a0 != 1) {
        if (a0 < 2) {
            if (a0 == 0) {
                return func_800CE560(*(void **)(D_800F33E0 + 8), 8, 6, func_8019A21C);
            }
            return 0;
        }
    } else {
        if (D_8019A86E != 0) {
            return 2;
        }
        if (D_8019A856 == 0) {
            return 0;
        }
        p = func_800CE610(*(void **)(D_800F33E0 + 8));
        if (p == 0) {
            return 0;
        }
        *p = D_8019A860;
        q = p + 1;
        *q = D_800942EC;
        q = p + 2;
        *q = D_8019A862;
        D_8019A856 = 0;
    }
    return 0;
}
