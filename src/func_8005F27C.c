/* func_8005F27C — VRAM 0x8005F27C, size 0xD8, file 0x4FA7C-0x4FB54.
 *
 * String-decoder driver: pushes D_8009D124/D_8009D128 onto the
 * D_8009D12C stack seed, walks the NUL-terminated 0xFF-delimited byte
 * string feeding every character to func_8005EED4, then pops the two
 * saved words back.  Overflow raises func_800527C0(2); underflow,
 * func_800527C0(3).
 *
 * era -O2 -G8.  The three gp words are D_8009D12C (0x3BC), D_8009D124
 * (0x3B4) and D_8009D128 (0x3B8).  Retail homes the pop cursor in $v1
 * and the push cursor in $a1, the two saved words in $v0/$a0 and the
 * stack-limit bases absolutely, so the locals are pinned.
 */
extern int D_8009D12C;
extern int D_8009D124;
extern int D_8009D128;
extern unsigned char D_800A22B0[];
extern unsigned char D_800A2270[];
extern void func_800527C0(int);
extern void func_8005EED4(int);

void func_8005F27C(unsigned char *s) {
    register int *p asm("$5");
    register int *n asm("$3");
    register int t0 asm("$2");
    register int t1 asm("$4");
    unsigned char v;

    if (s != 0) {
        p = (int *)D_8009D12C;
        if ((unsigned int)p < (unsigned int)D_800A22B0) {
            n = p + 2;
            t0 = D_8009D124;
            t1 = D_8009D128;
            D_8009D12C = (int)n;
            p[0] = t0;
            p[1] = t1;
        } else {
            func_800527C0(2);
        }
        v = *s;
        if (v != 0xFF) {
            do {
                s++;
                func_8005EED4(v);
                v = *s;
            } while (v != 0xFF);
        }
        n = (int *)D_8009D12C;
        if ((unsigned int)D_800A2270 < (unsigned int)n) {
            t0 = n[-2];
            t1 = n[-1];
            D_8009D12C = (int)(n - 2);
            D_8009D124 = t0;
            D_8009D128 = t1;
            return;
        }
        func_800527C0(3);
    }
}
