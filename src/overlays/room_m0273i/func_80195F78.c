/* room_m0273i — func_80195F78, blob offset 0x6F90, 0x17C bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl5 2026-09-27).
 * 3-mode controller; D_8019AEB2 as its own scalar extern (SVECTOR .pad field: address CSE'd into s0); D_800F3368 struct + climb. */

typedef struct { short vx, vy, vz, pad; } SVECTOR;
extern unsigned char *D_800F33E0;
extern unsigned char D_8019AEF8;
extern SVECTOR D_8019AEAC;
extern short D_8019AEB2;
extern struct { short a68, a6A, a6C, a6E, a70, a72, a74, a76, a78; } D_800F3368;
extern unsigned short D_800E11FA;
extern unsigned short D_800E2850[];
extern void func_80195E10();
extern int func_800CE560();
extern unsigned char *func_800CE610();

#define P(o, x) (*(void **)((char *)(o) + (x)))

int func_80195F78(int a0)
{
    unsigned char *e;

    switch (a0) {
    case 0:
        return func_800CE560(P(D_800F33E0, 8), 8, 4, func_80195E10);
    case 1:
        if (D_8019AEF8 != 0) {
            return 2;
        }
        if (D_8019AEB2 != 0) {
            e = func_800CE610(P(D_800F33E0, 8));
            if (e) {
                D_8019AEB2 = 0;
                *(SVECTOR *)e = D_8019AEAC;
            }
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
