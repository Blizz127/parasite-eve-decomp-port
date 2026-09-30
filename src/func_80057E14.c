/*
 * func_80057E14 - NONMATCHING, 2 word mismatches (band3)
 *
 * Check:
 *   MASPSX_SYMBOL_AT_TEMP=1 python3 tools/analysis/era_link_check.py \
 *       src/func_80057E14.c 0x80057E14 0xB8 -O2 -G8
 *
 * Residual (the ONLY two words; everything else matches):
 *   0x80057e68: ROM 00051140 (sll $v0,$a1,5)   LNK 00052940 (sll $a1,$a1,5)
 *   0x80057e70: ROM 00220821 (addu $at,$at,$v0) LNK 00250821 (addu $at,$at,$a1)
 * i.e. retail puts the `v << 5` index in $v0 (t's register, dead by then); cc1 shifts v in place
 * into $5 because v dies at the shift. Pure allocator choice.
 *
 * The five register pins ARE load-bearing (v=$5 out=$6 base=$7 lim=$8 t=$2 w=$3): without them the
 * whole block allocates one register lower. Two further points already solved and worth keeping:
 *  - `lim = base + 3;` must be an explicit local; leaving `v < base + 3` inline with a PINNED base
 *    stops cc1 hoisting it (hard registers block loop-invariant motion) and costs the preheader.
 *  - `t = v + 6; v = t - base;` in two statements with t in its own register is what stops cc1
 *    reassociating into `v - (base - 6)`; writing `k = v + 6 - base` hoists `base - 6` as invariant.
 *  - the loop must be the compound `while (n < 0xA && (v = a0[0]) != 0)` form.
 *
 * Tried without effect on the last two words: pinning k/idx/sh to $2 (reintroduces the nop by
 * blocking the delay-slot fill - $2 is the return register so cc1 treats it as live-out),
 * a separate `idx = v << 5;` local pinned and unpinned, computing the shift before `*out = v+0x200`,
 * swapping the two stores, `D_800A1E6E[v << 4]` element indexing.
 */
extern int D_8009D03C;
extern short *D_8009D04C;
extern int D_8009D054;
extern int D_8009D078;
extern short D_800A1FD4[];
extern unsigned short D_800A1E6E[];
extern void func_80055760(void);

int func_80057E14(short *a0)
{
    int n;
    register int v asm("$5");
    register short *out asm("$6");
    register int base asm("$7");
    register int lim asm("$8");
    register int t asm("$2");
    register unsigned short w asm("$3");
    int k;

    n = 0;
    if (a0 != 0) {
        base = D_8009D03C;
        out = D_800A1FD4;
        lim = base + 3;
        while (n < 0xA && (v = a0[0]) != 0) {
            if (v >= base && v < lim) {
                t = v + 6;
                v = t - base;
                *out = v + 0x200;
                w = a0[1];
                *(unsigned short *)((char *)D_800A1E6E + (v << 5)) = w;
                __asm__ __volatile__("" : : "r"(v));
                out++;
            } else {
                *out = v;
                out++;
            }
            n++;
            a0 += 2;
        }
        D_8009D04C = D_800A1FD4;
        D_8009D054 = n;
        func_80055760();
    }
    D_8009D078 = n;
    return n;
}
