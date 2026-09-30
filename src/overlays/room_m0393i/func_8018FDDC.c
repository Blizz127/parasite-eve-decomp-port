/* room_m0393i (PE.IMG room m0393i chunk 2, VRAM 0x8018EFE8)
 * func_8018FDDC — blob offset 0xdf4, 0x42c bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0065i func_8018FBCC; C re-targeted by symbol address
 * (docs/evidence/room_m0393i-ports-2026-09-23/REPORT.md). */

typedef struct { int vx, vy, vz, pad; } VECTOR;
typedef struct { short vx, vy, vz, pad; } SVECTOR;
extern int D_8009D248;
extern unsigned short D_8009D1CC;
extern unsigned char *func_800C2B50();
extern int func_80071A54();
extern int func_80077CF4();
extern int func_8001CAB0();
extern int func_800C6B90();

#define SB(o, x) (*(signed char *)((char *)(o) + (x)))
#define UB(o, x) (*(unsigned char *)((char *)(o) + (x)))
#define SH(o, x) (*(short *)((char *)(o) + (x)))
#define VX(x) (*(int *)((char *)o + i * 16 + (x)))
#define UH(o, x) (*(unsigned short *)((char *)(o) + (x)))

void func_8018FDDC(int a0, unsigned char *a1, unsigned char *o)
{
    unsigned char *r;
    unsigned int i;
    SVECTOR sv;

    r = func_800C2B50();
    for (i = 0; i < 3; i++) {
        if (SB(o + i, 0x1A) != 0 && --SB(o + i, 0x1A) == 0) {
            SB(o + i, 0x16) = 0;
        }
        if (SB(o + i, 0x16) == 0) {
            if (UB(o + i, 0x1E) < 0x80) {
                UB(o + i, 0x1E) += 4;
            } else {
                SB(o + i, 0x16) = 1;
            }
        }
        if (UB(o + i, 0xE) == 1) {
            VX(0x3C) += (SH(o + i * 8, 0x9C) * SH(o, 2)) >> 4;
            VX(0x40) += (SH(o + i * 8, 0x9E) * SH(o, 2)) >> 4;
            VX(0x44) += (SH(o + i * 8, 0xA0) * SH(o, 2)) >> 4;
        }
        if (UH(o + i * 2, 0x2A) < SH(o, 6)) {
            UH(o + i * 2, 0x2A) += func_80071A54() % 80 + 20;
            UH(o + i * 2, 0x32) += func_80071A54() % 40 + 40;
            VX(0x40) += -0xA0000;
        }
        UH(o + i * 2, 0x2A) += func_80077CF4(SH(a1, 2) << 5) >> 7;
        if (UH(o + i * 2, 0x22) < 0x14) {
            if (UB(o + i, 0x1E) >= 7) {
                UB(o + i, 0x1E) -= 6;
            }
        }
        if (UB(o + i, 0x12) == 0) {
            if (func_8001CAB0(SH(o + i * 16, 0x3E) << 16, SH(o + i * 16, 0x46) << 16, D_8009D248, D_8009D1CC) == 0) {
                UB(o + i, 0x12) = 1;
                UB(o + i, 0xE) = 0;
                if (UB(o, 0xA) == 0) {
                    UH(o + i * 2, 0x22) = 0x14;
                }
            }
        }
        if (--UH(o + i * 2, 0x22) == 0) {
            SB(o + i, 0x16) = -1;
            UH(o, 0) -= 1;
        }
        sv.vx = VX(0x3C) >> 16;
        sv.vy = VX(0x40) >> 16;
        sv.vz = VX(0x44) >> 16;
        SB(o, 0xB) = func_80071A54() % 11 - 5;
        SB(o, 0xC) = func_80071A54() % 11 - 5;
        SB(o, 0xD) = func_80071A54() % 11 - 5;
        if (func_800C6B90(&sv, UH(o + i * 2, 0x2A) >> 4)) {
            if (UB(o + i, 0x12) == 0) {
                SH(r, 0x28) = 1;
                UH(o + i * 2, 0x22) = 0x14;
                UB(o + i, 0xE) = 0;
            }
            UB(o + i, 0x12) = 1;
        }
    }
    SH(o, 2) += SH(o, 4);
    if (SH(o, 2) < 0) {
        SH(o, 2) = 0;
    }
    if (SH(o, 0) == 0) {
        a1[1] = 2;
    }
}
