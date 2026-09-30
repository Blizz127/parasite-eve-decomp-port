/*
 * func_80030584 — angle helper. ratan2 of two signed shorts shifted
 * to 16.16 vs a vector (a1[0], a1[2]), then +2048 truncated to i16.
 *
 * VRAM 0x80030584 / file 0x20D84 / size 0x44 (17 words). Non-leaf:
 * frame -0x18, $ra at 0x10; second subu in the jal delay; (short)
 * truncate is sll/sra with lw $ra between; addiu $sp in the jr delay.
 *
 * era -O2 -G0 + maspsx 2.21 --dont-expand-li.
 * Head of 20D84: C 0x44, resume 20DC8.s 0x78, then existing 30640.
 */
extern int func_80079FB4(int x, int z);

int func_80030584(unsigned char *a0, int *a1) {
    int x = ((int)*(short *)(a0 + 0xB4)) << 16;
    int z = ((int)*(short *)(a0 + 0xB8)) << 16;
    int r = func_80079FB4(x - a1[0], z - a1[2]);
    return (int)(short)(r + 2048);
}
