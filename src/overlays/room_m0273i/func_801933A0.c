/* room_m0273i — func_801933A0, blob offset 0x43B8, 0x19C bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl11 2026-09-28).
 * Emitter controller (sin/cos-scaled bytes); table base local t keeps retail's la base + index operand order. */

extern unsigned char *D_800F33E0;
extern unsigned char *D_800F32D0;
extern int D_800E27EC;
extern short D_800966EC[];
extern void func_8019320C();
extern int func_800CE560();
extern short *func_800CE610();

#define P(o, x) (*(void **)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))

int func_801933A0(int a0)
{
    short *e;
    char *q;
    short *t;


    switch (a0) {
    case 0:
        return func_800CE560(P(D_800F33E0, 8), 0xC, 9, func_8019320C);
    case 1:
        if (D_800E27EC >= 0xC) {
            return 2;
        }
        e = func_800CE610(P(D_800F33E0, 8));
        if (e) {
            t = D_800966EC;
            q = P(P(D_800F32D0, 8), 0x238);
            e[0] = W(q, 0x594);
            e[1] = W(q, 0x598);
            e[2] = W(q, 0x59C);
            ((char *)e)[8] = D_800966EC[(((D_800E27EC << 10) / 12) & 0xFFF) * 2] * 40 / 4096;
            ((char *)e)[9] = 8;
            ((char *)e)[10] = t[(((D_800E27EC << 10) / 12) & 0xFFF) * 2 + 1] * 40 / 4096;
        }
        break;
    case 2:
        break;
    }
    return 0;
}
