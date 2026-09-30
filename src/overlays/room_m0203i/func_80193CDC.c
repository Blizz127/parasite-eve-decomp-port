/* room_m0203i — func_80193CDC, blob offset 0x4CF4, 0x2E4 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl5 2026-09-27).
 * 3-mode effect controller; fresh local for the D_800F33E0 read before *a1 = 0 (e reuse: 3w). */

typedef struct { short vx, vy, vz, pad; } SVECTOR;
extern SVECTOR D_8018F218;
extern unsigned char *D_800E2368;
extern unsigned char *D_800F32D0;
extern unsigned char *D_800F33E0;
extern int D_800E27EC;
extern short D_800F3372;
extern short D_800F3374;
extern void func_80192F00();
extern int func_800D3FD8();
extern void func_800D3F64();
extern int func_800CE560();
extern void func_800CE8F0();
extern void func_800CE9D4();
extern unsigned char *func_800CE610();

#define SH(o, x) (*(short *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))

int func_80193CDC(int a0, int *a1)
{
    SVECTOR sv;
    SVECTOR p;
    SVECTOR q;
    unsigned char *e;
    unsigned char *r;
    unsigned char *w;

    sv = D_8018F218;
    switch (a0) {
    case 0:
        if (D_800E2368[0xD] != 0 && P(D_800F32D0, 8) != 0 && *(void **)P(D_800F32D0, 8) != 0) {
            r = P(*(void **)P(D_800F32D0, 8), 0x18);
            if (r[0] == 1) {
                r[0] = 2;
            }
        }
        func_800D3F64(0x59E, func_800D3FD8());
        w = D_800F33E0;
        *a1 = 0;
        return func_800CE560(P(w, 8), 0x14, 0x37, func_80192F00);
    case 1:
        func_800CE8F0(P(D_800F32D0, 8), 0x11, &sv, &p);
        func_800CE9D4(P(D_800F32D0, 8), 0, &q);
        q.vx -= 0x200;
        if (D_800E27EC % 3 == 1 && *a1 < 8) {
            e = func_800CE610(P(D_800F33E0, 8));
            if (e) {
                SH(e, 0) = p.vx;
                SH(e, 2) = p.vy;
                SH(e, 4) = p.vz;
                SH(e, 8) = q.vx;
                SH(e, 0xA) = q.vy;
                SH(e, 0xC) = q.vz;
                SH(e, 0x10) = 0;
                SH(e, 0x12) = 0;
            }
            e = func_800CE610(P(D_800F33E0, 8));
            if (e) {
                SH(e, 0) = p.vx;
                SH(e, 2) = p.vy;
                SH(e, 4) = p.vz;
                SH(e, 8) = q.vx;
                SH(e, 0xA) = q.vy;
                SH(e, 0xC) = q.vz;
                SH(e, 0x10) = 4;
                SH(e, 0x12) = 0;
            }
            (*a1)++;
        }
        if (D_800E27EC >= 8) {
            return 2;
        }
        break;
    case 2:
        D_800F3372 = 0;
        D_800F3374 = 4;
        break;
    }
    return 0;
}
