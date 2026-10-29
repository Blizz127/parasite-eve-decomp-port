/* room_m0318i — func_80192ED4, blob offset 0x3EEC, 0x398 bytes. NON_MATCHING
 * (behaviour C, proven by the e21 differential; not a byte match).
 * Effect callback, a0 = mode, a1 = effect record, a2 = parameter block.
 *   mode 0: a1 +8 = func_80071A54() (RNG), a1 +0xC = 0; walk the D_8009D20C
 *           list (stop at the D_8009D254 sentinel / null) for a node whose +0
 *           pointer has a positive +0x10 and whose bytes +0xD/+0xC equal
 *           a2 +0 / +4, then func_800CE870(node, 0, a1); else printf
 *           D_8018F1CC. Returns func_800CE560(D_800F33E0->+8, 8, 0x18,
 *           func_80192DA0).
 *   mode 1: while D_800E27EC < 0x33 spawn a particle in the D_800F33E0->+8
 *           pool: +0/+4 = a1 +0/+4 plus cos/sin(a1 +8) * a1 +0xC / 4096,
 *           +2 = a1 +2, +6 = RNG & 3; a1 +8 advances by 0x8AA + (RNG & 0x1F).
 *           Returns 1 once D_800E27EC >= 0x4A, else 0.
 *   mode 2: D_800F3374 = 8; while D_800E27EC < 0x33 draw a 0x15E/0x1F4/0x14
 *           func_800D0728 ring at a1 +0/+2/+4 (a1 +0xC = sin(frame * 1024 / 50)
 *           / 8) using the D_801994BC colour table; then set the
 *           D_800F3368.. draw-state shorts. Returns 0.
 * a0 outside 0..2 returns 0. Retail leaves no stack read-before-write in this
 * function (the mode 2 frame at sp+0x30..0x47 is written in full before the
 * func_800CF3AC / func_800D0728 calls). */

#define H(o, x) (*(unsigned short *)((char *)(o) + (x)))
#define SH(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))

extern unsigned char *D_8009D20C;
extern unsigned char *D_8009D254;
extern unsigned char *D_800F33E0;
extern int D_800E27EC;
extern unsigned short D_800E11FA;
extern unsigned short D_800E2850[];
extern short D_800F3368;
extern short D_800F336A;
extern short D_800F336C;
extern short D_800F336E;
extern unsigned short D_800F3370;
extern short D_800F3372;
extern short D_800F3374;
extern short D_800F3376;
extern short D_800F3378;
extern char D_8018F1CC[];
extern unsigned char D_801994BC[];
extern unsigned char func_80192DA0[];

extern int func_80071A54();
extern int func_80071A74();
extern int func_80077CF4();
extern int func_80077DC4();
extern int func_800CE560();
extern unsigned char *func_800CE610();
extern void func_800CE870();
extern void func_800CF3AC();
extern void func_800D0728();

int func_80192ED4(int mode, unsigned char *a1, unsigned char *a2)
{
    unsigned char *p, *sentinel, *s;
    int key0, key1, v, c;
    /* one frame, as retail's sp+0x30..0x47: colour (+0), position (+8), angle (+16) */
    struct { unsigned char col[8]; unsigned short pos[4]; short ang[4]; } f;

    switch (mode) {
    case 0:
        W(a1, 8) = func_80071A54();
        p = D_8009D20C;
        W(a1, 0xC) = 0;
        key0 = W(a2, 0);
        key1 = W(a2, 4);
        if (p != 0) {
            sentinel = D_8009D254;
            do {
                if (p != sentinel && W(p, 0) != 0 && W(*(unsigned char **)p, 0x10) > 0
                    && p[0xD] == key0 && p[0xC] == key1) {
                    func_800CE870(p, 0, a1);
                    goto found;
                }
                p = *(unsigned char **)(p + 4);
            } while (p != 0);
        }
        func_80071A74(D_8018F1CC, key0, key1);
    found:
        return func_800CE560(W(D_800F33E0, 8), 8, 0x18, func_80192DA0);
    case 1:
        if (D_800E27EC < 0x33) {
            s = func_800CE610(W(D_800F33E0, 8));
            if (s != 0) {
                v = func_80077DC4(W(a1, 8)) * W(a1, 0xC);
                SH(s, 0) = H(a1, 0) + (v / 4096);
                v = func_80077CF4(W(a1, 8)) * W(a1, 0xC);
                SH(s, 4) = H(a1, 4) + (v / 4096);
                SH(s, 2) = H(a1, 2);
                SH(s, 6) = func_80071A54() & 3;
                W(a1, 8) = W(a1, 8) + 0x8AA + (func_80071A54() & 0x1F);
            }
        }
        if (D_800E27EC >= 0x4A)
            return 1;
        return 0;
    case 2:
        D_800F3374 = 8;
        if (D_800E27EC < 0x33) {
            f.pos[0] = H(a1, 0);
            f.pos[1] = H(a1, 2);
            f.pos[2] = H(a1, 4);
            f.ang[0] = 0x400;
            f.ang[1] = 0;
            f.ang[2] = D_800E27EC << 5;
            f.ang[3] = 1;
            v = func_80077CF4((D_800E27EC << 10) / 50);
            c = v;
            W(a1, 0xC) = c / 8;
            func_800CF3AC(D_801994BC, f.col, D_800E27EC);
            func_800D0728(f.pos, 0x15E, 0x1F4, 0x14, f.ang, c, c, 0, f.col, 0x80, 1);
        }
        D_800F3368 = 0x20;
        D_800F336A = 2;
        D_800F3376 = 0x20;
        D_800F3378 = 0x20;
        v = D_800E2850[D_800E11FA];
        D_800F336C = 3;
        D_800F336E = 1;
        D_800F3372 = 0;
        D_800F3374 = 4;
        D_800F3370 = v;
        return 0;
    default:
        return 0;
    }
}
