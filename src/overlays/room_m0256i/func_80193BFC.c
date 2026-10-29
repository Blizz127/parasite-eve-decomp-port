/* room_m0256i — func_80193BFC, blob offset 0x4C14, 0x4B4 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (2026-10-08, written from the retail asm).
 * 3-mode effect controller (spawn / per-frame particles / fade setup), the
 * effect callback m0256i stopped on under the native-only policy. */

typedef struct { short vx, vy, vz, pad; } SVECTOR;
extern SVECTOR D_8018F210;
extern void *D_800B0E64;
extern unsigned char *D_800F32D0;
extern unsigned char *D_800F33E0;
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
extern void func_8019377C();
extern int func_800D3FD8();
extern void func_8006DF50();
extern int func_800CE560();
extern void func_800CE8F0();
extern void func_800CE9D4();
extern unsigned char *func_800CE610();
extern int func_80071A54();
extern void func_800CFB7C();

int func_80193BFC(int a0, short *a1)
{
    SVECTOR sv;
    SVECTOR p;
    SVECTOR q;
    SVECTOR v;
    unsigned char *e;
    int n;
    int r;
    short t;

    sv = D_8018F210;
    n = 0;
    switch (a0) {
    case 0:
    {
        void **sound = &D_800B0E64;
        int volume;
        void *current;
        *a1 = 0;
        if (*sound != 0) {
            volume = 0x7F;
            func_8006DF50(*sound, 0x5A7, func_800D3FD8(), 0x80, volume);
            current = *sound;
            asm("" : : "r"(current) : "memory");
            if (current != 0) {
                func_8006DF50(*sound, 0x5BF, 0x80, 0x80, volume);
            }
        }
    }
        return func_800CE560(*(void **)(D_800F33E0 + 8), 0x14, 0x20, func_8019377C);
    case 1:
        func_800CE8F0(*(void **)(D_800F32D0 + 8), 4, &sv, &p);
        func_800CE9D4(*(void **)(D_800F32D0 + 8), 4, &q);
        if (D_800E27EC < 0x6F) {
            e = func_800CE610(*(void **)(D_800F33E0 + 8));
            if (e) {
                ((short *)e)[0] = p.vx;
                ((short *)e)[1] = p.vy;
                ((short *)e)[2] = p.vz;
                switch (*a1) {
                case 0:
                    n = (func_80071A54() & 7) + 0xE;
                    v.vx = q.vx; v.vy = q.vy; v.vz = q.vz;
                    r = func_80071A54() & 0x7F; t = v.vy - 0x40; v.vy = t + r;
                    break;
                case 1:
                    n = (func_80071A54() & 0xF) + 0xA;
                    v.vx = q.vx; v.vy = q.vy; v.vz = q.vz;
                    r = func_80071A54() & 0x7F; t = v.vy + 0x3C0; v.vy = t + r;
                    break;
                case 2:
                    n = (func_80071A54() & 0xF) + 0xA;
                    v.vx = q.vx; v.vy = q.vy; v.vz = q.vz;
                    r = func_80071A54() & 0x7F; t = v.vy - 0x3C0; v.vy = t - r;
                    break;
                }
                func_800CFB7C(&v, n, e + 8);
                ((short *)e)[8] = 0;
                ((short *)e)[9] = 0;
            }
            e = func_800CE610(*(void **)(D_800F33E0 + 8));
            if (e) {
                ((short *)e)[0] = p.vx;
                ((short *)e)[1] = p.vy;
                ((short *)e)[2] = p.vz;
                switch (*a1 & 1) {
                case 0:
                    n = (func_80071A54() & 0xF) + 0x10;
                    v.vx = q.vx; v.vy = q.vy; v.vz = q.vz;
                    r = func_80071A54() & 0xFF; t = v.vy + 0x380; v.vy = t + r;
                    break;
                case 1:
                    n = (func_80071A54() & 0xF) + 0x10;
                    v.vx = q.vx; v.vy = q.vy; v.vz = q.vz;
                    r = func_80071A54() & 0xFF; t = v.vy - 0x380; v.vy = t - r;
                    break;
                }
                func_800CFB7C(&v, n, e + 8);
                ((short *)e)[3] = func_80071A54() & 1;
                ((short *)e)[8] = 1;
                ((short *)e)[9] = 0;
            }
            *a1 = (short)(*a1 + 1) % 3;
        }
        if (D_800E27EC >= 2) {
            return 2;
        }
        break;
    case 2:
    {
        int value;
        int index;
        index = D_800E11EA;
        D_800F3368 = 0x20;
        D_800F336A = 2;
        D_800F3376 = 0x20;
        D_800F3378 = 0x20;
        value = D_800E2850[index];
        D_800F336C = 3;
        D_800F336E = 0;
        D_800F3372 = 0;
        D_800F3374 = 0;
        D_800F3370 = value;
    }
        break;

    }
    return 0;
}
