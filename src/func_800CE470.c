void func_800CE470(void *unused, signed char *state, signed char *counter) {
    if (++counter[3] == 6) {
        state[1] = 2;
    }
}
