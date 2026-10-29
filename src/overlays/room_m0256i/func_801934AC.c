/* room_m0256i — func_801934AC, blob offset 0x44C4, 0x2D0 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (2026-10-08, written from the retail asm).
 * 3-mode effect controller (arm / emit drifting particles / palette setup),
 * the next m0256i effect callback after func_80193BFC. */

typedef struct { short vx, vy, vz, pad; } SVECTOR;
extern SVECTOR D_8018F208;
extern unsigned char *D_800E2368;
extern unsigned char *D_800F32D0;
extern unsigned char *D_800F33E0;
extern int D_800E27EC;
extern volatile unsigned short D_800E11EA;
extern unsigned short D_800E2850[];
extern volatile short D_800F3368;
extern volatile short D_800F336A;
extern volatile short D_800F336C;
extern volatile short D_800F336E;
extern volatile short D_800F3370;
extern volatile short D_800F3372;
extern volatile short D_800F3374;
extern volatile short D_800F3376;
extern volatile short D_800F3378;
extern void func_80192844();
extern int func_800CE560();
extern void func_800CE8F0();
extern void func_800CE9D4();
extern unsigned char *func_800CE610();
extern int func_80071A54();
extern void func_800CFB7C();

int func_801934AC(int a0, short *a1)
{
    SVECTOR sv;
    SVECTOR p;
    SVECTOR q;
    SVECTOR v;
    unsigned char *e;
    unsigned char *s, *pp, *qq, *rr;
    int n;
    int r;
    short t;

    sv = D_8018F208;
    switch (a0) {
    case 0:
        s = D_800E2368;
        *a1 = 0;
        if (s[0xD]) {
            pp = *(unsigned char **)(D_800F32D0 + 8);
            if (pp) {
                qq = *(unsigned char **)pp;
                if (qq) {
                    rr = *(unsigned char **)(qq + 0x18);
                    if (*rr == 1) *rr = 2;
                }
            }
        }
        return func_800CE560(*(void **)(D_800F33E0 + 8), 0x14, 0x32, func_80192844);
    case 1:
        func_800CE8F0(*(void **)(D_800F32D0 + 8), 4, &sv, &p);
        func_800CE9D4(*(void **)(D_800F32D0 + 8), 4, &q);
        q.vx -= 0xDC;
        if (D_800E27EC < 0x6E && (D_800E27EC & 1)) {
            e = func_800CE610(*(void **)(D_800F33E0 + 8));
            if (e) {
                n = (func_80071A54() & 7) + 0x37;
                ((short *)e)[0] = p.vx;
                ((short *)e)[1] = p.vy;
                ((short *)e)[2] = p.vz;
                v.vx = q.vx; v.vy = q.vy; v.vz = q.vz;
                r = func_80071A54() & 0x1F; t = v.vy - 0x10; v.vy = t + r;
                func_800CFB7C(&v, n, e + 8);
                ((short *)e)[3] = *a1 & 1;
                ((short *)e)[8] = 0;
                ((short *)e)[9] = 0;
            }
            (*a1)++;
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
        asm volatile("" : : : "memory");
        D_800F336C = 3;
        D_800F336E = 0;
        D_800F3372 = 0;
        D_800F3374 = 8;
        D_800F3370 = value;
    }
        break;
    }
    return 0;
}
