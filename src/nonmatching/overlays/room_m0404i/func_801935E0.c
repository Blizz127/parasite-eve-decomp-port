/* room_m0404i — func_801935E0, blob offset 0x45F8, 0x48C bytes. NON_MATCHING
 * (behaviour C, proven by the e21 differential; not a byte match).
 * Effect-object callback a0 = mode (1 step, 2 draw), a1 = object, state at
 * +0x10, frame at +0x12.
 * mode 1 state 0: ballistic step (vx/vz decay 255/256, vy gravity +1 with a
 * bounce off the floor D_800942EC), position z/x advanced by the sine/cosine
 * of frame << 9 / 512; ends at frame 0x40. State 1: same with 127/128 decay,
 * gravity +2 and a plain bounce, ends at frame 0x20.
 * mode 2 state 0: SetRot/SetTrans from D_800BCFA4[0] and two func_800D2104
 * draws (object position, then position with y + 8). State 1: sets the
 * D_800F3368 draw block, picks a colour from the D_80193F68 curve and draws
 * the sprite through func_800CEE20.
 * inline GTE/asm: Psy-Q inline_c.h-shaped GTE (COP2) SetRot/SetTrans macros. */
#define gte_SetRotMatrix(r0) __asm__ volatile ( \
    "lw $12,0(%0)\n\tlw $13,4(%0)\n\tctc2 $12,$0\n\tctc2 $13,$1\n\t" \
    "lw $12,8(%0)\n\tlw $13,12(%0)\n\tlw $14,16(%0)\n\t" \
    "ctc2 $12,$2\n\tctc2 $13,$3\n\tctc2 $14,$4" : : "r"(r0) : "$12", "$13", "$14")
#define gte_SetTransMatrix(r0) __asm__ volatile ( \
    "lw $12,20(%0)\n\tlw $13,24(%0)\n\tctc2 $12,$5\n\tlw $14,28(%0)\n\t" \
    "ctc2 $13,$6\n\tctc2 $14,$7" : : "r"(r0) : "$12", "$13", "$14")

typedef struct { short x, y, z, w; } GteShortVector;
typedef struct { unsigned char c[4]; } B4;
typedef struct { short m[3][3]; int t[3]; } MATRIX;

#define H(o, x) (*(unsigned short *)((char *)(o) + (x)))
#define SH(o, x) (*(short *)((char *)(o) + (x)))

extern GteShortVector D_8018F210;
extern B4 D_8018F218;
extern unsigned char D_80193F68[];
extern short D_800942EC;
extern unsigned short D_800E1204[];
extern int D_800E27EC;
extern int D_800F3428;
extern MATRIX *D_800BCFA4[];
extern struct { short a68, a6A; unsigned short a6C; short a6E; unsigned short a70; short a72, a74, a76, a78; } D_800F3368;

extern int func_80077CF4();
extern int func_80077DC4();
extern int func_80077AA4();
extern void func_800CF3AC();
extern void func_800CEE20();
extern void func_800D2104();

int func_801935E0(int mode, unsigned char *a1)
{
    GteShortVector g;
    GteShortVector p;
    B4 col;
    B4 buf;
    int t;
    int size;
    int w;
    int clut;

    g = D_8018F210;
    buf = D_8018F218;
    if (mode == 1) {
        if (SH(a1, 0x10) == 0) {
            H(a1, 0) += H(a1, 8);
            H(a1, 0x12)++;
            H(a1, 2) += H(a1, 0xA);
            H(a1, 4) += H(a1, 0xC);
            SH(a1, 8) = SH(a1, 8) * 255 / 256;
            SH(a1, 0xC) = SH(a1, 0xC) * 255 / 256;
            {
                short y = H(a1, 0xA) + 1;
                SH(a1, 0xA) = y;
                if (!(SH(a1, 2) < D_800942EC)) {
                    y = -y;
                    SH(a1, 0xA) = y;
                    if (!(SH(a1, 2) < D_800942EC))
                        SH(a1, 0xA) = y / 4;
                }
            }
            t = func_80077CF4(SH(a1, 0x12) << 9);
            H(a1, 4) += t / 512;
            t = func_80077DC4(SH(a1, 0x12) << 9);
            H(a1, 0) += t / 512;
            if (SH(a1, 0x12) < 0x40)
                return 0;
            return 1;
        }
        if (SH(a1, 0x10) == 1) {
            H(a1, 0) += H(a1, 8);
            H(a1, 0x12)++;
            H(a1, 2) += H(a1, 0xA);
            H(a1, 4) += H(a1, 0xC);
            SH(a1, 8) = SH(a1, 8) * 127 / 128;
            SH(a1, 0xC) = SH(a1, 0xC) * 127 / 128;
            {
                short y = H(a1, 0xA) + 2;
                SH(a1, 0xA) = y;
                if (!(SH(a1, 2) < D_800942EC))
                    SH(a1, 0xA) = -y;
            }
            if (SH(a1, 0x12) < 0x20)
                return 0;
            return 1;
        }
        return 0;
    }
    if (mode == 2) {
        if (SH(a1, 0x10) == 0) {
            gte_SetRotMatrix(D_800BCFA4[0]);
            gte_SetTransMatrix(D_800BCFA4[0]);
            func_800D2104(a1, &buf, 0x28, 0xFF);
            p.x = H(a1, 0);
            p.y = H(a1, 2);
            p.y = H(a1, 2) + 8;
            p.z = H(a1, 4);
            func_800D2104(&p, &buf, 0x28, 0xFF);
            return 0;
        }
        if (SH(a1, 0x10) == 1) {
            size = func_80077DC4(SH(a1, 0x12) << 5);
            D_800F3368.a68 = 0x20;
            D_800F3368.a6A = 2;
            D_800F3368.a76 = 0x20;
            D_800F3368.a78 = 0x20;
            g.z = D_800E27EC << 5;
            size += 0x1000;
            func_800CF3AC(D_80193F68, &col, SH(a1, 0x12));
            w = D_800E1204[D_800F3368.a6C];
            if (D_800F3368.a6C == 4 && D_800F3428 != 0)
                w += 4;
            clut = func_80077AA4(0, w);
            func_800CEE20(a1, &g, size / 2, size / 2, D_800F3368.a6A * (SH(a1, 0x12) / 4),
                          clut & 0xFFFF, 1, 0x60, &col);
            return 0;
        }
        return 0;
    }
    return 0;
}
