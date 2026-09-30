/*
 * func_800C2E08 — clear active 6-byte records, return flag (retail 0x800C2E08).
 *
 * VRAM 0x800C2E08 / file 0xB3608 / size 0xA4 (41 words), inside B3390.s.
 *
 * Walks the 0x40 record slots at D_800F34F4 (stride 6). For every slot
 * whose byte +1 is nonzero, it clears that byte, decrements the count
 * byte D_800E2248[6], and ORs a flag that becomes -1 when the word at
 * D_800E2248+4 has the high halfword 0x0100. Returns the accumulated
 * flag (0 or -1). `off` is the strength-reduced i*6 cursor; the check
 * uses `off`, the clear recomputes `(i & 0xFFFF) * 6` (retail keeps both
 * forms, `$a2` and `$a1`).
 *
 * era -O2 -G0. Levers: D_800F34F4 must be an INTEGER (0x800F34F4 holds
 * the pointer value): `off + D_800F34F4` then emits `addu $v0,$a2,$a0`
 * as retail. The flag expression must be staged through a temp
 * (`int f = -cond; found |= f;`) — inlined `found |= -cond` makes cc1
 * if-convert into `bne`/`li` (15 diffs). The 0xFFFF0000/0x01000000 masks
 * must be locals declared between `i` and `off` so the entry block emits
 * `lui $t1; lui $t0` before `off = 0` as retail.
 */
extern unsigned int D_800F34F4;
extern unsigned char *D_800E2248;

int func_800C2E08(void) {
    int found = 0;
    int i = 0;
    unsigned int mask = 0xFFFF0000;
    unsigned int want = 0x01000000;
    int off = 0;
    do {
        if (*(signed char *)(off + D_800F34F4 + 1) != 0) {
            *(signed char *)((i & 0xFFFF) * 6 + D_800F34F4 + 1) = 0;
            D_800E2248[6] = D_800E2248[6] - 1;
            {
                int f = -((*(unsigned int *)(D_800E2248 + 4) & mask) == want);
                found |= f;
            }
        }
        i++;
        off += 6;
    } while (i < 0x40);
    return found;
}
