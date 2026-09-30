/* room_m0273i — func_801993F0, blob offset 0xA408, 0x178 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl11 2026-09-28).
 * Flagged 2-slot vector emitter; flag-byte base pointer b pinned $3 with q = b-0x18 / v = b-0x1E (retail's CSE base), v pinned $16. */

typedef struct { unsigned short x, y, z, w; } SV;
extern unsigned char *D_800F33E0;
extern int D_800B0E64;
extern int D_8019AF6C;
typedef struct { SV t[2]; unsigned char pad[0xE]; unsigned char f; } ST;
extern ST D_8019AF84;
extern struct { short a68, a6A, a6C, a6E, a70, a72, a74, a76, a78; } D_800F3368;
extern void func_80198E94();
extern int func_800CE560();
extern short *func_800CE610();
extern int func_8006E498();
extern void func_800C6D5C();

int func_801993F0(int a0)
{
    short *e;
    int i;
    register unsigned char *b asm("$3");
    unsigned short *q;
    register SV *v asm("$16");

    switch (a0) {
    case 0:
        D_8019AF6C = func_8006E498(D_800B0E64, 0xC5541704);
        func_800C6D5C(D_8019AF6C, 0, 0);
        return func_800CE560(((void **)D_800F33E0)[2], 8, 4, func_80198E94);
    case 1:
        b = &D_8019AF84.f;
        if (*b != 0) {
            return 2;
        }
        for (i = 0, q = (unsigned short *)(b - 0x18), v = (SV *)(b - 0x1E); i < 2; q += 4, i++, v++) {
            if (*q & 1) {
                e = func_800CE610(((void **)D_800F33E0)[2]);
                if (e == 0) {
                    return 0;
                }
                e[0] = v->x;
                e[1] = v->y;
                e[2] = v->z;
                e[3] = 0;
                *q &= ~1;
            }
        }
        break;
    case 2:
        D_800F3368.a72 = 0;
        D_800F3368.a74 = 0;
        break;
    }
    return 0;
}
