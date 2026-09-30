/* Byte-identical twin of func_8005186C (era -O2 -G0).
 *
 * func_800DFC44 at file 0xD0444 / VRAM 0x800DFC44, size 0x3C (15 words), is
 * byte-for-byte equal to the already-matched loop-as-volume leaf
 * func_8005186C in the retail image, so the same C body matches. Proved by
 * scripts/build_us.sh: candidate SHA-1 452fb033... equals retail.
 */
int func_800DFC44(int value) {
    int result;
    int shift;
    int trial;

    result = 0;
    shift = 0x1E;
    do {
        trial = ((result << 2) + 1) << shift;
        result <<= 1;
        if (value >= trial) {
            value -= trial;
            result |= 1;
        }
        shift -= 2;
    } while (shift >= 0);
    return result;
}
