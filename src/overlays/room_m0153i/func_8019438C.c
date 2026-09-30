/* room_m0153i (PE.IMG room m0153i chunk 2, VRAM 0x8018EFE8)
 * func_8019438C — blob offset 0x53a4, 0x344 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0145i func_80192E78; C re-targeted by symbol address
 * (docs/evidence/room_m0153i-ports-2026-09-23/REPORT.md). */

#define B(o, x) (*(unsigned char *)((char *)(o) + (x)))
#define H(o, x) (*(unsigned short *)((char *)(o) + (x)))
#define W(o, x) (*(unsigned int *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))
extern void *D_800F32D0;
extern void *D_800F33E0;
extern void *D_800E2368;
extern void *D_800B0E64;
extern void **D_8009D254;
extern int D_800E27EC;
extern volatile unsigned short D_800942EC;
extern unsigned short D_800E11EA;
extern unsigned short D_800E2850[];
extern short D_800F3368, D_800F336A, D_800F336C, D_800F336E, D_800F3370, D_800F3372,
    D_800F3374, D_800F3376, D_800F3378;
extern void func_801941F4();
extern int func_800CE560();
extern void func_800CE870();
extern int func_800D3FD8();
extern void func_8006DF50();
extern unsigned short *func_800CE610();

int func_8019438C(int a0, unsigned char *a1)
{
    void *x;
    unsigned char *p;
    volatile unsigned short *q;
    void **r;
    void **pp;
    void *g;
    unsigned short e;

    switch (a0) {
    case 0:
        func_800CE870(P(D_800F32D0, 8), 0, a1 + 4);
        x = D_800E2368;
        H(a1, 0) = 0;
        H(a1, 2) = 0;
        if (B(x, 0xD) != 0) {
            x = P(D_800F32D0, 8);
            if (x != 0) {
                x = P(x, 0);
                if (x != 0) {
                    p = P(x, 0x18);
                    if (*p == 1) {
                        *p = 2;
                    }
                }
            }
        }
        pp = &D_800B0E64;
        if (*pp != 0) {
            func_8006DF50(*pp, 0x588, func_800D3FD8(), 0x80, 0x7F);
        }
        return func_800CE560(P(D_800F33E0, 8), 0xC, 8, func_801941F4);
    case 1:
        if (D_800E27EC < 0x20 && D_800E27EC % 6 == 0) {
            q = func_800CE610(P(D_800F33E0, 8));
            if (q != 0) {
                q[0] = H(a1, 4);
                q[1] = H(a1, 6);
                q[2] = H(a1, 8);
                e = D_800942EC;
                q[4] = 0;
                q[5] = 0;
                q[1] = e;
            }
        }
        if (D_800E27EC == 7 && B(D_800E2368, 0xD) != 0 &&
            (W(P(P(g = D_800F32D0, 8), 0), 0) & 0x3F000000) == 0x01000000) {
            W(*D_8009D254, 0x4C) |= 0x4000;
            r = P(P(g, 8), 0);
            W(r, 0) = (W(r, 0) & 0xC0FFFFFF) | 0x21000000;
            r = P(P(g, 8), 0);
            W(r, 0) |= 0x80000000;
        }
        if (D_800E27EC >= 8) {
            return 2;
        }
        break;
    case 2:
        D_800F3368 = 0x20;
        D_800F336A = 2;
        D_800F3376 = 0x20;
        D_800F3378 = 0x20;
        D_800F336C = 3;
        D_800F336E = 0;
        D_800F3372 = 0;
        D_800F3374 = 0;
        D_800F3370 = D_800E2850[D_800E11EA];
        break;
    }
    return 0;
}
