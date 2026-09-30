/* room_m0174i — func_80191914, blob offset 0x292C, 0x428 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl5 2026-09-27).
 * Spark particle update; sv2 declared before sv (frame layout); per-particle ints read as W>>16 give lh +2. */

typedef struct { short vx, vy, vz, pad; } SVECTOR;
extern int D_8009D248;
extern unsigned short D_8009D1CC;
extern short D_800942EC;
extern unsigned char D_80196E14[];
extern unsigned char D_80196D6C[];
extern unsigned char *func_800C2B50();
extern unsigned char *func_800C2B90();
extern int func_80077CF4();
extern int func_80077DC4();
extern int func_8001CAB0();
extern int func_800C6B90();

#define SH(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))

void func_80191914(void *a0, unsigned char *a1, unsigned char *o)
{
    unsigned char *x;
    unsigned char *p;
    unsigned int i;
    SVECTOR sv2;
    SVECTOR sv;

    x = func_800C2B50();
    W(o, 0xE0) += W(x, 0x54) << 8;
    W(o, 0xE4) += (W(o, 0xE4) * W(x, 0x50)) >> 12;
    for (i = 0; i < SH(o, 0xE8); i++) {
        if ((o + i)[0xC8] == 1) {
            SH(o + i * 2, 0x88) += 100;
            SH(o + i * 2, 0x98) += 20;
            W(o + i * 4, 0xA8) += W(o, 0xE4);
            W(o + i * 16, 8) = ((func_80077CF4(SH(o + i * 2, 0x98)) * (W(o + i * 4, 0xA8) >> 12)) >> 12) << 16;
            W(o + i * 16, 0xC) += W(o, 0xE0);
            W(o + i * 16, 0x10) = ((func_80077DC4(SH(o + i * 2, 0x98)) * (W(o + i * 4, 0xA8) >> 12)) >> 12) << 16;
            sv.vx = SH(o, 0) + (W(o + i * 16, 8) >> 16);
            sv.vy = SH(o, 2) + (W(o + i * 16, 0xC) >> 16);
            sv.vz = SH(o, 4) + (W(o + i * 16, 0x10) >> 16);
            if (func_8001CAB0(sv.vx << 16, sv.vz << 16, D_8009D248, D_8009D1CC) == 0) {
                (o + i)[0xC8] = 0;
                if (--SH(o, 0xEC) == 0) {
                    a1[1] = 2;
                }
                p = func_800C2B90(a0, 6, D_80196E14, D_80196D6C);
                if (p) {
                    *(SVECTOR *)p = sv;
                }
                p = func_800C2B90(a0, 6, D_80196E14, D_80196D6C);
                if (p) {
                    *(SVECTOR *)p = sv;
                }
            }
            if (SH(o + i * 16, 0xE) + SH(o, 2) > D_800942EC) {
                (o + i)[0xC8] = 2;
                p = func_800C2B90(a0, 6, D_80196E14, D_80196D6C);
                if (p) {
                    *(SVECTOR *)p = sv;
                }
                p = func_800C2B90(a0, 6, D_80196E14, D_80196D6C);
                if (p) {
                    *(SVECTOR *)p = sv;
                }
            }
        }
        if ((o + i)[0xC8] >= 2) {
            (o + i)[0xC8]++;
            SH(o + i * 2, 0xD0) -= 8;
            if (SH(o + i * 2, 0xD0) < 0) {
                SH(o + i * 2, 0xD0) = 0;
            }
            if ((o + i)[0xC8] >= 11) {
                if ((o + i)[0xC8] == 20) {
                    if (--SH(o, 0xEC) == 0) {
                        a1[1] = 2;
                    }
                }
            }
        }
        if ((o + i)[0xC8] != 0) {
            sv2.vx = SH(o, 0) + (W(o + i * 16, 8) >> 16);
            sv2.vy = SH(o, 2) + (W(o + i * 16, 0xC) >> 16);
            sv2.vz = SH(o, 4) + (W(o + i * 16, 0x10) >> 16);
            if (func_800C6B90(&sv2, 200)) {
                SH(x, 0x4C) = 1;
            }
        }
    }
}
