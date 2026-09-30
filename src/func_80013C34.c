/*
 * func_80013C34 - field-VM handler: turn the camera toward a target object.
 *
 * VRAM 0x80013C34 / file 0x4434 / size 0x250 (148 words). asm/disc1/3B00.s.
 *
 * era -O2 -G8 with
 *   MASPSX_FORCE_ABSOLUTE_SYMBOLS=D_8009D2F0,D_8009D254,D_8009D20C
 * (the VM state block D_8009D300 at 0x590($gp) and the VM cursor D_8009CE00 at
 * 0x90($gp) must stay gp-relative; the three object/state pointers must not).
 * Verified LINK_EXACT, 0 word mismatches, zero pad.
 *
 * Levers that were load-bearing here:
 *  - `for (obj = HEAD; obj != 0; obj = obj->next) { if (match) break; }` is the
 *    only spelling that gives retail's pre-test + body + advance + bottom
 *    `bnez` layout; a `do {...} while (obj)` with a trailing `if (obj == 0)`
 *    lets cc1 rotate the loop and merge the two null checks (117 mismatches).
 *  - The D_8009D254 arm's null check must NOT cross-jump into the loop's.
 *    Retail tests the value in $v0 and copies it to $a2 in the branch delay
 *    slot; reproduce with `register T x asm("$2")` + a zero-code
 *    `__asm__ __volatile__("" : "=r"(x) : "0"(x))` barrier (the pin alone is
 *    coalesced away).  Same treatment for the operand id.  133 -> 10.
 *  - cc1 reassociates `cd + 0x1000 - t`; hoist the addition into its own local
 *    to keep retail's `addiu $v0,$a1,0x1000` / `subu $v0,$v0,$v1`.
 *  - The func_80079FB4 call evaluates its SECOND argument's subtraction first,
 *    so both differences go into named locals in that order.
 */
extern unsigned char *D_8009D300;
extern int D_8009CE00;
extern unsigned char *D_8009D254;
extern unsigned char *D_8009D20C;
extern unsigned char *D_8009D2F0;

extern int func_80079FB4(int a0, int a1);

int func_80013C34(int **a0) {
    unsigned char *st;
    unsigned char *obj;
    unsigned short fl;
    int step;
    int id;
    int t;
    int cd;
    int res;
    int diff;

    st = D_8009D300;
    fl = *(unsigned short *)(st + 8);
    if ((fl & 0x20) == 0) {
        {
            register int idv asm("$2");
            idv = *a0[0];
            __asm__ __volatile__("" : "=r"(idv) : "0"(idv));
            if (idv == 0) {
                register unsigned char *o1 asm("$2");
                o1 = D_8009D254;
                __asm__ __volatile__("" : "=r"(o1) : "0"(o1));
                if (o1 == 0) {
                    return 1;
                }
                obj = o1;
                goto have;
            }
            id = idv;
        }
        for (obj = D_8009D20C; obj != 0; obj = *(unsigned char **)(obj + 4)) {
            if (obj[0xC] == id && obj[0xD] == *a0[1] &&
                (*(int *)(obj + 0x98) & 0x10) == 0) {
                break;
            }
        }
        if (obj == 0) {
            return 1;
        }
    have:
        {
            unsigned char *s2;
            step = *a0[2];
            s2 = D_8009D300;
            *(unsigned char **)(s2 + 0x18) = obj;
            *(unsigned short *)(s2 + 8) = *(unsigned short *)(s2 + 8) | 0x20;
            *(int *)(s2 + 0x14) = step;
        }
    } else {
        obj = *(unsigned char **)(st + 0x18);
        if (*(int *)(obj + 0x98) & 0x10) {
            *(unsigned short *)(st + 8) = fl & 0xFFDF;
            return 1;
        }
        step = *(int *)(st + 0x14);
    }

    {
        int dy = *(int *)(D_8009D2F0 + 0x28) - *(int *)(obj + 0x28);
        int dx = *(int *)(D_8009D2F0 + 0x30) - *(int *)(obj + 0x30);
        t = (0x1400 - func_80079FB4(dx >> 16, dy >> 16)) & 0xFFF;
    }
    cd = *(short *)(D_8009D2F0 + 0x3A);
    res = t;
    if (t != cd) {
        if (cd < t) {
            diff = t - cd;
            if (diff < 0x800) {
                if (step < diff) {
                    res = cd + step;
                }
            } else if (step < diff) {
                res = cd - step;
                if (res < 0) {
                    int x = cd + 0x1000;
                    if (x - t < step) {
                        res = t;
                    }
                }
            }
        } else {
            diff = cd - t;
            if (diff < 0x800) {
                if (step < diff) {
                    res = cd - step;
                }
            } else if (step < diff) {
                res = cd + step;
                if (res > 0x1000) {
                    int x = t + 0x1000;
                    if (x - cd < step) {
                        res = t;
                    }
                }
            }
        }
        res = res & 0xFFF;
        *(short *)(D_8009D2F0 + 0x3A) = res;
        if (res != t) {
            D_8009CE00 = D_8009CE00 - 0x14;
            *(int *)(D_8009D300 + 0x10) = 1;
            return 0;
        }
    }
    *(unsigned short *)(D_8009D300 + 8) = *(unsigned short *)(D_8009D300 + 8) & 0xFFDF;
    return 1;
}
