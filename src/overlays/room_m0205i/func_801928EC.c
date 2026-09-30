/* room_m0205i — func_801928EC, blob offset 0x3904, 0x328 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl5 2026-09-27).
 * 3-mode effect controller; case-1 hA via in-struct store so D_800E27EC load hoists above it; D_800F3368 block as one struct (scalar externs: 14w) + climb. */

typedef struct { short vx, vy, vz, pad; } SVECTOR;
extern SVECTOR D_8018F1D0;
extern unsigned char *D_800E2368;
extern unsigned char *D_800F32D0;
extern unsigned char *D_800F33E0;
extern int D_800E27EC;
extern struct { short a68, a6A, a6C, a6E, a70, a72, a74, a76, a78; } D_800F3368;
extern unsigned short D_800E11E8;
extern unsigned short D_800E2850[];
extern void func_801924F4();
extern int func_800D3FD8();
extern int func_800D3F64();
extern int func_800CE560();
extern void func_800CE8F0();
extern void func_800CE9D4();
extern unsigned char *func_800CE610();
extern int func_80077CF4();
extern int func_80071A54();
extern void func_800866A4();

#define SH(o, x) (*(short *)((char *)(o) + (x)))
typedef struct { short h0, h2, h4, h6, h8, hA, hC; } OB;
#define P(o, x) (*(void **)((char *)(o) + (x)))

int func_801928EC(int a0, unsigned char *o)
{
    SVECTOR sv;
    SVECTOR p;
    SVECTOR q;
    unsigned char *e;
    unsigned char *r;
    unsigned char *w;
    int t;
    int v;

    sv = D_8018F1D0;
    switch (a0) {
    case 0:
        v = func_800D3F64(0x5A1, func_800D3FD8());
        w = D_800F33E0;
        SH(o, 0xC) = v;
        return func_800CE560(P(w, 8), 0x18, 0x18, func_801924F4);
    case 1:
        ((OB *)o)->hA += 1;
        if (D_800E27EC < 0x47) {
            func_800CE8F0(P(D_800F32D0, 8), 9, &sv, &p);
            t = func_80077CF4((D_800E27EC << 11) / 70) / 32;
            if (D_800E27EC % 3 == 0) {
                e = func_800CE610(P(D_800F33E0, 8));
                if (e) {
                    func_800CE9D4(P(D_800F32D0, 8), 0, &q);
                    SH(e, 0x10) = q.vy;
                    SH(e, 0) = p.vx;
                    SH(e, 2) = p.vy;
                    SH(e, 4) = p.vz;
                    SH(e, 0x12) = 0;
                    SH(e, 0x16) = 0;
                    SH(e, 0x14) = 0;
                    SH(e, 6) = func_80071A54();
                    SH(e, 0xE) = t;
                }
                if (D_800E2368[0xD] != 0 && P(D_800F32D0, 8) != 0 && *(void **)P(D_800F32D0, 8) != 0) {
                    r = P(*(void **)P(D_800F32D0, 8), 0x18);
                    if (r[0] == 1) {
                        r[0] = 2;
                    }
                }
            }
        }
        if (D_800E27EC == 0x46 && SH(o, 0xC) != -1) {
            func_800866A4(SH(o, 0xC), 0);
        }
        if (D_800E27EC >= 8) {
            return 2;
        }
        break;
    case 2:
        D_800F3368.a68 = 0x10;
        D_800F3368.a6A = 1;
        D_800F3368.a76 = 0x10;
        D_800F3368.a78 = 0x10;
        D_800F3368.a70 = D_800E2850[D_800E11E8];
        D_800F3368.a6C = 2;
        D_800F3368.a6E = 0;
        D_800F3368.a72 = 0;
        D_800F3368.a74 = 8;
        break;
    }
    return 0;
}
