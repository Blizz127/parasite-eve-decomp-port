/*
 * func_800155FC — mode-7 / pause gate (retail 0x800155FC).
 *
 * VRAM 0x800155FC / file 0x5DFC / size 0x4C (19 words). Unit 38B4.
 *
 * Reads D_800BCF88 and returns 1 when its low three bits are 4 or 0;
 * otherwise decrements the gp-relative frame budget D_8009CDF8+8 (0x90($gp))
 * by 8, sets the flag at *(D_8009D300)+0x10 (0x590($gp)) and returns 0.
 *
 * Build: era -O2 -G8. D_8009CDF8 and D_8009D300 are scalars so both gp
 * accesses stay small-data-relative. D_800BCF88 (0x800BCF88) is far outside
 * the $gp window and must stay absolute, so it is declared as an incomplete
 * array — that emits no `.extern D_800BCF88, N` and maspsx keeps the load
 * absolute. `D_8009D300` is declared `char *` so the load of the pointer and
 * the `sw` through it match retail's `lw $a0,0x590($gp); sw $v1,0x10($a0)`.
 */
extern int D_8009CDF8;
extern char *D_8009D300;
extern int D_800BCF88[];

int func_800155FC(void) {
    int temp_v1;

    temp_v1 = D_800BCF88[0] & 7;
    if (temp_v1 == 4 || temp_v1 == 0) {
        return 1;
    }
    *(int *)((char *)&D_8009CDF8 + 8) -= 8;
    *(int *)(D_8009D300 + 0x10) = 1;
    return 0;
}
