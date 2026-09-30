/* room_m0245i — func_80194AAC, blob offset 0x5AC4, 0x1A8 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl5 2026-09-27).
 * In-room twin of func_80193244 (callback func_80193D7C). */

extern unsigned char *D_800E2368;
extern unsigned char *D_800F32D0;
extern unsigned char *D_800F33E0;
extern int D_800E27EC;
extern struct { short a68, a6A, a6C, a6E, a70, a72, a74, a76, a78; } D_800F3368;
extern unsigned short D_800E11EA;
extern unsigned short D_800E2850[];
extern void func_80193D7C();
extern int func_800CE560();
extern unsigned char *func_800CE610();

#define SH(o, x) (*(short *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))

typedef struct { short h0; } OB;

int func_80194AAC(int a0, OB *a1)
{
    unsigned char *e;
    unsigned char *r;
    int v;

    switch (a0) {
    case 0:
        a1->h0 = 0;
        if (D_800E2368[0xD] != 0 && P(D_800F32D0, 8) != 0 && *(void **)P(D_800F32D0, 8) != 0) {
            r = P(*(void **)P(D_800F32D0, 8), 0x18);
            if (r[0] == 1) {
                r[0] = 2;
            }
        }
        return func_800CE560(P(D_800F33E0, 8), 0x18, 0x20, func_80193D7C);
    case 1:
        if (D_800E27EC == 1) {
            e = func_800CE610(P(D_800F33E0, 8));
            if (e) {
                SH(e, 6) = a1->h0;
                SH(e, 0x14) = 0;
                SH(e, 0x16) = 0;
            }
        }
        if (D_800E27EC >= 8) {
            return 2;
        }
        break;
    case 2:
        v = D_800E2850[D_800E11EA];
        D_800F3368.a6C = 3;
        D_800F3368.a6E = 0;
        D_800F3368.a72 = 0;
        D_800F3368.a74 = 8;
        D_800F3368.a70 = v;
        break;
    }
    return 0;
}
