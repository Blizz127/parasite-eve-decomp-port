/*
 * func_80030640 — RNG gate (Phase 5FJ volume leaf).
 * If record inner+0x10 has bit 16 set and RNG%100 < rec+0x22, store 9000
 * at rec+0x10.
 *
 * VRAM 0x80030640 / file 0x20E40 / size 0xA0 (40 words). Non-leaf:
 * frame -0x18, $ra at 0x14, $s0 at 0x10 (unsigned-short threshold).
 *
 * ROM `lui $v1,1` / `and` is 0x10000, not bit 0. Writing `& 1` emits
 * `andi` and is a hard mismatch. The signed `% 100` expansion clobbers
 * $a0, so the store reloads D_8009D278 into $v1 rather than reusing rec.
 *
 * era -O2 -G0 + maspsx 2.21 --dont-expand-li. Mid-20210 carve: prefix
 * 0xC30, C 0xA0, resume 20EE0.s.
 */
extern unsigned char *D_8009D278;
extern int func_80071A54(void);

void func_80030640(void) {
    unsigned char *rec;
    unsigned int *inner;
    unsigned short thresh;
    int rnd;

    rec = D_8009D278;
    inner = *(unsigned int **)(rec + 0x68);
    if ((inner[4] & 0x10000) == 0)
        return;
    thresh = *(unsigned short *)(rec + 0x22);
    rnd = func_80071A54();
    if ((rnd % 100) < (int)thresh) {
        unsigned char *rec2 = D_8009D278;
        *(short *)(rec2 + 0x10) = 9000;
    }
}
