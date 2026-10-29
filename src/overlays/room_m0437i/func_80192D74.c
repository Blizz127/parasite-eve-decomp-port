/* room_m0437i (PE.IMG room m0437i chunk 2, VRAM 0x8018EFE8)
 * func_80192D74 — blob offset 0x3D8C, 0x360 bytes. Profile era_o2_g0_expand_div;
 * LINK_EXACT at the room VMA (docs/evidence/room-lane-2026-09-23/REPORT.md).
 * Effect callback (called through func_800D413C): mode 0 nudges the actor state when the room flag is set, plays sound 1448 and opens a 20-entry particle pool with callback D_80192500 (func_80192500); mode 1 takes two bone points of the current actor and spawns a marker particle on frame 1 and rising sparks on odd frames below 47 (returns 2 from frame 2); mode 2 sets the 16x16 sprite globals. */

/* Matching notes: the arg[1] modulo needs the era_o2_g0_expand_div profile
 * (retail div carries the divide-by-zero / overflow break checks). */
typedef struct { short vx, vy, vz, pad; } SVECTOR;

extern int *D_800F32D0;
extern int *D_800F33E0;
extern unsigned char *D_800E2368;
extern int D_800E27EC;
extern volatile short D_800F3368;
extern volatile short D_800F336A;
extern volatile short D_800F336C;
extern volatile short D_800F336E;
extern volatile short D_800F3370;
extern short D_800F3372;
extern short D_800F3374;
extern volatile short D_800F3376;
extern volatile short D_800F3378;
extern unsigned short D_800E11EC;
extern unsigned short D_800E2850[];
extern SVECTOR D_8018F1D4;
extern SVECTOR D_8018F1DC;
extern int D_80192500;
extern int func_80071A54();
extern int func_800CE560();
extern int func_800CE610();
extern void func_800CE8F0();
extern void func_800CE9D4();
extern void func_800D3F64();
extern int func_800D3FD8(void);

int func_80192D74(int mode, short *p, int *arg)
{
    SVECTOR sv0 = D_8018F1D4;
    SVECTOR sv1 = D_8018F1DC;
    SVECTOR a;
    SVECTOR b;
    short *q;
    int *e;
    unsigned char *s;

    switch (mode) {
    case 0:
        if (D_800E2368[13] != 0) {
            e = (int *)D_800F32D0[2];
            if (e != 0 && e[0] != 0) {
                s = ((unsigned char **)e[0])[6];
                if (*s == 1)
                    *s = 2;
            }
        }
        func_800D3F64(1448, func_800D3FD8());
        return func_800CE560(D_800F33E0[2], 20, 55, &D_80192500);
    case 1:
        func_800CE8F0(D_800F32D0[2], 3, &sv0, &a);
        func_800CE9D4(D_800F32D0[2], 0, &b);
        if (D_800E27EC == 1) {
            q = (short *)func_800CE610(D_800F33E0[2]);
            if (q != 0) {
                q[0] = a.vx;
                q[1] = a.vy;
                q[2] = a.vz;
                q[8] = 2;
                q[9] = 0;
            }
        }
        func_800CE8F0(D_800F32D0[2], 3, &sv1, &a);
        if ((D_800E27EC & 1) && D_800E27EC < 47) {
            q = (short *)func_800CE610(D_800F33E0[2]);
            if (q != 0) {
                q[0] = a.vx;
                q[1] = a.vy;
                q[2] = a.vz;
                q[6] = 0;
                q[4] = 0;
                q[5] = b.vy + func_80071A54() % arg[1] - arg[1] / 2;
                q[8] = 0;
                q[9] = 0;
            }
        }
        if (D_800E27EC >= 2)
            return 2;
        break;
    case 2:
    {
        int value;
        int index;
        index = D_800E11EC;
        D_800F3368 = 16;
        D_800F336A = 1;
        D_800F3376 = 16;
        D_800F3378 = 16;
        value = D_800E2850[index];
        asm("" : : "r"(value) : "$2", "memory");
        D_800F336C = 4;
        D_800F336E = 0;
        D_800F3372 = 0;
        D_800F3374 = 4;
        D_800F3370 = value;
    }
        break;
    }
    return 0;
}
