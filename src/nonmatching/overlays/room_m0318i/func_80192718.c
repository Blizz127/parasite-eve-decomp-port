/* room_m0318i — func_80192718, blob offset 0x3730, 0x364 bytes. NON_MATCHING
 * (behaviour C, proven by the e21 differential; not a byte match).
 * Particle callback registered by func_80192A7C, a0 = mode, a1 = particle.
 *   mode 1: ++a1[4]; a1[8] = 700 * D_800E27EC / 36 + D_801998FA - 0x2BC;
 *           a1[6]/a1[0xA] = D_801998F8/FC + sin/cos(a1[2]) * a1[0xC] / 4096;
 *           a1[2] += 0x80; every 8th RNG tick spawns a trail point (+0/2/4 =
 *           a1[6/8/0xA]) from func_800CE610(D_80199900); returns
 *           D_800E27EC >= 0x24.
 *   mode 2: three-state sprite (a1[0]: 0 grow 12 frames, 1 hold 12 frames,
 *           2 fall with a1[0xC] += a1[4] << 2); draws it through func_800CEE20
 *           at a1[6/8/0xA] with CLUT from D_800E1204[D_800F336C]; returns 0.
 * inline GTE/asm: none used here. */
typedef struct { short x, y, z, w; } GteShortVector;
typedef struct { unsigned char c[4]; } B4;

#define H(o, x) (*(unsigned short *)((char *)(o) + (x)))
#define SH(o, x) (*(short *)((char *)(o) + (x)))

extern B4 D_8018F1F0;
extern int D_800E27EC;
extern unsigned short D_801998F8;
extern unsigned short D_801998FA;
extern unsigned short D_801998FC;
extern int D_80199900;
extern unsigned short D_800F336C;
extern unsigned short D_800E1204[];
extern int D_800F3428;

extern int func_80077DC4();
extern int func_80077CF4();
extern int func_80077AA4();
extern int func_80071A54();
extern unsigned short *func_800CE610();
extern void func_800CEE20();

int func_80192718(int mode, unsigned char *a1)
{
    GteShortVector v;
    GteShortVector g;
    B4 buf;
    int s1;
    int s2;
    int t;
    int w;
    int clut;
    unsigned short *p;

    buf = D_8018F1F0;
    if (mode == 1) {
        H(a1, 4)++;
        SH(a1, 8) = D_800E27EC * 700 / 36 + (D_801998FA - 0x2BC);
        t = func_80077DC4(SH(a1, 2));
        SH(a1, 6) = D_801998F8 + t * SH(a1, 0xC) / 4096;
        t = func_80077CF4(SH(a1, 2));
        SH(a1, 0xA) = D_801998FC + t * SH(a1, 0xC) / 4096;
        H(a1, 2) += 0x80;
        if ((func_80071A54() & 7) == 0) {
            p = func_800CE610(D_80199900);
            if (p != 0) {
                p[0] = H(a1, 6);
                p[1] = H(a1, 8);
                p[2] = H(a1, 0xA);
            }
        }
        if (D_800E27EC < 0x24)
            return 0;
        return 1;
    }
    if (mode == 2) {
        s2 = 0x40;
        if (SH(a1, 0) == 0) {
            s1 = func_80077CF4((SH(a1, 4) << 10) / 12);
            if (SH(a1, 4) >= 0xC) {
                SH(a1, 4) = 0;
                SH(a1, 0) = 1;
                goto draw;
            }
        } else if (SH(a1, 0) == 1) {
            s1 = 0x1000;
            if (SH(a1, 4) >= 0xC) {
                SH(a1, 4) = 0;
                SH(a1, 0) = 2;
                goto draw;
            }
        } else {
            s1 = 0x1000;
            s2 = func_80077DC4((SH(a1, 4) << 10) / 12) / 64;
            H(a1, 0xC) += SH(a1, 4) << 2;
        }
draw:
        v.x = H(a1, 6);
        v.y = H(a1, 8);
        w = D_800F336C;
        s1 = s1 * 3 / 2;
        v.z = H(a1, 0xA);
        g.x = 0;
        g.y = 0;
        g.w = 0;
        g.z = D_800E27EC << 7;
        t = D_800E1204[w];
        if (w == 4 && D_800F3428 != 0)
            t += 9;
        else
            t += 5;
        clut = func_80077AA4(0, t);
        func_800CEE20(&v, &g, s1, s1, 0x24, clut & 0xFFFF, 1, s2, &buf);
        return 0;
    }
    return 0;
}
