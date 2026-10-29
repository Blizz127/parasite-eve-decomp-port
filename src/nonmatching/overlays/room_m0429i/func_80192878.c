/* room_m0429i — func_80192878, blob offset 0x3890, 0x298 bytes. NON_MATCHING
 * (behaviour C, proven by the e21 differential; not a byte match).
 * Per-frame effect-object step. Copies the 10-word argument block
 * (a0+0x1C..0x40) into the D_8009D254 player record (+0x1E8..0x204, +0x28,
 * +0x30), then dispatches on the mode byte s0+0xE (s0 = *(a0+8)): 0xD and
 * 0x10 are the two timed modes, 0xC does nothing, any other mode (and a
 * dead/empty record *D_8009D254, lh +0xC <= 0) runs the teardown
 * func_80020CE4 + func_80192B10(s0, a0+0xC) + func_80192BFC(a0).
 * The shorts s0+0x16 / s0+0x1A are the current / previous frame counters,
 * compared with the limit s0+0xF - 1 and the thresholds 0x12 / 3 / 0xB.
 * Declared void: the retail v0 on exit is whatever the last call/load left
 * and no caller reads it. */
#define SH(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define UB(o, x) (*(unsigned char *)((char *)(o) + (x)))

extern unsigned char *D_8009D254;

extern int func_80020CE4();
extern void func_80192B10();
extern int func_80192BFC();

void func_80192878(unsigned char *a0)
{
    unsigned char *s0 = *(unsigned char **)(a0 + 8);
    unsigned char *s2 = a0 + 0xC;
    unsigned char *rec;
    unsigned char *p;
    short a1, a3;
    int v;

    rec = D_8009D254;
    W(rec, 0x1E8) = W(a0, 0x1C);
    W(rec, 0x1EC) = W(a0, 0x20);
    W(rec, 0x1F0) = W(a0, 0x24);
    W(rec, 0x1F4) = W(a0, 0x28);
    W(rec, 0x1F8) = W(a0, 0x2C);
    W(rec, 0x1FC) = W(a0, 0x30);
    W(rec, 0x200) = W(a0, 0x34);
    W(rec, 0x204) = W(a0, 0x38);
    rec = D_8009D254;
    W(rec, 0x28) = W(a0, 0x3C);
    a1 = SH(s0, 0x16);
    a3 = SH(s0, 0x1A);
    W(rec, 0x30) = W(a0, 0x40);

    p = *(unsigned char **)rec;
    if (p == 0 || SH(p, 0xC) <= 0) {
        func_80192B10(s0, s2);
        {
        func_80192BFC(a0);
        return;
    }
    }

    v = UB(s0, 0xE);
    if (v == 0xD) {
        p = *(unsigned char **)s0;
        if (p != 0) {
            unsigned char *q = *(unsigned char **)(p + 0x18);
            if (UB(q, 0) == 1)
                UB(q, 0) = 2;
        }
        if (a3 < 0x12 && a1 >= 0x12) {
            func_80020CE4();
            {
            func_80192B10(s0, s2);
            return;
        }
        }
        if (a3 < 3 && a1 >= 3) {
            rec = D_8009D254;
            p = *(unsigned char **)rec;
            if (p == 0)
                return;
            if (SH(p, 0xC) <= 0)
                return;
            v = W(p, 0x4C) | 0x4000;
            W(p, 0x4C) = v;
            p = *(unsigned char **)s0;
            if (p == 0)
                return;
            v = W(p, 0) | 0x80000000;
            W(p, 0) = v;
            return;
        }
        v = UB(s0, 0xF) - 1;
        if (a1 < v)
            return;
        p = *(unsigned char **)s0;
        if (p != 0)
            UB(*(unsigned char **)(p + 0x18), 0) = 4;
        {
        func_80192BFC(a0);
        return;
    }
    }
    if (v < 0xE) {
        if (v == 0xC)
            return;
        func_80020CE4();
        func_80192B10(s0, s2);
        {
        func_80192BFC(a0);
        return;
    }
    }
    if (v != 0x10) {
        func_80020CE4();
        func_80192B10(s0, s2);
        {
        func_80192BFC(a0);
        return;
    }
    }
    if (a1 >= 0xB && a3 < 0xB) {
        W(s0, 0x14) = 0x280000;
        return;
    }
    v = UB(s0, 0xF) - 1;
    if (a1 < v)
        return;
    p = *(unsigned char **)(s2 + 4);
    W(p, 0) = 2;
    return;
}
