/* room_m0188i — func_80194588, blob offset 0x55A0, 0x214 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl11 2026-09-28).
 * Counted emitter controller; retail copies an uninitialised stack SVECTOR v (source bug reproduced); ladder-flag poke in case 0. */

typedef struct { unsigned short vx, vy, vz, pad; } SVECTOR;
extern unsigned char *D_800F33E0;
extern unsigned char *D_800F32D0;
extern unsigned char *D_800E2368;
extern int D_800E27EC;
extern struct { short a68, a6A, a6C, a6E, a70, a72, a74, a76, a78; } D_800F3368;
extern unsigned short D_800E11EA;
extern unsigned short D_800E2850[];
extern void func_80193BCC();
extern int func_800CE560();
extern short *func_800CE610();
extern int func_800D3FD8();
extern void func_800D3F64();

int func_80194588(int a0, short *a1)
{
    SVECTOR v;
    short *e;
    int *p;
    unsigned char *q;

    switch (a0) {
    case 0:
        a1[0] = 0;
        func_800D3F64(0x594, func_800D3FD8());
        if (D_800E2368[0xD] != 0) {
            p = ((int **)D_800F32D0)[2];
            if (p != 0) {
                p = (int *)p[0];
                if (p != 0) {
                    q = ((unsigned char **)p)[6];
                    if (q[0] == 1) {
                        q[0] = 2;
                    }
                }
            }
        }
        return func_800CE560(((void **)D_800F33E0)[2], 0x18, 0xE, func_80193BCC);
    case 1:
        if (a1[0] < 0xB) {
            e = func_800CE610(((void **)D_800F33E0)[2]);
            if (e) {
                e[0] = v.vx;
                e[1] = v.vy;
                e[2] = v.vz;
                e[9] = 0;
                e[10] = a1[0];
                if (a1[0] >= 10) {
                    e[8] = 1;
                } else {
                    e[8] = 0;
                }
            }
            a1[0]++;
        }
        if (D_800E27EC >= 10) {
            return 2;
        }
        break;
    case 2:
        D_800F3368.a70 = D_800E2850[D_800E11EA];
        D_800F3368.a6C = 3;
        D_800F3368.a6E = 0;
        D_800F3368.a72 = 0;
        D_800F3368.a74 = 8;
        break;
    }
    return 0;
}
