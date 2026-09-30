/* band7 near-miss, NOT matching.  For band1's fresh-eyes pass.
 *
 *   vram 0x800C5EB0   size 0x1DC (119 words)   asm/disc1/B3B24.s:2906
 *   best:  tools/analysis/era_link_check.py <this> 0x800C5EB0 0x1DC -O2 -G0 -fno-schedule-insns
 *          -> word mismatches=12, nonzero_pad=0   (NO env knobs)
 *          without -fno-schedule-insns: 21.   -O1 -G0: 33.
 *   Inert: MASPSX_SYMBOL_AT_TEMP, MASPSX_EXPAND_DIV, ERA_ASPSX_VER=2.30,
 *   -fno-strength-reduce.
 *
 * Residual: two register-pressure-bound instruction PLACEMENTS --
 *   (a) the `p = base + 4` preheader init, which retail emits inside the
 *       preheader after the other invariants, and
 *   (b) an epilogue that must compute the return value BEFORE its store.
 * The five pins below already reproduce retail's $s0..$s4 assignment; dropping
 * them regresses hard (118 @-O2, 27 @-O1), so unlike func_800C251C the pins are
 * net-positive here.
 *
 * NOTE (see the long writeup in func_800C251C.c in this directory): any
 * `register T x asm("$N")` pin suppresses cc1's prologue save/copy INTERLEAVE.
 * Retail's prologue here IS interleaved --
 *     sw $s3,0x44($sp) / addu $s3,$a0,$zero / sw $ra / sw $s4 / sw $s2 / sw $s1
 *     / sw $s0 / lw $s1,0($s3) / addu $s4,$a2,$zero
 * -- so a few of the remaining 12 words are that same pins-vs-interleave
 * conflict, and the same ref-count/live-range shaping (rather than pinning)
 * is the way out.
 */
typedef struct { short vx; short vy; short vz; short pad; } SV4;
typedef struct { char b[8]; } Blk8;

extern int func_80071A54(void);
extern int func_800C6B20(SV4 *a0);

int func_800C5EB0(unsigned char *a0, int a1, int *a2) {
    SV4 v[4];
    register unsigned char *base asm("$17");
    register unsigned int i asm("$18");
    register unsigned char *s asm("$19");
    register int *dst asm("$20");
    register unsigned char *p asm("$16");
    unsigned short e;

    s = a0;
    dst = a2;
    base = *(unsigned char **)s;
    *dst = 0;
    {
        int e0 = *(short *)(s + 0xE);
        int n0 = *(short *)(s + 4);
        if ((unsigned int)e0 < (unsigned int)n0) {
            base[e0 * 0x44] = 2;
        }
    }
    i = 0;
    if (i < (unsigned int)(*(short *)(s + 4) - 1)) {
    p = base + 4;
    do {
        int r;
        r = func_80071A54();
        p[-1] = r % 4;
        if (base[0] == 2) {
            int res;
            *(Blk8 *)&v[0] = *(Blk8 *)(p + 0x10);
            *(Blk8 *)&v[1] = *(Blk8 *)(p + 0x18);
            *(Blk8 *)&v[2] = *(Blk8 *)(p + 0x54);
            *(Blk8 *)&v[3] = *(Blk8 *)(p + 0x5C);
            v[0].vy = 0;
            v[1].vy = 0;
            v[2].vy = 0;
            v[3].vy = 0;
            res = func_800C6B20(&v[0]);
            if (res == 1) {
                *dst = res;
            }
            {
                unsigned short t = *(unsigned short *)p;
                if (t >= 0xD) {
                    *(short *)p = t - 0xC;
                } else {
                    *(short *)p = 0;
                    base[0] = 0;
                }
            }
        }
        i++;
        p += 0x44;
        base += 0x44;
    } while (i < (unsigned int)(*(short *)(s + 4) - 1));
    }
    e = *(unsigned short *)(s + 0xE);
    {
        int r;
        __asm__ __volatile__("" : "=r"(e) : "0"(e));
        r = ((short)e == 0x64);
        __asm__ __volatile__("" : "=r"(r) : "0"(r));
        *(unsigned short *)(s + 0xE) = e + 1;
        return r;
    }
}
