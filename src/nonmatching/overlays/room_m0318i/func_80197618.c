/* room_m0318i — func_80197618, blob offset 0x8630, 0x6A4 bytes. NON_MATCHING
 * (behaviour C, proven by the e21 differential; not a byte match).
 * Sibling of func_801944E8 / the m0429i particle callbacks, read from the
 * retail disassembly (7E64.s). a0 = command, a1 = effect object.
 * cmd 1: state 0 integrates the velocity (+8/+A/+C) into the position
 * (+0/+2/+4) with 50/51 damping of vx/vz and a vy gravity step that is
 * negated once the object is at or above D_800942EC (16 frames); state 1
 * snaps the object to the D_8019993C/3E/40 anchor and bumps the velocity
 * (+8 += 0x1D, +A += 0xB) for 24 frames; state 2 drifts y down by 1 for 24
 * frames.
 * cmd 2: state 0 draws the 0xD8 sprite on a glow sized by sin/cos of f<<6,
 * colour curve D_80199868; state 1 places a vector (velocity * cos(f*1024/24))
 * around the object position and draws a 6-sprite; state 2 draws the 0x42
 * glow (growing, CLUT x +4) with a NULL position vector.
 * Return: cmd 1 returns 1 when the timer ends, otherwise 0. */
typedef struct { short x, y, z, w; } GteShortVector;
typedef struct { unsigned char c[4]; } B4;

#define H(o, x) (*(unsigned short *)((char *)(o) + (x)))
#define SH(o, x) (*(short *)((char *)(o) + (x)))

extern GteShortVector D_8018F1F4;
extern B4 D_8018F240;
extern unsigned char D_80199868[];
extern unsigned short D_8019993C;
extern unsigned short D_8019993E;
extern unsigned short D_80199940;
extern short D_800942EC;
extern unsigned short D_800E11E8;
extern unsigned short D_800E11FA;
extern unsigned short D_800E1204[];
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

int func_80197618(int mode, unsigned char *a1)
{
    GteShortVector g;
    GteShortVector v;
    B4 buf;
    B4 cbuf;
    int s1;
    int s2;
    int s3;
    int t;
    int clut;
    unsigned short k;
    int w;

    g = D_8018F1F4;
    buf = D_8018F240;
    switch (mode) {
    case 1:
        switch (SH(a1, 0x10)) {
        case 0:
            t = SH(a1, 8) * 50;
            H(a1, 0) += H(a1, 8);
            H(a1, 0x12)++;
            H(a1, 2) += H(a1, 0xA);
            H(a1, 4) += H(a1, 0xC);
            SH(a1, 8) = t / 51;
            SH(a1, 0xC) = SH(a1, 0xC) * 50 / 51;
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
            H(a1, 0) = D_8019993C;
            H(a1, 2) = D_8019993E;
            H(a1, 8) += 0x1D;
            H(a1, 4) = D_80199940;
            H(a1, 0xA) += 0xB;
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
            s1 = SH(a1, 0x12) << 6;
            s3 = func_80077DC4(s1);
            t = func_80077CF4(s1);
            s2 = t / 32;
            D_800F3368.a68 = 0x10;
            D_800F3368.a76 = 0x10;
            D_800F3368.a78 = 0x10;
            D_800F3368.a6A = 1;
            g.z = -(SH(a1, 0x12) * 24);
            D_800F3368.a6C = 2;
            D_800F3368.a6E = 0;
            D_800F3368.a70 = D_800E2850[D_800E11E8];
            func_800CF3AC(D_80199868, &cbuf, SH(a1, 0x12) * 24 / 16);
            k = D_800F3368.a6C;
            w = D_800E1204[k];
            if (k == 4 && D_800F3428 != 0)
                w += 4;
            clut = func_80077AA4(0x20, w);
            func_800CEE20(a1, &g, s3, s3, 0xD8, clut & 0xFFFF, 1, s2, &cbuf);
            return 0;
        case 1:
            s1 = (SH(a1, 0x12) << 10) / 24;
            t = func_80077DC4(s1);
            func_800CFB7C(a1 + 8, SH(a1, 6) * t / 4096, &v);
            v.x += H(a1, 0);
            v.y += H(a1, 2);
            v.z += H(a1, 4);
            t = func_80077CF4(s1);
            s3 = t * 3 / 2;
            D_800F3368.a6A = 2;
            D_800F3368.a68 = 0x20;
            D_800F3368.a76 = 0x20;
            D_800F3368.a78 = 0x20;
            g.z = SH(a1, 0x12) << 5;
            D_800F3368.a6C = 3;
            D_800F3368.a6E = 1;
            D_800F3368.a70 = D_800E2850[D_800E11FA];
            func_800CF3AC(D_80199868, &cbuf, SH(a1, 0x12));
            k = D_800F3368.a6C;
            w = D_800E1204[k];
            if (k == 4 && D_800F3428 != 0)
                w += 7;
            else
                w += 3;
            clut = func_80077AA4(0, w);
            func_800CEE20(&v, &g, s3, s3, 6, clut & 0xFFFF, 1, 0x80, &cbuf);
            return 0;
        case 2:
            s1 = (SH(a1, 0x12) << 10) / 24;
            s3 = func_80077DC4(s1) + 0x1000;
            t = func_80077CF4(s1 * 2);
            s2 = t / 32;
            w = D_800E120A;
            D_800F3368.a6A = 2;
            D_800F3368.a68 = 0x20;
            D_800F3368.a76 = 0x20;
            D_800F3368.a78 = 0x20;
            D_800F3368.a70 = D_800E2850[D_800E11FA];
            D_800F3368.a6C = 3;
            D_800F3368.a6E = 1;
            clut = func_80077AA4(0, w + 4);
            func_800CEE20(a1, 0, s3, s3, 0x42, clut & 0xFFFF, 1, s2 / 2, &buf);
            return 0;
        }
        return 0;
    }
    return 0;
}
