/* room_m0318i — func_80193638, blob offset 0x4650, 0x374 bytes. NON_MATCHING
 * (behaviour C, proven by the e21 differential; not a byte match).
 * Handler dispatched on mode a0 with a state block a1:
 *   0: a1[8] = func_80071A54(); func_800CE870(D_8009D254, 1, a1);
 *      a1 halfword +2 = D_800942EC; while D_800B0E64 is set,
 *      func_8006DF50(D_800B0E64, 0x5EF, func_800D3FD8(), 0x80, 0x7F) then
 *      func_8006DF50(D_800B0E64, 0x5F0, 0x80, 0x80, 0x7F); if
 *      D_800E2368[0xD] is set, bump the byte at (**(D_800F32D0 +8))[0x18]
 *      from 1 to 2; func_800C6D5C(D_800F32D8, 0, 0); returns
 *      func_800CE560(D_800F33E0->+8, 0x10, 0x18, func_8019326C).
 *   1: frame counter D_800E27EC: func_80020D50() below 0x2B; every sixth
 *      frame below 0x28 take a node from func_800CE610(D_800F33E0->+8),
 *      clear its halfwords +6/+0xC, set +0xA = a1[8], and lower a1[8] by
 *      0x555 + (func_80071A54() & 0xFF); func_80020DD0() at 0x2B; at 0x2C
 *      with D_800E2368[0xD] set and the node word tagged 0x01000000, set
 *      bit 0x4000 in (*D_8009D254)[0x4C] and rewrite the node word to
 *      0x19000000 tag then bit 31; returns 1 once the counter reaches 0x40.
 *   2: D_80199904/06/08 = the first three halfwords of a1; returns 0.
 *   other modes return 0. */
typedef struct { unsigned char pad[8]; unsigned int **f8; } Root;

#define H(o, x) (*(unsigned short *)((char *)(o) + (x)))
#define SH(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))

extern unsigned short D_800942EC;
extern int **D_8009D254;
extern int D_800B0E64;
extern unsigned char *D_800E2368;
extern int D_800E27EC;
extern Root *D_800F32D0;
extern int D_800F32D8;
extern Root *D_800F33E0;
extern unsigned short D_80199904;
extern unsigned short D_80199906;
extern unsigned short D_80199908;

extern int func_80020D50();
extern int func_80020DD0();
extern int func_8006DF50();
extern int func_80071A54();
extern int func_800C6D5C();
extern int func_800CE560();
extern void *func_800CE610();
extern int func_800CE870();
extern int func_800D3FD8();
extern unsigned char func_8019326C[];

int func_80193638(int mode, unsigned char *a1)
{
    int t;
    void *p;
    unsigned int *w;
    unsigned char *flag;

    if (mode == 1) {
        if (D_800E27EC < 0x2B)
            func_80020D50();
        if (D_800E27EC < 0x28 && D_800E27EC == (D_800E27EC / 6) * 6) {
            p = func_800CE610(D_800F33E0->f8);
            if (p != 0) {
                SH(p, 6) = 0;
                SH(p, 0xC) = 0;
                SH(p, 0xA) = W(a1, 8);
                t = func_80071A54();
                W(a1, 8) = W(a1, 8) - 0x555 - (t & 0xFF);
            }
        }
        t = D_800E27EC;
        if (t == 0x2B) {
            func_80020DD0();
            t = D_800E27EC;
        }
        if (t == 0x2C && D_800E2368[0xD] != 0 &&
            (**D_800F32D0->f8 & 0x3F000000) == 0x01000000) {
            Root *r = D_800F32D0;
            p = *D_8009D254;
            W(p, 0x4C) = W(p, 0x4C) | 0x4000;
            w = *r->f8;
            *w = (*w & 0xC0FFFFFF) | 0x19000000;
            w = *r->f8;
            *w = *w | 0x80000000;
        }
        if (D_800E27EC >= 0x40)
            return 1;
        return 0;
    }
    if (mode == 0) {
        W(a1, 8) = func_80071A54();
        func_800CE870(D_8009D254, 1, a1);
        H(a1, 2) = D_800942EC;
        if (D_800B0E64 != 0) {
            t = func_800D3FD8();
            func_8006DF50(D_800B0E64, 0x5EF, t, 0x80, 0x7F);
            if (D_800B0E64 != 0)
                func_8006DF50(D_800B0E64, 0x5F0, 0x80, 0x80, 0x7F);
        }
        if (D_800E2368[0xD] != 0) {
            unsigned int **q = D_800F32D0->f8;
            if (q != 0) {
                w = *q;
                if (w != 0) {
                    flag = *(unsigned char **)((char *)w + 0x18);
                    if (*flag == 1)
                        *flag = 2;
                }
            }
        }
        func_800C6D5C(D_800F32D8, 0, 0);
        return func_800CE560(D_800F33E0->f8, 0x10, 0x18, func_8019326C);
    }
    if (mode == 2) {
        D_80199904 = H(a1, 0);
        D_80199906 = H(a1, 2);
        D_80199908 = H(a1, 4);
        return 0;
    }
    return 0;
}
