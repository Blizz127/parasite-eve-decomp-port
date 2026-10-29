/* room_m0318i — func_80194164, blob offset 0x517C, 0x384 bytes. NON_MATCHING
 * (behaviour C, proven by the e21 differential; not a byte match).
 * Effect-object callback taking (mode, a1 = base position vector).
 * mode 0: init/attach (func_800CE870, arm the +0xD flag object, start the two
 *         D_800B0E64 sounds, register func_80193FC4 via func_800CE560);
 * mode 1: per-frame (randomised position jitter around a1, frame-0x27/0x28
 *         hooks, returns 1 once D_800E27EC >= 0x35);
 * mode 2: set the D_800F3368 draw block from the D_800E2850 table.
 * Any other mode returns 0. */
typedef struct { short x, y, z, w; } GteShortVector;

#define SH(o, x) (*(short *)((char *)(o) + (x)))

extern unsigned int **D_8009D254;
extern unsigned char *D_800E2368;
extern unsigned int ***D_800F32D0;
extern unsigned int *D_800F33E0;
extern int D_800B0E64;
extern int D_800E27EC;
extern unsigned short D_800E11EA;
extern unsigned short D_800E2850[];
extern short D_800F3368;
extern short D_800F336A;
extern short D_800F336C;
extern short D_800F336E;
extern short D_800F3370;
extern short D_800F3372;
extern short D_800F3374;
extern short D_800F3376;
extern short D_800F3378;
extern unsigned char func_80193FC4[];

extern int func_800CE870();
extern int func_800D3FD8();
extern int func_8006DF50();
extern int func_800CE560();
extern int func_80020D50();
extern int func_80020DD0();
extern int func_80071A54();
extern short *func_800CE610();

int func_80194164(int mode, unsigned short *a1)
{
    short *s0;
    unsigned char *p;
    unsigned int *q;
    unsigned int *r;
    int tmp;
    int frame;

    if (mode == 1) {
        if (D_800E27EC < 0x27)
            func_80020D50();
        if (D_800E27EC < 0x15) {
            s0 = func_800CE610(D_800F33E0[2]);
            if (s0 != 0) {
                tmp = func_80071A54();
                s0[0] = a1[0] + (tmp & 0x1FF) - 0x100;
                tmp = func_80071A54();
                s0[1] = a1[1] + (tmp & 0x1FF) - 0x100;
                tmp = func_80071A54();
                s0[2] = a1[2] + (tmp & 0x1FF) - 0x100;
            }
        }
        frame = D_800E27EC;
        if (frame == 0x27) {
            func_80020DD0();
            frame = D_800E27EC;
        }
        if (frame == 0x28) {
            if (D_800E2368[0xD] != 0) {
                if ((D_800F32D0[2][0][0] & 0x3F000000) == 0x01000000) {
                    q = D_8009D254[0];
                    q[0x4C / 4] |= 0x4000;
                    r = D_800F32D0[2][0];
                    r[0] = (r[0] & 0xC0FFFFFF) | 0x19000000;
                    r = D_800F32D0[2][0];
                    r[0] |= 0x80000000;
                }
            }
        }
        if (D_800E27EC < 0x35)
            return 0;
        return 1;
    }
    if (mode == 0) {
        func_800CE870(D_8009D254, 0, a1);
        if (D_800E2368[0xD] != 0) {
            if (D_800F32D0[2] != 0) {
                if (D_800F32D0[2][0] != 0) {
                    p = *(unsigned char **)((char *)D_800F32D0[2][0] + 0x18);
                    if (*p == 1)
                        *p = 2;
                }
            }
        }
        if (D_800B0E64 != 0) {
            tmp = func_800D3FD8();
            func_8006DF50(D_800B0E64, 0x5F1, tmp, 0x80, 0x7F);
            if (D_800B0E64 != 0)
                func_8006DF50(D_800B0E64, 0x5F2, 0x80, 0x80, 0x7F);
        }
        return func_800CE560(D_800F33E0[2], 8, 0x14, func_80193FC4);
    }
    if (mode == 2) {
        D_800F3368 = 0x20;
        D_800F336A = 2;
        D_800F3376 = 0x20;
        D_800F3378 = 0x20;
        tmp = D_800E2850[D_800E11EA];
        D_800F336C = 3;
        D_800F336E = 0;
        D_800F3372 = 0;
        D_800F3374 = 0x10;
        D_800F3370 = tmp;
        return 0;
    }
    return 0;
}
