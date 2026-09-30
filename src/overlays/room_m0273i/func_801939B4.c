/* room_m0273i — func_801939B4, blob offset 0x49CC, 0x1A8 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl11 2026-09-28).
 * Emitter controller w/ flag byte; array-indexed pointer reads (in_struct) let the flag stores sink below them. */

extern unsigned char *D_800F33E0;
extern unsigned char *D_8009D254;
extern int D_800E27EC;
extern unsigned char D_8019AE5C;
extern struct { short a68, a6A, a6C, a6E, a70, a72, a74, a76, a78; } D_800F3368;
extern unsigned short D_800E11FA;
extern unsigned short D_800E2850[];
extern void func_80193870();
extern int func_800CE560();
extern void **func_800CE610();

#define P(o, x) (*(void **)((char *)(o) + (x)))

int func_801939B4(int a0)
{
    void **e;

    switch (a0) {
    case 0:
        D_8019AE5C = 0;
        return func_800CE560(((void **)D_800F33E0)[2], 4, 1, func_80193870);
    case 1:
        if (D_800E27EC >= 0x14) {
            return 2;
        }
        if (D_8019AE5C != 0) {
            return 2;
        }
        if (D_8009D254[0xE] >= 4) {
            return 0;
        }
        e = func_800CE610(((void **)D_800F33E0)[2]);
        if (e) {
            D_8019AE5C = 1;
            *e = (char *)((void **)D_8009D254)[0x238 / 4] + 0x14;
        }
        break;
    case 2:
        if (D_8019AE5C != 0) {
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
            D_800F3368.a74 = 0;
        }
        break;
    }
    return 0;
}
