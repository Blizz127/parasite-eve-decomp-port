/* room_m0273i — func_80197A48, blob offset 0x8A60, 0x174 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl11 2026-09-28).
 * 3-mode emitter controller; source vector as a struct (MEM_IN_STRUCT keeps load/store pairs in order). */

extern unsigned char *D_800F33E0;
extern unsigned char D_8019AF68;
extern unsigned char D_8019AF69;
extern struct { unsigned short x, y, z; } D_8019AEFC;
extern struct { short a68, a6A, a6C, a6E, a70, a72, a74, a76, a78; } D_800F3368;
extern unsigned short D_800E11EA;
extern unsigned short D_800E2850[];
extern void func_801977F8();
extern int func_800CE560();
extern short *func_800CE610();

#define P(o, x) (*(void **)((char *)(o) + (x)))

int func_80197A48(int a0)
{
    short *e;

    switch (a0) {
    case 0:
        return func_800CE560(P(D_800F33E0, 8), 8, 4, func_801977F8);
    case 1:
        if (D_8019AF69 != 0) {
            return 2;
        }
        if (D_8019AF68 != 0) {
            e = func_800CE610(P(D_800F33E0, 8));
            if (e) {
                e[0] = D_8019AEFC.x;
                e[1] = D_8019AEFC.y;
                e[2] = D_8019AEFC.z;
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
        D_800F3368.a70 = D_800E2850[D_800E11EA];
        D_800F3368.a6C = 3;
        D_800F3368.a6E = 0;
        D_800F3368.a72 = 0;
        D_800F3368.a74 = 0x40;
        break;
    }
    return 0;
}
