/*
 * func_800370A8 — fixed-point quotient helper.
 *
 * VRAM 0x800370A8 / file 0x278A8 / size 0x14.
 */
int func_800370A8(int value, int divisor) {
    return (value / (divisor >> 8)) << 8;
}
