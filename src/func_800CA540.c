void func_800CA540(void *unused, signed char *state, signed short *values) {
    values[2] -= 20;
    if (values[2] < 20) {
        values[2] = 0;
        state[1] = 2;
    }
}
