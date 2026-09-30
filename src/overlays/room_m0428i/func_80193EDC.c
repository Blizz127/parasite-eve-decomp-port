/* room_m0428i — func_80193EDC, blob offset 0x4EF4, 0x128 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl 2026-09-27).
 * event handler: spawn / count up then fill slot {0,0xF0D0C0,0x200,0,0x280}; levers: goto shared return 0, $3-pinned base, scalar *hp load before the flag store */

extern unsigned char *D_800F33E0;
extern short D_801941CC;
extern int func_800CE560();
extern unsigned char *func_800CE610();
extern void func_80192E60();

int func_80193EDC(int a0, short *a1)
{
    unsigned char *p;
    short n;
    register unsigned char *g asm("$3");
    void **hp;
    void *h;
    short *q;

    if (a0 != 1) {
        if (a0 < 2) {
            if (a0 == 0) {
                g = D_800F33E0;
                a1[0] = 0;
                a1[1] = 0;
                hp = (void **)(g + 8);
                h = *hp;
                D_801941CC = 4;
                return func_800CE560(h, 0x10, 4, func_80192E60);
            }
            return 0;
        }
    } else {
        if (a1[0] >= 4) {
            if (D_801941CC != 0) {
                goto out;
            }
            return 1;
        }
        a1[1] = n = a1[1] + 1;
        if (n < 6) {
            return 0;
        }
        p = func_800CE610(*(void **)(D_800F33E0 + 8));
        if (p != 0) {
            *(int *)(p + 4) = 0xF0D0C0;
            *(short *)(p + 8) = 0x200;
            *(int *)(p + 0) = 0;
            *(short *)(p + 0xA) = 0;
            *(short *)(p + 0xC) = 0x280;
            a1[1] = 0;
            a1[0]++;
        }
    }
    return 0;
out:
    return 0;
}
