/*
 * func_8006DF50 — lookup a stream record, forward 3 args to the copy helper.
 *
 * VRAM 0x8006DF50 / file 0x5E750 / size 0x58 (22 words).
 *
 * Retail (asm/disc1/5E39C.s):
 *   r = func_8006E514(a0, a1);
 *   if (r == 0) return -1;
 *   return func_80086608(r, a2, a3, a4);
 *
 * Build: era -O2 -G0.
 * ROM: asm/disc1/5E39C.s @ file 0x5E750, 22 words (0x58 bytes).
 */

extern int func_8006E514(int a0, int a1);
extern int func_80086608(int a0, int a1, int a2, int a3);

int func_8006DF50(int a0, int a1, int a2, int a3, int a4) {
    int r;

    r = func_8006E514(a0, a1);
    if (r == 0)
        return -1;
    return func_80086608(r, a2, a3, a4);
}
