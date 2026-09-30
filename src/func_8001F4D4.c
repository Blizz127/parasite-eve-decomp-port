/*
 * func_8001F4D4 - battle: apply an attack's ATB/timer cost to the actor.
 *
 * VRAM 0x8001F4D4 / file 0xFCD4 / size 0x340 (208 words). asm/disc1/DB40.s.
 * era -O2 -G0 with MASPSX_EXPAND_DIV=1 (the `(x << 2) / short` near the end
 * carries retail's inline div-by-zero / overflow guards).
 * Verified LINK_EXACT, 0 word mismatches, zero pad.
 *
 * Durable lever for the divide-by-constant sequences: retail's
 *   andi $v1,$v1,0xFFFF / sll+addu (x3) / mult $v0,0x66666667 / mfhi / srl 2
 * is NOT a plain `x * 3 / 10`.  A signed `/10` gives `sra <post_shift>` plus the
 * `sra 31` + `subu` sign correction, and an unsigned one gives
 * `multu 0xCCCCCCCD` + `srl 3`.  The SIGNED magic with an UNSIGNED post-shift
 * and no correction is what cc1 emits when the quotient is assigned to an
 * `unsigned short`: `unsigned short v; ... v = v * 3 / 10;` -- combine drops the
 * sign fix because only the low 16 bits survive, and the truncating `andi` is
 * deferred to each later use of `v` (which is exactly where retail has it).
 * The first divide, `*(unsigned short *)(p + 0x20) / 5`, is a plain unsigned
 * `multu 0xCCCCCCCD` + `srl 2`; the `% 100` / `/ 100` pair is the ordinary
 * signed 0x51EB851F expansion.
 * Second lever: a masked bitfield compared against a constant needs an `int`
 * local -- `kind = (w >> 21) & 7; if (kind < 3)` gives retail's signed `slti`,
 * while folding the mask into the condition keeps the value unsigned and emits
 * `sltiu` (the one-word residual before this change).
 */
extern unsigned char *D_8009D278;
extern unsigned char *D_8009D254;
extern unsigned char **D_8009D1D0;

extern int func_80071A54(void);
extern void func_8001F814(unsigned char **a0);
extern void func_80020288(unsigned char *a0);

void func_8001F4D4(unsigned char **a0) {
    unsigned char *p;
    unsigned char *ob;
    unsigned short v;
    int fl;
    int base;
    int amt;
    int kind;

    p = D_8009D278;
    v = *(unsigned short *)(p + 0x20) / 5;
    fl = *(int *)(p + 0x4C);
    ob = a0[0];
    if (fl & 0x1000) {
        v = v * 3 / 10;
    }
    if (fl & 0x100) {
        v = v * 3 / 10;
    }
    kind = (*(unsigned int *)ob >> 21) & 7;
    if (kind < 3) {
        unsigned char *r = *(unsigned char **)(ob + 0x18);
        base = **(unsigned int **)(p + 0x6C) & 0x3FF;
        if (r[0xE] == 0) {
            r[0] = 4;
        } else if (r[0xE] != 1) {
            r[0] = 3;
        }
    } else {
        base = (**(unsigned int **)(p + 0x6C) >> 10) & 0x3FF;
    }
    amt = *(unsigned short *)(*(unsigned char **)(ob + 0x18) + 0xC) - v - base;
    {
        int t = func_80071A54();
        unsigned char *p2 = D_8009D278;
        int lim = ob[0x90] * (100 - ((**(unsigned int **)(p2 + 0x6C) >> 20) & 0xFF));
        if (t % 100 < lim / 100) {
            amt = amt * 3 / 2;
            *(int *)(p2 + 0x4C) = *(int *)(p2 + 0x4C) | 0x8000;
        }
    }
    {
        unsigned char *p3 = D_8009D278;
        if ((*(int *)(p3 + 0x4C) & 0x200) == 0) {
            if (*(int *)(p3 + 0x34) > 0) {
                amt = amt / 2;
            }
            if (amt > 0) {
                if (*(*(unsigned char **)(ob + 0x18) + 1) == 0xA) {
                    amt = amt / 2;
                }
                *(unsigned short *)(p3 + 0xC) = *(unsigned short *)(p3 + 0xC) - amt;
            }
            if (*(short *)(D_8009D278 + 0xC) != 0) {
                func_8001F814(a0);
                if ((*(int *)(D_8009D254 + 0x98) & 0x100) == 0) {
                    D_8009D1D0 = a0;
                }
            }
        } else {
            if (amt > 0) {
                if (*(*(unsigned char **)(ob + 0x18) + 1) == 0xA) {
                    amt = amt / 2;
                }
                *(int *)(p3 + 8) = *(int *)(p3 + 8) -
                    amt * ((*(int *)(p3 + 0x28) << 2) / *(short *)(p3 + 0x1C));
            }
        }
    }
    if (*(*(unsigned char **)(ob + 0x18) + 1) != 0) {
        func_80020288(ob);
    }
}
