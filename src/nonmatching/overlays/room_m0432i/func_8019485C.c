/* room_m0432i — func_8019485C, blob offset 0x5874, 0x704 bytes. NON_MATCHING
 * (behaviour C, proven by the e21 differential; not a byte match).
 * Effect-object callback, cmd 1 (integrate: x/y/z += velocity, velocity decays
 * by 127/128, state 0 also bounces y against D_800942EC and adds a sin/cos
 * wobble; frame limits 0x20 / 0x10 / 8 for states 0 / 1 / 2) and cmd 2 (draw
 * the sprite through func_800CEE20 with a per-state D_800F3368 block).
 * Read from the retail disassembly. */

typedef struct { unsigned char c[4]; } B4;

#define H(o, x) (*(unsigned short *)((char *)(o) + (x)))
#define SH(o, x) (*(short *)((char *)(o) + (x)))

extern B4 D_8018F204;
extern unsigned char D_80199388[];
extern short D_800942EC;
extern unsigned short D_800E11EA;
extern unsigned short D_800E11FA;
extern unsigned short D_800E120A;
extern unsigned short D_800E2850[];
extern unsigned short D_800E1204[];
extern int D_800F3428;
extern short D_800F3368;
extern short D_800F336A;
extern unsigned short D_800F336C;
extern short D_800F336E;
extern unsigned short D_800F3370;
extern short D_800F3376;
extern short D_800F3378;

extern int func_80077CF4();
extern int func_80077DC4();
extern int func_80077AA4();
extern void func_800CF3AC();
extern void func_800CEE20();

/* shared integrate step: position += velocity, frame++, velocity damped */
#define INTEGRATE_POS(a1) \
    do { \
        H(a1, 0) += H(a1, 8); \
        H(a1, 0x12)++; \
        H(a1, 2) += H(a1, 0xA); \
        H(a1, 4) += H(a1, 0xC); \
        SH(a1, 8) = SH(a1, 8) * 127 / 128; \
    } while (0)

int func_8019485C(int mode, unsigned char *a1)
{
    short v[4];
    B4 buf;
    int t;
    int s0;
    int s2;
    int a3;
    int w;
    int clut;

    buf = D_8018F204;
    switch (mode) {
    case 1:
        switch (SH(a1, 0x10)) {
        case 0: {
            unsigned short ya;
            INTEGRATE_POS(a1);
            SH(a1, 0xC) = SH(a1, 0xC) * 127 / 128;
            ya = H(a1, 0xA);
            H(a1, 0xA) = ya;
            if (SH(a1, 2) >= D_800942EC)
                SH(a1, 0xA) = -(short)ya;
            t = func_80077CF4(SH(a1, 0x12) << 8);
            H(a1, 0) += t * 40 / 4096;
            t = func_80077DC4(SH(a1, 0x12) << 8);
            H(a1, 2) += t * 40 / 4096;
            if (SH(a1, 0x12) < 0x20)
                return 0;
            return 1;
        }
        case 1:
            INTEGRATE_POS(a1);
            SH(a1, 0xA) = SH(a1, 0xA) * 127 / 128;
            SH(a1, 0xC) = SH(a1, 0xC) * 127 / 128;
            if (SH(a1, 0x12) < 0x10)
                return 0;
            return 1;
        case 2:
            INTEGRATE_POS(a1);
            SH(a1, 0xA) = SH(a1, 0xA) * 127 / 128;
            SH(a1, 0xC) = SH(a1, 0xC) * 127 / 128;
            if (SH(a1, 0x12) < 8)
                return 0;
            return 1;
        }
        return 0;
    case 2:
        switch (SH(a1, 0x10)) {
        case 0:
            t = func_80077DC4(SH(a1, 0x12) << 5);
            s0 = t / 2 + 0x800;
            D_800F3368 = 0x20;
            D_800F336A = 2;
            D_800F3376 = 0x20;
            D_800F3378 = 0x20;
            D_800F336C = 3;
            D_800F336E = 1;
            v[0] = 0;
            v[1] = 0;
            D_800F3370 = D_800E2850[D_800E11FA];
            v[3] = 0;
            v[2] = SH(a1, 0x12) << 5;
            func_800CF3AC(D_80199388, &buf, SH(a1, 0x12));
            w = D_800E1204[D_800F336C];
            if (D_800F336C == 4 && D_800F3428 != 0)
                w += 8;
            else
                w += 4;
            clut = func_80077AA4(0, w);
            func_800CEE20(a1, v, s0, s0, D_800F336A + 0x64, clut & 0xFFFF, 1, 0x80, &buf);
            return 0;
        case 1:
            s0 = func_80077CF4(SH(a1, 0x12) << 6) + 0x800;
            a3 = func_80077DC4(SH(a1, 0x12) * 1204 / 16);
            if (a3 < 0)
                a3 += 0x7F;
            D_800F3368 = 0x40;
            D_800F336A = 4;
            D_800F3376 = 0x40;
            D_800F3378 = 0x40;
            D_800F336C = 3;
            D_800F336E = 1;
            v[0] = 0;
            v[1] = 0;
            D_800F3370 = D_800E2850[D_800E11FA];
            v[3] = 0;
            v[2] = SH(a1, 0x12) * 60;
            s2 = a3 >> 7;
            func_800CF3AC(D_80199388, &buf, (SH(a1, 0x12) << 5) / 12, a3);
            w = D_800E1204[D_800F336C];
            if (D_800F336C == 4 && D_800F3428 != 0)
                w += 6;
            else
                w += 2;
            clut = func_80077AA4(0, w);
            func_800CEE20(a1, v, s0 * 2, s0 * 2, 0x40, clut & 0xFFFF, 1, s2, &buf);
            return 0;
        case 2:
            D_800F3368 = 0x20;
            D_800F336A = 2;
            D_800F3376 = 0x20;
            D_800F3378 = 0x20;
            D_800F336C = 3;
            D_800F336E = 0;
            v[0] = 0;
            v[1] = 0;
            D_800F3370 = D_800E2850[D_800E11EA];
            v[3] = 0;
            v[2] = SH(a1, 0x12) * 60;
            clut = func_80077AA4(0, D_800E120A);
            func_800CEE20(a1, v, 0x1800, 0x1800, D_800F336A * SH(a1, 0x12) + 0x80, clut & 0xFFFF, 1, 0x40, 0);
            return 0;
        }
        return 0;
    }
    return 0;
}
