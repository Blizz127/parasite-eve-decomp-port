/* room_m0318i — func_80195190, blob offset 0x61A8, 0x454 bytes. NON_MATCHING
 * (behaviour C, proven by the e21 differential; not a byte match).
 * Effect-object callback taking (mode, a1 = effect record), the cmd 1 / cmd 2
 * sibling of room_m0429i func_801982A4 (minus the GTE matrix set-up).
 * mode 1: per-frame motion; state (+0x10) 0 lasts 16 frames, state 1 moves
 *         the position (+0/+2/+4) by the velocity (+8/+A/+C), damping x/z by
 *         127/128, bouncing y against D_800942EC, and lasts 24 frames;
 *         returns 1 when the frame counter (+0x12) reaches the limit;
 * mode 2: draw. State 0 sets the D_800F3368 block (+0/+0xE/+0x10 = 0x20,
 *         +2 = 2, +4 = 2, +6 = 1) and draws D_80199924 with a sprite whose
 *         u/v offset (buf +4) is advanced by D_800E27EC * 32; state 1 sets
 *         the block (0x10, 1, 2, 0) and draws the record itself.
 * Any other mode / state returns 0. The 8-byte buf is seeded from
 * D_8018F208 (unaligned word-pair copy). */
typedef struct { unsigned char c[8]; } B8;
typedef struct { unsigned char c[8]; } V8;

#define H(o, x) (*(unsigned short *)((char *)(o) + (x)))
#define SH(o, x) (*(short *)((char *)(o) + (x)))

extern B8 D_8018F208;
extern unsigned char D_801996EC[];
extern unsigned char D_80199924[];
extern short D_800942EC;
extern int D_800E27EC;
extern unsigned short D_800E11F8;
extern unsigned short D_800E11E8;
extern unsigned short D_800E2850[];
extern unsigned short D_800E1204[];
extern int D_800F3428;
extern short D_800F3368;
extern short D_800F336A;
extern unsigned short D_800F336C;
extern short D_800F336E;
extern short D_800F3370;
extern short D_800F3376;
extern short D_800F3378;

extern int func_80077CF4();
extern int func_80077DC4();
extern int func_80077AA4();
extern void func_800CF3AC();
extern void func_800CEE20();

int func_80195190(int mode, unsigned char *a1)
{
    B8 buf;
    V8 v;
    int s1;
    int s2;
    int w;
    int clut;

    buf = D_8018F208;
    if (mode == 1) {
        switch (SH(a1, 0x10)) {
        case 0:
            H(a1, 0x12)++;
            if (SH(a1, 0x12) < 0x10)
                return 0;
            return 1;
        case 1:
            H(a1, 0)  += H(a1, 8);
            H(a1, 2)  += H(a1, 0xA);
            H(a1, 0x12)++;
            H(a1, 4)  += H(a1, 0xC);
            SH(a1, 8) = SH(a1, 8) * 127 / 128;
            SH(a1, 0xC) = SH(a1, 0xC) * 127 / 128;
            {
                short y = SH(a1, 0xA) + 1;
                SH(a1, 0xA) = y;
                if (SH(a1, 2) >= D_800942EC)
                    SH(a1, 0xA) = -y;
            }
            if (SH(a1, 0x12) < 0x18)
                return 0;
            return 1;
        }
        return 0;
    }
    if (mode == 2) {
        switch (SH(a1, 0x10)) {
        case 0:
            s1 = SH(a1, 0x12) << 6;
            s2 = func_80077CF4(s1) * 5 / 2;
            s1 = func_80077DC4(s1) / 32;
            D_800F3368 = 0x20;
            D_800F3376 = 0x20;
            D_800F3378 = 0x20;
            D_800F336A = 2;
            H(&buf, 4) = H(&buf, 4) + D_800E27EC * 32;
            D_800F336C = 2;
            D_800F336E = 1;
            D_800F3370 = D_800E2850[D_800E11F8];
            func_800CF3AC(D_801996EC, &v, SH(a1, 0x12));
            w = D_800E1204[D_800F336C];
            if (D_800F336C == 4 && D_800F3428 != 0)
                w += 8;
            else
                w += 4;
            clut = func_80077AA4(0, w);
            func_800CEE20(D_80199924, &buf, s2, s2, 0x44, clut & 0xFFFF, 1, s1, &v);
            return 0;
        case 1:
            s1 = (SH(a1, 0x12) << 10) / 24;
            s2 = func_80077DC4(s1) / 2 + 0x800;
            s1 = func_80077DC4(s1) / 32;
            D_800F3368 = 0x10;
            D_800F336A = 1;
            D_800F3376 = 0x10;
            D_800F3378 = 0x10;
            D_800F336C = 2;
            D_800F336E = 0;
            D_800F3370 = D_800E2850[D_800E11E8];
            func_800CF3AC(D_801996EC, &v, SH(a1, 0x12) * 2);
            w = D_800E1204[D_800F336C];
            if (D_800F336C == 4 && D_800F3428 != 0)
                w += 4;
            clut = func_80077AA4(0x20, w);
            func_800CEE20(a1, 0, s2 / 2, s2 / 2, D_800F336A * 2 + 0xD8, clut & 0xFFFF, 1, s1, &v);
            return 0;
        }
        return 0;
    }
    return 0;
}
