/* NON_MATCHING: functionally equivalent to retail room_m0273i 0xAC68..0xAF88
 * (func_80199C50; spimdisasm marks it handwritten for its inline cop2 loads).
 * Written from the retail asm, not byte-matched; native port differential-tested
 * by tools/analysis/e21_diff.py.
 * Slime (m0273i) swing-attack battle effect main: mode 0 resets the swing state
 * and flags the owner (0x40000000); mode 1 records the owner's animation state
 * (D_8019AF94..98), puts two joint points (0,0,-0x130 through the matrices at
 * +0x320 / +0x480) in D_8019AF74 / D_8019AF7C, steps animation 0xF (done -> 1
 * at its last frame) and turns the owner toward the player for 8 frames once
 * the swing starts (frame 0xF). */
typedef struct {
    short vx, vy, vz, pad;
} SVec;

extern int *D_800F32D0;
extern unsigned char *D_8009D254;
extern unsigned char D_800966EC[];
extern unsigned char D_8019AFA2;
extern short D_8019AF74[], D_8019AF7A, D_8019AF82, D_8019AF8A, D_8019AF92;
extern short D_8019AF94, D_8019AF96, D_8019AF98, D_8019AF9A, D_8019AF9C, D_8019AF9E, D_8019AFA0;
extern int func_800DFF80(void *from, void *to);

int func_80199C50(int mode)
{
    SVec v;
    unsigned char *w = (unsigned char *)D_800F32D0[2];
    unsigned char *p;
    short *out;
    int *m, *o;
    int i, f, pf, k, a, t;

    if (mode == 0) {
        D_8019AFA2 = 0;
        o = *(int **)w;
        *o |= 0x40000000;
        D_8019AF7A = 0;
        D_8019AF82 = 1;
        D_8019AF8A = 0;
        D_8019AF92 = 0;
        D_8019AF9C = 0;
        D_8019AFA0 = 0;
        return 0;
    }
    if (mode != 1)
        return 0;
    p = *(unsigned char **)(*(unsigned char **)w + 0x18);
    if (*p == 1)
        *p = 2;
    f = *(short *)(w + 0x16);
    pf = *(short *)(w + 0x1A);
    v.vx = 0;
    v.vy = 0;
    v.vz = -0x130;
    D_8019AF94 = w[0xE];
    D_8019AF96 = f;
    D_8019AF98 = pf;
    out = D_8019AF74;
    for (i = 0; i < 2; i++, out += 4) {
        m = (int *)(*(int *)(w + 0x238) + (i ? 0x480 : 0x320));
        asm volatile("lw $12,0(%0)\n\t" "lw $13,4(%0)\n\t" "ctc2 $12,$0\n\t" "ctc2 $13,$1\n\t" "lw $12,8(%0)\n\t" "lw $13,12(%0)\n\t" "lw $14,16(%0)\n\t" "ctc2 $12,$2\n\t" "ctc2 $13,$3\n\t" "ctc2 $14,$4" : : "r"(m) : "$12", "$13", "$14");
        asm volatile("lw $12,20(%0)\n\t" "lw $13,24(%0)\n\t" "ctc2 $12,$5\n\t" "lw $14,28(%0)\n\t" "ctc2 $13,$6\n\t" "ctc2 $14,$7" : : "r"(m) : "$12", "$13", "$14");
        asm volatile("lwc2 $0,0(%0)\n\t" "lwc2 $1,4(%0)\n\t" "nop\n\t" "nop\n\t" ".word 0x4A480012" : : "r"(&v) : "memory");
        asm volatile("mfc2 $12,$9\n\t" "mfc2 $13,$10\n\t" "mfc2 $14,$11\n\t" "sh $12,0(%0)\n\t" "sh $13,2(%0)\n\t" "sh $14,4(%0)" : : "r"(out) : "$12", "$13", "$14", "memory");
    }
    if (w[0xE] == 0xF) {
        if (f >= w[0xF] - 1) {
            D_8019AFA2 = 1;
            o = *(int **)w;
            if (o)
                *(unsigned char *)o[6] = 4;
            return 1;
        }
        if (f >= 0x16) {
            k = D_8019AF9A;
            D_8019AF9A = k - 1;
            if (k > 0)
                *(int *)(w + 0x14) = 0;
        }
    }
    if (D_8019AF9A != 0 && f >= 0xF && pf < 0xF) {
        D_8019AF9C = 8;
        a = (short)func_800DFF80(D_8009D254 + 0x1FC, w + 0x1FC);
        D_8019AF9E = ((a - *(short *)(w + 0x3A)) & 0x800) ? -0x20 : 0x20;
    }
    if (D_8019AF9C != 0) {
        t = D_8019AF9E * *(short *)(D_800966EC + ((D_8019AF9C << 9) & 0x3E00));
        *(unsigned short *)(w + 0x3A) = *(unsigned short *)(w + 0x3A) + (t / 4096);
        D_8019AF9C = D_8019AF9C - 1;
    }
    if (D_8019AFA0 != 0)
        D_8019AFA0 = D_8019AFA0 - 1;
    return 0;
}
