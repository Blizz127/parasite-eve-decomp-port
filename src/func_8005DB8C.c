/*
 * func_8005DB8C — global word + record-base helper (fan-in 9).
 * VRAM 0x8005DB8C, size 0x20 (8 words), file 0x4E38C in asm/disc1/4E2FC.s.
 *
 * Returns `D_800A8038 + (a0 << 9) + (&D_800A8038 - 0x10)`: the caller gets
 * a record base pointer for slot `a0` biased by -0x10, plus the global word
 * at the head of the record table.
 *
 * Stage-0 typing: the function returns `int` and the low word is read with a
 * plain `lw` (no arith on the loaded value), so `D_800A8038` is typed `int`
 * here — the codegen is identical to `unsigned int`.
 *
 * Build: era -O2 -G0 (YAML default).
 */
extern int D_800A8038;
int func_8005DB8C(unsigned int a0) {
    unsigned char *base = (unsigned char *)&D_800A8038;
    unsigned int off = a0 << 9;
    unsigned char *q = base - 0x10;
    unsigned char *p = off + (unsigned int)q;
    return *(int *)base + (int)p;
}
