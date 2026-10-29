/* NON_MATCHING: functionally equivalent to retail room_m0273i 0x8BD4..0x9078
 * (func_80197BBC; spimdisasm marks it handwritten for its inline cop2 loads).
 * Written from the retail asm, not byte-matched; native port differential-tested
 * by tools/analysis/e21_diff.py.
 * Battle effect main (m0273i, the giant slime / Eve): mode 0 clears the turn
 * state and flags the owner (0x40000000); mode 1 steers the owner's yaw toward
 * the player for phase D_8019AF5A of the D_8019AD80 table (12-byte rows:
 * start anim, frame window lo/hi, end frame, swing, base), opens the hit
 * window and reports done (1) at the end frame.  Every mode but the done
 * return puts the owner's mouth point (0,0,-0x180 through its joint matrix)
 * in D_8019AEFC. */
typedef struct {
    short anim, lo, hi, end, swing, base;
} Row;
typedef struct {
    short vx, vy, vz, pad;
} SVec;

extern int *D_800F32D0;
extern int *D_8009D254;
extern short D_800942EC;
extern Row D_8019AD80[];
extern short D_8019AEFC[3];
extern unsigned char D_8019AF68, D_8019AF69, D_8019AF6A, D_8019AF6B;
extern short D_8019AF0A, D_8019AF54, D_8019AF56, D_8019AF5A, D_8019AF5C, D_8019AF5E;
extern short D_8019AF60, D_8019AF62, D_8019AF64, D_8019AF66;
extern int func_80079FB4(int dy, int dx);

int func_80197BBC(int mode)
{
    SVec v;
    unsigned char *w = (unsigned char *)D_800F32D0[2];
    unsigned char *o;
    int *m;
    Row *r;
    int t, d, e, a, b;

    if (mode == 0) {
        D_8019AF6B = 0;
        D_8019AF69 = 0;
        D_8019AF6A = 0;
        D_8019AF68 = 0;
        D_8019AF5E = 0;
        D_8019AF54 = 0;
        D_8019AF0A = 0;
        D_8019AF62 = D_800942EC - 0x180;
        o = *(unsigned char **)w;
        *(int *)o |= 0x40000000;
    } else if (mode == 1) {
        o = *(unsigned char **)w;
        if (**(unsigned char **)(o + 0x18) == 1)
            **(unsigned char **)(o + 0x18) = 2;
        if (w[0xE] == 0xD) {
            t = *(short *)(w + 0x16);
            r = &D_8019AD80[D_8019AF5A];
            if (t == 0) {
                *(int *)(w + 0x14) = r->anim << 16;
                a = func_80079FB4(*(int *)(w + 0x28) - D_8009D254[10],
                                  *(int *)(w + 0x30) - D_8009D254[12]);
                d = (a - *(unsigned short *)(w + 0x3A)) & 0xFFF;
                if (D_8019AF5A != 0) {
                    D_8019AF66 = d < 0x801 ? d >> 6 : 0;
                } else {
                    e = (short)(0x1000 - d);
                    D_8019AF66 = e < 0x801 ? (-e) >> 6 : 0;
                }
            } else {
                if (t >= r->end) {
                    D_8019AF69 = 1;
                    o = *(unsigned char **)w;
                    if (o)
                        **(unsigned char **)(o + 0x18) = 4;
                    return 1;
                }
                if (t >= r->hi && D_8019AF56 > 0) {
                    *(int *)(w + 0x14) = r->lo << 16;
                    D_8019AF56 = D_8019AF56 - 1;
                }
            }
            if (t >= r->lo)
                D_8019AF6B = 1;
            D_8019AF6A = t >= r->lo ? !(r->hi < t) : 0;
            o = (unsigned char *)D_800F32D0[2];
            a = *(short *)(o + 0x16);
            b = *(short *)(o + 0x1A);
            D_8019AF68 = a >= D_8019AF64 ? b < D_8019AF64 : 0;
            if (a >= r->lo && a <= r->hi) {
                e = (r->swing * D_8019AF5E) / D_8019AF5C;
                w[0x25A] = 0x15;
                w[0x25B] = 2;
                *(short *)(w + 0x256) = e;
                D_8019AF60 = r->base + e + *(unsigned short *)(w + 0x3A);
                D_8019AF5E = D_8019AF5E + 1;
            } else {
                w[0x25A] = 0;
                w[0x25B] = 0;
            }
            if (a > r->hi)
                D_8019AF66 = D_8019AF66 >> 1;
            *(unsigned short *)(w + 0x3A) = *(unsigned short *)(w + 0x3A) + D_8019AF66;
        }
    }
    v.vx = 0;
    v.vy = 0;
    v.vz = -0x180;
    m = (int *)(*(int *)(w + 0x238) + 0x2C0);
    asm volatile("lw $12,0(%0)\n\t" "lw $13,4(%0)\n\t" "ctc2 $12,$0\n\t" "ctc2 $13,$1\n\t" "lw $12,8(%0)\n\t" "lw $13,12(%0)\n\t" "lw $14,16(%0)\n\t" "ctc2 $12,$2\n\t" "ctc2 $13,$3\n\t" "ctc2 $14,$4" : : "r"(m) : "$12", "$13", "$14");
    asm volatile("lw $12,20(%0)\n\t" "lw $13,24(%0)\n\t" "ctc2 $12,$5\n\t" "lw $14,28(%0)\n\t" "ctc2 $13,$6\n\t" "ctc2 $14,$7" : : "r"(m) : "$12", "$13", "$14");
    asm volatile("lwc2 $0,0(%0)\n\t" "lwc2 $1,4(%0)\n\t" "nop\n\t" "nop\n\t" ".word 0x4A480012" : : "r"(&v) : "memory");
    asm volatile("mfc2 $12,$9\n\t" "mfc2 $13,$10\n\t" "mfc2 $14,$11\n\t" "sh $12,0(%0)\n\t" "sh $13,2(%0)\n\t" "sh $14,4(%0)" : : "r"(D_8019AEFC) : "$12", "$13", "$14", "memory");
    return 0;
}
