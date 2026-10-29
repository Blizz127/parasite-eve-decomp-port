/* room_m0318i — func_801944E8, blob offset 0x5500, 0x5C4 bytes. NON_MATCHING
 * (behaviour C, proven by the e21 differential; not a byte match).
 * Sibling of the m0429i/m0269i particle callbacks, read from the retail
 * disassembly (517C.s). a0 = command, a1 = effect object.
 * cmd 1: state 0 integrates the velocity (+8/+A/+C) into the position
 * (+0/+2/+4) with 31/32 damping of vx/vz and a vy gravity step that is
 * negated once the object falls below D_800942EC (16 frames); state 1 is a
 * 24-frame timer; state 2 drifts y down by 8 for 16 frames.
 * cmd 2: state 0 draws the sprite (D_800F3368 block 0x10, +2 = 1) on a
 * sin-sized glow 0xDC at {0,0,-(f<<5)}; state 1 places the object at
 * velocity-vector * cos(f*1024/24) around its position, draws an 0x24 sprite
 * with a colour curve (D_80199674) and a 0x80 tint; state 2 draws the glow
 * 0x42 (growing, CLUT y 0x0 / +4).
 * Return: cmd 1 returns 1 when the timer ends, otherwise 0. */
typedef struct { short x, y, z, w; } GteShortVector;
typedef struct { unsigned char c[4]; } B4;

#define H(o, x) (*(unsigned short *)((char *)(o) + (x)))
#define SH(o, x) (*(short *)((char *)(o) + (x)))

extern GteShortVector D_8018F1F4;
extern B4 D_8018F1FC;
extern unsigned char D_80199674[];
extern short D_800942EC;
extern unsigned short D_800E11E8;
extern unsigned short D_800E11FA;
extern unsigned short D_800E1204[];
extern unsigned short D_800E1208;
extern unsigned short D_800E120A;
extern unsigned short D_800E2850[];
extern int D_800F3428;
extern struct { short a68, a6A; unsigned short a6C; short a6E; unsigned short a70; short a72, a74, a76, a78; } D_800F3368;

extern int func_80077CF4();
extern int func_80077DC4();
extern int func_80077AA4();
extern void func_800CFB7C(void *, short, void *);
extern void func_800CF3AC();
extern void func_800CEE20();

int func_801944E8(int mode, unsigned char *a1)
{
    GteShortVector g;
    GteShortVector v;
    B4 buf;
    int s1;
    int s2;
    int s3;
    int t;
    int clut;
    unsigned short k;
    int w;

    g = D_8018F1F4;
    buf = D_8018F1FC;
    switch (mode) {
    case 1:
        switch (SH(a1, 0x10)) {
        case 0:
            t = SH(a1, 8) * 31;
            H(a1, 0) += H(a1, 8);
            H(a1, 0x12)++;
            H(a1, 2) += H(a1, 0xA);
            H(a1, 4) += H(a1, 0xC);
            SH(a1, 8) = t / 32;
            SH(a1, 0xC) = SH(a1, 0xC) * 31 / 32;
            {
                short y = H(a1, 0xA) - 1;
                SH(a1, 0xA) = y;
                if (SH(a1, 2) >= D_800942EC)
                    SH(a1, 0xA) = -y;
            }
            if (SH(a1, 0x12) < 0x10)
                return 0;
            return 1;
        case 1:
            H(a1, 0x12)++;
            if (SH(a1, 0x12) < 0x18)
                return 0;
            return 1;
        case 2:
            H(a1, 0x12)++;
            H(a1, 2) -= 8;
            if (SH(a1, 0x12) < 0x10)
                return 0;
            return 1;
        }
        return 0;
    case 2:
        switch (SH(a1, 0x10)) {
        case 0:
            s1 = SH(a1, 0x12) << 6;
            t = func_80077CF4(s1);
            s2 = t / 2 + 0x800;
            s3 = func_80077DC4(s1) / 32;
            w = D_800E1208;
            D_800F3368.a6A = 1;
            D_800F3368.a68 = 0x10;
            D_800F3368.a76 = 0x10;
            D_800F3368.a78 = 0x10;
            g.z = -(SH(a1, 0x12) << 5);
            D_800F3368.a6C = 2;
            D_800F3368.a6E = 0;
            D_800F3368.a70 = D_800E2850[D_800E11E8];
            clut = func_80077AA4(0x30, w);
            func_800CEE20(a1, &g, s2, s2, 0xDC, clut & 0xFFFF, 1, s3, 0);
            return 0;
        case 1:
            s1 = (SH(a1, 0x12) << 10) / 24;
            t = func_80077DC4(s1);
            func_800CFB7C(a1 + 8, SH(a1, 6) * t / 4096, &v);
            v.x += H(a1, 0);
            v.y += H(a1, 2);
            v.z += H(a1, 4);
            s2 = func_80077CF4(s1);
            D_800F3368.a6A = 2;
            D_800F3368.a68 = 0x20;
            D_800F3368.a76 = 0x20;
            D_800F3368.a78 = 0x20;
            g.z = SH(a1, 0x12) << 5;
            D_800F3368.a6C = 3;
            D_800F3368.a6E = 1;
            D_800F3368.a70 = D_800E2850[D_800E11FA];
            func_800CF3AC(D_80199674, &buf, SH(a1, 0x12));
            k = D_800F3368.a6C;
            w = D_800E1204[k];
            if (k == 4 && D_800F3428 != 0)
                w += 8;
            else
                w += 4;
            clut = func_80077AA4(0, w);
            func_800CEE20(&v, &g, s2 / 2, s2 / 2, 0x24, clut & 0xFFFF, 1, 0x80, &buf);
            return 0;
        case 2:
            s1 = SH(a1, 0x12) << 6;
            s2 = func_80077CF4(s1) + 0x1000;
            s1 = func_80077DC4(s1);
            s3 = s1 / 32;
            w = D_800E120A;
            D_800F3368.a6A = 2;
            D_800F3368.a68 = 0x20;
            D_800F3368.a76 = 0x20;
            D_800F3368.a78 = 0x20;
            g.z = -(SH(a1, 0x12) << 5);
            D_800F3368.a6C = 3;
            D_800F3368.a6E = 1;
            D_800F3368.a70 = D_800E2850[D_800E11FA];
            clut = func_80077AA4(0, w + 4);
            func_800CEE20(a1, &g, s2, s2, 0x42, clut & 0xFFFF, 1, s3 / 2, 0);
            return 0;
        }
        return 0;
    }
    return 0;
}
