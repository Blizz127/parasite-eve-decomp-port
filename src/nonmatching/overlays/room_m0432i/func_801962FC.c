/* room_m0432i — func_801962FC, blob offset 0x7314, 0x258 bytes. NON_MATCHING
 * (behaviour C, proven by the e21 differential; not a byte match).
 * Effect-object callback, read from the retail disassembly.
 * cmd 1, state 0: frame counter (+0x12)++, +0xE += 0x20; done (1) at 0x20 frames.
 * cmd 2, state 0: places the sprite from func_800CF844 around a vector scaled by
 * the sine of the frame (<< 5), seeds the D_800F3368 draw block (+0/+0xE/+0x10 =
 * 0x20, +2 = 2, +4 = 3, +6 = 1, +8 = D_800E2850[D_800E11FA]), picks a colour via
 * func_800CF3AC and draws with func_800CEE20. The 8-byte seed buffer copied from
 * D_8018F1F4 has its halfword at +4 overwritten with +0xE - frame * 30. */
typedef struct { short x, y, z, w; } GteShortVector;
typedef struct { unsigned char c[4]; } B4;
typedef struct { unsigned char c[8]; } B8;

#define H(o, x) (*(unsigned short *)((char *)(o) + (x)))
#define SH(o, x) (*(short *)((char *)(o) + (x)))

extern B8 D_8018F1F4;
extern unsigned char D_8019944C[];
extern unsigned short D_800E11FA;
extern unsigned short D_800E2850[];
extern unsigned short D_800E1204[];
extern int D_800F3428;
extern struct { short a68, a6A; unsigned short a6C; short a6E; unsigned short a70; short a72, a74, a76, a78; } D_800F3368;

extern int func_80077CF4();
extern int func_80077DC4();
extern int func_80077AA4();
extern void func_800CF844();
extern void func_800CF3AC();
extern void func_800CEE20();

int func_801962FC(int mode, unsigned char *a1)
{
    B8 buf;
    GteShortVector v;
    B4 col;
    int s1;
    int r;
    int half;
    int w;
    int clut;

    buf = D_8018F1F4;
    switch (mode) {
    case 1:
        if (SH(a1, 0x10) != 0)
            return 0;
        H(a1, 0x12)++;
        H(a1, 0xE) += 0x20;
        if (SH(a1, 0x12) < 0x20)
            return 0;
        return 1;
    case 2:
        if (SH(a1, 0x10) != 0)
            return 0;
        s1 = SH(a1, 0x12) << 5;
        r = func_80077CF4(s1);
        func_800CF844(a1, &v, 200, a1 + 8, r * SH(a1, 6) / 4096, SH(a1, 0xE));
        r = func_80077DC4(s1);
        D_800F3368.a68 = 0x20;
        D_800F3368.a76 = 0x20;
        D_800F3368.a78 = 0x20;
        D_800F3368.a6A = 2;
        *(short *)((char *)&buf + 4) = H(a1, 0xE) - SH(a1, 0x12) * 30;
        D_800F3368.a70 = D_800E2850[D_800E11FA];
        D_800F3368.a6C = 3;
        D_800F3368.a6E = 1;
        half = r * 3 / 2;
        func_800CF3AC(D_8019944C, &col, SH(a1, 0x12));
        w = D_800E1204[D_800F3368.a6C];
        if (D_800F3368.a6C == 4 && D_800F3428 != 0)
            w += 10;
        else
            w += 6;
        clut = func_80077AA4(0, w);
        func_800CEE20(&v, &buf, half, half, D_800F3368.a6A + 0x64, clut & 0xFFFF, 1, 0x80, &col);
        return 0;
    }
    return 0;
}
