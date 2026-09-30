/* room_m0123i — func_801945EC, blob offset 0x5604, 0x17C bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl7 2026-09-27).
 * Effect-sprite phase handler (switch on a0; k==4 colour boost; func_800CF844 then func_800CEE20); HS struct view of a1 + post-increment keep retail order. */

typedef struct { short x, y, z, pad; } SV;
extern short D_801956A4;
extern short D_801956A0;
extern char D_801956B0[];
extern char D_801956A8[];
extern unsigned short D_800F336C;
extern short D_800F336A;
extern unsigned short D_800E1204[];
extern int D_800F3428;
extern unsigned short func_80077AA4();
extern void func_800CF844(), func_800CEE20();

typedef struct { short h[4]; } HS;

int func_801945EC(int a0, HS *a1)
{
    SV s;
    int k;
    int v;

    switch (a0) {
    case 1:
        a1->h[0]++;
        a1->h[1] += D_801956A4 * 24 / 4096;
        if (a1->h[0] >= 0x18) {
            return 1;
        }
        break;
    case 2:
        func_800CF844(D_801956B0, &s, a1->h[0] << 6, D_801956A8, D_801956A4 * a1->h[2] / 4096, a1->h[1]);
        k = D_800F336C;
        v = D_800E1204[k];
        if (k == 4 && D_800F3428 != 0) {
            v += 4;
        }
        func_800CEE20(&s, 0, 0x555, 0x555, D_800F336A * (a1->h[0] & 7) + 0x40, func_80077AA4(0x20, v), 1, D_801956A0, 0);
        break;
    default:
        return 0;
    }
    return 0;
}
