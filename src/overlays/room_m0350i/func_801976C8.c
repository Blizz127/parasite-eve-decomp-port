/* room_m0350i — func_801976C8, blob offset 0x86E0, 0x184 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl11 2026-09-28).
 * Emitter controller (func_801938E4 shape): struct source vector, in-arm return 0 between asm volatile barriers. */

extern unsigned char *D_800F33E0;
extern struct { unsigned short x, y, z; } D_8019A864;
extern unsigned char D_8019A86E;
extern unsigned char D_8019A857;
extern struct { short a68, a6A, a6C, a6E, a70, a72, a74, a76, a78; } D_800F3368;
extern unsigned short D_800E11FA;
extern unsigned short D_800E2850[];
extern void func_80197594();
extern int func_800CE560();
extern short *func_800CE610();

#define P(o, x) (*(void **)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))

int func_801976C8(int a0)
{
    short *e;

    switch (a0) {
    case 0:
        return func_800CE560(P(D_800F33E0, 8), 8, 4, func_80197594);
    case 1:
        if (D_8019A86E != 0) {
            return 2;
        }
        if (D_8019A857 != 0) {
            e = func_800CE610(P(D_800F33E0, 8));
            if (e) {
                *e = D_8019A864.x;
                *(e + 1) = D_8019A864.y;
                *(e + 2) = D_8019A864.z;
                *(e + 3) = 0;
                asm volatile("");
                D_8019A857 = 0;
                asm volatile("");
                return 0;
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
