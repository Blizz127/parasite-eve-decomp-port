/* room_m0273i — func_801949EC, blob offset 0x5A04, 0x170 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl11 2026-09-28).
 * Dual emitter/group controller (CE560 + CE5AC sum, CE688/CE78C tick/stop); asm volatile after the first call keeps the s0 copy first. */

extern unsigned char *D_800F33E0;
extern unsigned char *D_800F32D0;
extern unsigned char *D_8009D254;
extern unsigned char D_8019AE9A;
extern int D_8019AE94;
extern void func_801947CC();
extern void func_80199F84();
extern int func_800CE560();
extern int func_800CE5AC();
extern void func_800CE688();
extern void func_800CE78C();
extern void **func_800CE610();

int func_801949EC(int a0)
{
    void **e;
    unsigned char *p;
    int r;
    int a;
    unsigned short b;

    switch (a0) {
    case 0:
        r = func_800CE560(((void **)D_800F33E0)[2], 8, 2, func_801947CC);
        asm volatile("");
        a = func_800CE5AC(&D_8019AE94, r, 0x10, 9, func_80199F84);
        return r + a;
    case 1:
        if (D_8019AE9A != 0) {
            return 2;
        }
        p = ((unsigned char **)D_800F32D0)[2];
        if (p[0xE] == 9) {
            a = *(short *)(p + 0x16);
            b = *(unsigned short *)(p + 0x1A);
            if (a >= 4 && (short)b < 4) {
                e = func_800CE610(((void **)D_800F33E0)[2]);
                if (e) {
                    *e = (char *)((void **)D_8009D254)[0x238 / 4] + 0x14;
                }
            }
        }
        func_800CE688(D_8019AE94);
        break;
    case 2:
        func_800CE78C(D_8019AE94);
        break;
    }
    return 0;
}
