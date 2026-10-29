/* room_m0404i — func_80193A6C, blob offset 0x4A84, 0x3B8 bytes. NON_MATCHING
 * (behaviour C, proven by the e21 differential; not a byte match).
 * Three-command room callback (mode in a0, record pointer in a1):
 *   mode 0: set up the D_800F32D0 / D_800F33E0 handles (func_800CE870,
 *           func_800CE8F0 with the 8-byte stack seed D_8018F21C, then
 *           func_800D3F64 0x5C0/0x5C1, then returns func_800CE560 with
 *           func_801935E0; v0 is left holding that call's result).
 *   mode 1: while D_800E27EC < 5 spawn 6 particles (random x%50-25 /
 *           -(x%39) / x%50-25 velocities, state 1), then 2 more
 *           (x%80-40 / -(x%80) / x%80-40, state 0); returns 2 once
 *           D_800E27EC >= 2, else 0.
 *   mode 2: set the D_800F336C draw block from table D_800E2850[D_800E11EA].
 * The magic-number multiplies (0x51EB851F, 0xD20D20D3, 0x66666667) are signed
 * constant divisions by 50, 39 and 80. */
typedef struct { unsigned char c[8]; } B8;

#define H(o, x) (*(unsigned short *)((char *)(o) + (x)))

extern B8 D_8018F21C;
extern int *D_800F32D0;
extern int *D_800F33E0;
extern int D_800E27EC;
extern unsigned short D_800E11EA;
extern unsigned short D_800E2850[];
extern short D_800F336C;
extern short D_800F336E;
extern unsigned short D_800F3370;
extern short D_800F3372;
extern short D_800F3374;
extern unsigned char func_801935E0[];

extern int func_800CE870();
extern int func_800CE8F0();
extern int func_800D3FD8();
extern int func_800D3F64();
extern int func_800CE560();
extern unsigned char *func_800CE610();
extern int func_80071A54();

int func_80193A6C(int mode, unsigned char *a1)
{
    B8 buf;
    unsigned char *p;
    int i;
    int r;

    buf = D_8018F21C;
    switch (mode) {
    case 0:
        func_800CE870(D_800F32D0[2], 0, a1);
        func_800CE8F0(D_800F32D0[2], 0, &buf, a1);
        r = func_800D3FD8();
        func_800D3F64(0x5C0, r);
        func_800D3F64(0x5C1, 0x80);
        return func_800CE560(D_800F33E0[2], 0x14, 0x20, func_801935E0);
    case 1:
        if (D_800E27EC < 5) {
            for (i = 0; i < 6; i++) {
                p = func_800CE610(D_800F33E0[2]);
                if (p) {
                    H(p, 0) = H(a1, 0);
                    H(p, 2) = H(a1, 2);
                    H(p, 4) = H(a1, 4);
                    r = func_80071A54();
                    H(p, 8) = r % 50 - 25;
                    r = func_80071A54();
                    H(p, 0xA) = -(r % 39);
                    r = func_80071A54();
                    H(p, 0x10) = 1;
                    H(p, 0x12) = 0;
                    H(p, 0xC) = r % 50 - 25;
                }
            }
            for (i = 0; i < 2; i++) {
                p = func_800CE610(D_800F33E0[2]);
                if (p) {
                    H(p, 0) = H(a1, 0);
                    H(p, 2) = H(a1, 2);
                    H(p, 4) = H(a1, 4);
                    r = func_80071A54();
                    H(p, 8) = r % 80 - 40;
                    r = func_80071A54();
                    H(p, 0xA) = -(r % 80);
                    r = func_80071A54();
                    H(p, 0x10) = 0;
                    H(p, 0x12) = 0;
                    H(p, 0xC) = r % 80 - 40;
                }
            }
        }
        if (D_800E27EC < 2)
            return 0;
        return 2;
    case 2:
        D_800F336C = 3;
        D_800F336E = 0;
        D_800F3372 = 0;
        D_800F3374 = 8;
        D_800F3370 = D_800E2850[D_800E11EA];
        return 0;
    }
    return 0;
}
