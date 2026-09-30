int func_8001A2F0(int **arg0) {
    int value = *arg0[0];
    int count = 0;

    while (value != 0) {
        value &= value - 1;
        count++;
    }

    *arg0[1] = count;
    return 1;
}
