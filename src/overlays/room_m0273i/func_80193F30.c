/* room_m0273i — func_80193F30, blob offset 0x4F48, 0x1F8 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl11 2026-09-28).
 * Model + dual emitter controller (8006E498 handle, 3-slot ring seeded from player pos); h pointer local, asm volatile after CE560. */

extern unsigned char *D_800F33E0;
extern unsigned char *D_800F32D0;
extern int D_800B0E64;
extern int D_8019AE8C[];
extern int D_8019AE90;
extern unsigned char D_8019AE9A;
extern short D_800942EC;
extern void func_80193CB8();
extern void func_80193B5C();
extern int func_800CE560();
extern int func_800CE5AC();
extern void func_800CE688();
extern void func_800CE78C();
extern short *func_800CE610();
extern int func_8006E498();
extern void func_800C6D5C();

int func_80193F30(int a0)
{
    short *e;
    unsigned char *p;
    int *h;
    int r;
    short i;
    int a;
    unsigned short b;

    switch (a0) {
    case 0:
        h = D_8019AE8C;
        h[0] = func_8006E498(D_800B0E64, 0xC5941704);
        func_800C6D5C(h[0], 0, 0);
        r = func_800CE560(((void **)D_800F33E0)[2], 0x10, 6, func_80193CB8);
        asm volatile("");
        return r + func_800CE5AC(&h[1], r, 4, 2, func_80193B5C);
    case 1:
        if (D_8019AE9A != 0) {
            return 2;
        }
        p = ((unsigned char **)D_800F32D0)[2];
        if (p[0xE] == 9) {
            a = *(short *)(p + 0x16);
            b = *(unsigned short *)(p + 0x1A);
            if (a > 0 && (short)b <= 0) {
                for (i = 0; i < 3; i++) {
                    e = func_800CE610(((void **)D_800F33E0)[2]);
                    if (e == 0) {
                        break;
                    }
                    e[0] = *(int *)(*(char **)(p + 0x238) + 0x594);
                    e[1] = D_800942EC;
                    e[2] = *(int *)(*(char **)(p + 0x238) + 0x59C);
                    e[3] = i * 384;
                    if (i == 0) {
                        *(short **)func_800CE610(D_8019AE90) = e;
                    }
                }
            }
        }
        func_800CE688(D_8019AE90);
        break;
    case 2:
        func_800CE78C(D_8019AE90);
        break;
    }
    return 0;
}
