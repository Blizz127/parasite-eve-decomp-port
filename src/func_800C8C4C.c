void func_800C8C4C(void *unused, signed char *state, signed short *values) {
    values[2] -= 20;
    if (values[2] < 20) {
        values[2] = 0;
        state[1] = 2;
    }
}
