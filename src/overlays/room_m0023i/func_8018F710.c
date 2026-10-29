/* room_m0023i (PE.IMG room m0023i chunk 2, VRAM 0x8018EFE8)
 * func_8018F710 — blob offset 0x728, 0x504 bytes. Profile era_o2_g0_expand_div
 * (-O2 -G0, MASPSX_EXPAND_DIV=1; the guarded divide by (D_800E27EC & 1) + 1).
 * Eve flash-effect callback: mode 0 init, 1 advance, 2 render (3 sub-states).
 * Levers:
 *   - mode 1 is `H++; if (SH(a1,4) < 3) break; return 1;`, so the counter store
 *     sits in the bnez delay slot to the switch's shared return 0;
 *   - the sub-states are a switch;
 *   - each post-call reload goes into its own per-state temp (c0/c1/c2);
 *   - s4 = 0x50 goes after the s2 computation;
 *   - the func_80077CF4 /32, both s2 * 3 / 2, the t & 0xFFFF masks and the
 *     divisor are inline call arguments;
 *   - the table index is an unsigned short, and the D_800E11EA index is its
 *     own variable (j).
 * Hand-written in this repo (khasinski's vendored tree has this body as asm only). */

typedef struct { short x, y, z, w; } GteShortVector;
typedef struct { unsigned char r, g, b, cd; } CVECTOR;

#define B(o, x) (*(unsigned char *)((char *)(o) + (x)))
#define H(o, x) (*(unsigned short *)((char *)(o) + (x)))
#define SH(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(unsigned int *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))

extern GteShortVector D_8018EFF4;
extern CVECTOR D_8018EFFC;
extern CVECTOR D_8018F000;

extern unsigned char *D_800E2368;
extern void *D_800F32D0;
extern int D_800E27EC;
extern unsigned short D_800E11EA;
extern unsigned short D_800E2850[];
extern unsigned short D_800E1204[];
extern int D_800F3428;
extern struct { short a68, a6A; unsigned short a6C; short a6E, a70, a72, a74, a76, a78; } D_800F3368;

extern int func_800D3FD8(void);
extern void func_800D3F64(int a0, int a1);
extern unsigned short func_80077AA4();
extern int func_80077CF4(int angle);
extern int func_80077DC4(int angle);
extern void func_800D0728(GteShortVector *pos, int outer, int inner, int n, GteShortVector *ang,
                          int sx, int sy, unsigned char *c0, unsigned char *c1, int bright,
                          int abr);
extern void func_800D004C(GteShortVector *pos, int r0, int r1, int n, GteShortVector *ang,
                          int sx, int sy, unsigned char *c0, unsigned char *c1, int bright,
                          int abr);
extern void func_800CE8F0(void *owner, int index, GteShortVector *input, GteShortVector *output);
extern void func_800CEE20(GteShortVector *pos, GteShortVector *ang, int sx, int sy, int tex,
                          int clut, int abr, int bright, unsigned char *col);

int func_8018F710(int a0, unsigned char *a1)
{
    GteShortVector pos;
    GteShortVector ang38;
    GteShortVector eff4;
    CVECTOR effc;
    CVECTOR f000;
    int mode;
    int c0, c1, c2;
    int t;
    int x;
    int state;
    int cursor;
    int s2;
    int s4;
    int r;
    int w;
    unsigned short idx;
    unsigned short j;

    mode = a0;
    eff4 = D_8018EFF4;
    effc = D_8018EFFC;
    f000 = D_8018F000;

    switch (mode) {
    case 0:
        switch (*(short *)(D_800E2368 + 0x12)) {
        case 0:
            SH(a1, 0) = 8;
            x = func_800D3FD8();
            func_800D3F64(0x5AE, x);
            func_800D3F64(0x5AF, 0x80);
            break;
        case 1:
            SH(a1, 0) = 0x10;
            break;
        }
        SH(a1, 4) = 0;
        SH(a1, 2) = 0;
        return 0;
    case 1:
        H(a1, 2)++;
        if (SH(a1, 4) < 3) {
            break;
        }
        return 1;
    case 2:
        s2 = 0;
        state = SH(a1, 4);
        s4 = 0;
        switch (state) {
        case 0:
            cursor = SH(a1, 2);
            func_800D0728(&pos, 0x258, 0x320, 0x20, 0, cursor, cursor, (unsigned char *)&effc, 0,
                          func_80077CF4(cursor << 7) / 32, 1);
            c0 = SH(a1, 2);
            s2 = c0 << 8;
            s4 = 0x50;
            if (c0 < 8) {
                break;
            }
            SH(a1, 4) = 1;
            SH(a1, 2) = 0;
            break;
        case 1:
            c1 = SH(a1, 2);
            s2 = (c1 << 6) + 0x800;
            s4 = c1 * 48 / 32 + 0x50;
            if (c1 < 0x20) {
                break;
            }
            SH(a1, 4) = 2;
            SH(a1, 2) = 0;
            break;
        case 2:
            r = func_80077DC4(SH(a1, 2) << 7);
            r = r / 2;
            s4 = 0xA0;
            c2 = SH(a1, 2);
            s2 = r + 0x1000;
            if (c2 < 8) {
                break;
            }
            SH(a1, 4) = 3;
            SH(a1, 2) = 0;
                    break;
        }
shared_tail:
        j = D_800E11EA;
        D_800F3368.a68 = 0x40;
        D_800F3368.a6A = 4;
        D_800F3368.a76 = 0x40;
        D_800F3368.a78 = 0x40;
        D_800F3368.a70 = D_800E2850[j];
        D_800F3368.a6C = 3;
        D_800F3368.a6E = 0;
        D_800F3368.a72 = 0;
        D_800F3368.a74 = 0x18;
        if (D_800E27EC & 1) {
            s2 = s2 * 5 / 4;
        }
        ang38.x = 0;
        ang38.y = 0;
        ang38.w = 0;
        ang38.z = -D_800E27EC << 6;
        func_800CE8F0(P(D_800F32D0, 8), SH(a1, 0), &eff4, &pos);
        idx = D_800F3368.a6C;
        w = D_800E1204[idx];
        if (idx == 4 && D_800F3428 != 0) {
            w += 4;
        }
        t = func_80077AA4(0x30, w);
        func_800CEE20(&pos, &ang38, s2 * 3 / 2, s2 * 3 / 2, 0x80, t & 0xFFFF, 1, s4 / ((D_800E27EC & 1) + 1), 0);
        idx = D_800F3368.a6C;
        w = D_800E1204[idx];
        if (idx == 4 && D_800F3428 != 0) {
            w += 4;
        }
        t = func_80077AA4(0x40, w);
        func_800CEE20(&pos, &ang38, s2 * 3 / 2, s2 * 3 / 2, 0x84, t & 0xFFFF, 1, s4, 0);
        func_800D004C(&pos, 0x258, 0x258, 0xC, 0, s2, s2, (unsigned char *)&f000, 0, 0x40, 1);
        return 0;
    }
    return 0;
}
