/*
 * func_80014DA0 — polygon hit test wrapper (retail 0x80014DA0).
 *
 * VRAM 0x80014DA0 / file 0x55A0 / size 0x90 (36 words). Unit 38B4; carved out
 * of the [0x38B4, asm] run.
 *
 * Copies four 8-byte vertex pairs from arg0 into a 4-entry stack polygon
 * (each entry: word at +0x00 and word at +0x04, both double-dereferenced),
 * runs func_8001CAB0(**(arg0+0x20), **(arg0+0x24), buf, 4) and stores the
 * result through *(arg0+0x28). Always returns 1.
 *
 * Build: era -O2 -G8. The destination pointer must be assigned before the
 * source pointer (`dst = buf; src = arg0;`): with `src` first cc1 gives the
 * stack buffer $v1 and the source $a0, swapping every load in the loop.
 */
extern int func_8001CAB0(int, int, int *, int);

int func_80014DA0(char *arg0) {
    int buf[8];
    unsigned int n;
    char *src;
    int *dst;

    n = 0;
    dst = buf;
    src = arg0;
    do {
        *dst = *(int *)(*(int *)src);
        n += 1;
        dst[1] = *(int *)(*(int *)(src + 4));
        src += 8;
        dst += 2;
    } while (n < 4);
    *(int *)(*(int *)(arg0 + 0x28)) = func_8001CAB0(
        *(int *)(*(int *)(arg0 + 0x20)),
        *(int *)(*(int *)(arg0 + 0x24)),
        buf, 4);
    return 1;
}
