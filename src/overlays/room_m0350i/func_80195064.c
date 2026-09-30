/* room_m0350i — func_80195064, blob offset 0x607C, 0x1B4 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl11 2026-09-28).
 * Emitter controller: queued-seed flush loop over a table+count object (struct D_8019A7A8 so the count and table share one base). */

extern unsigned char *D_800F33E0;
extern unsigned char D_8019A804;
extern short D_8019A802;
extern struct { int t[21]; short pad; short n; } D_8019A7A8;
extern struct { short a68, a6A, a6C, a6E, a70, a72, a74, a76, a78; } D_800F3368;
extern unsigned short D_800E11FA;
extern unsigned short D_800E2850[];
extern void func_80194F04();
extern int func_800CE560();
extern short *func_800CE610();

int func_80195064(int a0)
{
    short *e;
    int i;

    switch (a0) {
    case 0:
        return func_800CE560(((void **)D_800F33E0)[2], 8, 0x10, func_80194F04);
    case 1:
        if (D_8019A804 != 0 && D_8019A802 == 0) {
            return 2;
        }
        if (D_8019A7A8.n != 0) {
            for (i = 0; i < D_8019A7A8.n; i++) {
                e = func_800CE610(((void **)D_800F33E0)[2]);
                if (e == 0) {
                    break;
                }
                *(int *)e = D_8019A7A8.t[i];
                e[2] = 0;
            }
            D_8019A7A8.n = 0;
            return 0;
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
        D_800F3368.a74 = 0;
        break;
    }
    return 0;
}
