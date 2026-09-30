/* room_m0350i — func_801938E4, blob offset 0x48FC, 0x19C bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl11 2026-09-28).
 * Emitter controller; in-arm return 0 and two asm volatile("") sched barriers pin the D_8019A79C clear between the vector stores and the j/addu tail. */

extern unsigned char *D_800F33E0;
extern unsigned char *D_8009D254;
extern unsigned char D_8019A79A;
extern unsigned char D_8019A79C;
extern struct { short a68, a6A, a6C, a6E, a70, a72, a74, a76, a78; } D_800F3368;
extern unsigned short D_800E11FA;
extern unsigned short D_800E2850[];
extern void func_801937B4();
extern int func_800CE560();
extern short *func_800CE610();

#define P(o, x) (*(void **)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))

int func_801938E4(int a0)
{
    short *e;
    unsigned char *p;

    switch (a0) {
    case 0:
        return func_800CE560(P(D_800F33E0, 8), 8, 4, func_801937B4);
    case 1:
        if (D_8019A79A != 0) {
            return 2;
        }
        if (D_8019A79C != 0) {
            e = func_800CE610(P(D_800F33E0, 8));
            if (e) {
                p = D_8009D254;
                *e = W(P(p, 0x238), 0x14);
                *(e + 1) = W(P(p, 0x238), 0x18);
                *(e + 2) = W(P(p, 0x238), 0x1C);
                *(e + 3) = 0;
                asm volatile("");
                D_8019A79C = 0;
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
