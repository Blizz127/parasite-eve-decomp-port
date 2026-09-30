/* room_m0350i — func_80199D50, blob offset 0xAD68, 0xEC bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl 2026-09-27).
 * event handler (spawn / copy player pos-0x140 into slot); lever: scalar *q stores keep order vs flag byte */

extern unsigned char *D_800F33E0;
extern unsigned char D_8019A82E;
extern unsigned char D_8019A830;
extern unsigned char *D_8009D254;
extern int func_800CE560();
extern short *func_800CE610();
extern void func_80196C64();

int func_80199D50(int a0)
{
    short *p;
    short *q;
    unsigned char *d;

    if (a0 != 1) {
        if (a0 < 2) {
            if (a0 == 0) {
                return func_800CE560(*(void **)(D_800F33E0 + 8), 8, 2, func_80196C64);
            }
            return 0;
        }
    } else {
        if (D_8019A82E != 0) {
            return 2;
        }
        if (D_8019A830 == 0) {
            return 0;
        }
        p = func_800CE610(*(void **)(D_800F33E0 + 8));
        if (p == 0) {
            return 0;
        }
        d = D_8009D254;
        *p = *(int *)(d + 0x1FC);
        q = p + 1;
        *q = *(int *)(d + 0x200) - 0x140;
        q = p + 2;
        *q = *(int *)(d + 0x204);
        D_8019A830 = 0;
    }
    return 0;
}
