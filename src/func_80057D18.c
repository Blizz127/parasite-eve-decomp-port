extern short *D_8009D048;

int func_80057D18(int index) {
    int value = D_8009D048[index];
    D_8009D048[index] = 0;
    return value;
}
