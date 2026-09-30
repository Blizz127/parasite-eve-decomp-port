/* room_m0273i — func_801945A8, blob offset 0x55C0, 0x224 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl11 2026-09-28).
 * Burst emitter controller (count + cooldown shorts); both player fields read before the && test (unsigned short b, (short) compare). */

extern unsigned char *D_800F33E0;
extern unsigned char *D_800F32D0;
extern unsigned char *D_8009D254;
extern unsigned char D_8019AE9A;
extern short D_8019AE60;
extern short D_8019AE64;
extern struct { short a68, a6A, a6C, a6E, a70, a72, a74, a76, a78; } D_800F3368;
extern unsigned short D_800E11FA;
extern unsigned short D_800E2850[];
extern void func_80194470();
extern int func_800CE560();
extern short *func_800CE610();

int func_801945A8(int a0)
{
    short *e;
    unsigned char *p;
    int *q;
    short c;
    int a;
    unsigned short b;

    switch (a0) {
    case 0:
        D_8019AE60 = 0;
        D_8019AE64 = 0;
        return func_800CE560(((void **)D_800F33E0)[2], 8, 4, func_80194470);
    case 1:
        if (D_8019AE9A != 0) {
            return 2;
        }
        p = ((unsigned char **)D_800F32D0)[2];
        if (p[0xE] == 9) {
            a = *(short *)(p + 0x16);
            b = *(unsigned short *)(p + 0x1A);
            if (a >= 2 && (short)b < 2) {
            D_8019AE60 = 4;
            D_8019AE64 = 0;
            }
        }
        if (D_8019AE60 == 0) {
            return 0;
        }
        c = D_8019AE64;
        D_8019AE64 = c - 1;
        if (c > 0) {
            return 0;
        }
        e = func_800CE610(((void **)D_800F33E0)[2]);
        if (e) {
            q = ((int **)D_8009D254)[0x238 / 4];
            e[0] = q[5];
            e[1] = q[6];
            D_8019AE64 = 2;
            D_8019AE60--;
            e[2] = q[7];
        }
        break;
    case 2:
        D_800F3368.a68 = 0x20;
        D_800F3368.a6A = 2;
        D_800F3368.a76 = 0x20;
        D_800F3368.a78 = 0x20;
        D_800F3368.a76 = 0x20;
        D_800F3368.a78 = 0x20;
        D_800F3368.a70 = D_800E2850[D_800E11FA];
        D_800F3368.a6C = 3;
        D_800F3368.a6E = 1;
        D_800F3368.a72 = 0;
        D_800F3368.a74 = 0x20;
        break;
    }
    return 0;
}
