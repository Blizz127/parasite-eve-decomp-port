/* room_m0318i — func_80198268, blob offset 0x9280, 0x690 bytes. NON_MATCHING
 * (behaviour C, proven by the e21 differential; not a byte match).
 * Sibling of func_801944E8 in the same room (and of the m0429i/m0269i
 * particle callbacks), read from the retail disassembly (7E64.s).
 * a0 = command, a1 = effect object.
 * cmd 1: state 0 integrates the velocity (+8/+A/+C) into the position
 * (+0/+2/+4) with 50/51 damping of vx/vz and a vy gravity step that is
 * negated once the object falls below D_800942EC (16 frames); state 1 adds
 * 0x32 to +0xE for 24 frames; state 2 drifts y down by 1 for 24 frames.
 * cmd 2: state 0 draws the sprite on a sin-sized glow at {x, y, -(f<<5)}
 * (D_800F3368 block 0x20, +2 = 2) with a colour curve D_80199890; state 1
 * places the glow at velocity-vector * sin(f*64) / 4096 (func_800CF844) and
 * draws it 3/2 sin-sized with size 4 and a 0x80 tint; state 2 draws a cos
 * sized sprite (block 0x40, +2 = 4) with a 1/32 cos tint and sets
 * D_800F3374 = 4 afterwards.
 * Return: cmd 1 returns 1 when the timer ends, otherwise 0. */
typedef struct { short x, y, z, w; } GteShortVector;
typedef struct { unsigned char c[4]; } B4;

#define H(o, x) (*(unsigned short *)((char *)(o) + (x)))
#define SH(o, x) (*(short *)((char *)(o) + (x)))

extern GteShortVector D_8018F1F4;
extern unsigned char D_80199890[];
extern short D_800942EC;
extern unsigned short D_800E11E8;
extern unsigned short D_800E11FA;
extern unsigned short D_800E1204[];
extern unsigned short D_800E2850[];
extern int D_800F3428;
extern struct { short a68, a6A; unsigned short a6C; short a6E; unsigned short a70; short a72, a74, a76, a78; } D_800F3368;

extern int func_80077CF4();
extern int func_80077DC4();
extern int func_80077AA4();
extern void func_800CF3AC();
extern void func_800CF844();
extern void func_800CEE20();

int func_80198268(int mode, unsigned char *a1)
{
    GteShortVector g;
    GteShortVector v;
    B4 buf;
    int s1;
    int s2;
    int t;
    int clut;
    unsigned short k;
    int w;

    g = D_8018F1F4;
    switch (mode) {
    case 1:
        switch (SH(a1, 0x10)) {
        case 0:
            H(a1, 0x12)++;
            t = SH(a1, 8) * 50;
            H(a1, 0) += H(a1, 8);
            H(a1, 2) += H(a1, 0xA);
            H(a1, 4) += H(a1, 0xC);
            SH(a1, 8) = t / 51;
            {
                short y = H(a1, 0xA) - 1;
                SH(a1, 0xA) = y;
                SH(a1, 0xC) = SH(a1, 0xC) * 50 / 51;
                if (SH(a1, 2) >= D_800942EC)
                    SH(a1, 0xA) = -y;
            }
            if (SH(a1, 0x12) < 0x10)
                return 0;
            return 1;
        case 1:
            H(a1, 0x12)++;
            H(a1, 0xE) += 0x32;
            if (SH(a1, 0x12) < 0x18)
                return 0;
            return 1;
        case 2:
            H(a1, 0x12)++;
            H(a1, 2) -= 1;
            if (SH(a1, 0x12) < 0x18)
                return 0;
            return 1;
        }
        return 0;
    case 2:
        switch (SH(a1, 0x10)) {
        case 0:
            t = func_80077CF4(SH(a1, 0x12) << 6);
            s2 = t / 2 + 0x1000;
            D_800F3368.a68 = 0x20;
            D_800F3368.a76 = 0x20;
            D_800F3368.a78 = 0x20;
            D_800F3368.a6A = 2;
            g.z = -(SH(a1, 0x12) << 5);
            D_800F3368.a6C = 2;
            D_800F3368.a6E = 0;
            D_800F3368.a70 = D_800E2850[D_800E11E8];
            func_800CF3AC(D_80199890, &buf, SH(a1, 0x12));
            k = D_800F3368.a6C;
            w = D_800E1204[k];
            if (k == 4 && D_800F3428 != 0)
                w += 4;
            clut = func_80077AA4(0x30, w);
            func_800CEE20(a1, &g, s2, s2,
                          D_800F3368.a6A * (SH(a1, 0x12) / 2) + 0xA0,
                          clut & 0xFFFF, 1, 0x80, &buf);
            return 0;
        case 1:
            s1 = SH(a1, 0x12) << 6;
            t = func_80077CF4(s1);
            func_800CF844(a1, &v, 0xC8, a1 + 8, t * SH(a1, 6) / 4096, SH(a1, 0xE));
            s2 = func_80077DC4(s1) * 3 / 2;
            D_800F3368.a6A = 2;
            D_800F3368.a68 = 0x20;
            D_800F3368.a76 = 0x20;
            D_800F3368.a78 = 0x20;
            g.z = H(a1, 0xE) + (SH(a1, 0x12) << 5);
            D_800F3368.a6C = 3;
            D_800F3368.a6E = 1;
            D_800F3368.a70 = D_800E2850[D_800E11FA];
            func_800CF3AC(D_80199890, &buf, SH(a1, 0x12) * 3 / 2);
            k = D_800F3368.a6C;
            w = D_800E1204[k];
            if (k == 4 && D_800F3428 != 0)
                w += 7;
            else
                w += 3;
            clut = func_80077AA4(0, w);
            func_800CEE20(&v, &g, s2, s2, 4, clut & 0xFFFF, 1, 0x80, &buf);
            return 0;
        case 2:
            s1 = (SH(a1, 0x12) << 10) / 24;
            s2 = func_80077DC4(s1) + 0x1000;
            t = func_80077DC4(s1);
            D_800F3368.a68 = 0x40;
            D_800F3368.a6A = 4;
            D_800F3368.a76 = 0x40;
            D_800F3368.a78 = 0x40;
            D_800F3368.a6C = 3;
            D_800F3368.a6E = 1;
            D_800F3368.a74 = 0x20;
            D_800F3368.a70 = D_800E2850[D_800E11FA];
            s1 = t / 32;
            func_800CF3AC(D_80199890, &buf, SH(a1, 0x12));
            k = D_800F3368.a6C;
            w = D_800E1204[k];
            if (k == 4 && D_800F3428 != 0)
                w += 6;
            else
                w += 2;
            clut = func_80077AA4(0, w);
            func_800CEE20(a1, 0, s2, s2, 0, clut & 0xFFFF, 1, s1 / 2, &buf);
            D_800F3368.a74 = 4;
            return 0;
        }
        return 0;
    }
    return 0;
}
