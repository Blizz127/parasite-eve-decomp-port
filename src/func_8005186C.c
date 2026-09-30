/* Phase 5ET: loop-as-volume leaf (era path). */
int func_8005186C(int value) {
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
