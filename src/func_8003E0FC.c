int func_8003E0FC(unsigned char *a0, short key, int *out) {
    short *rec;
    int i;

    rec = *(short **)(a0 + 0x80);
    i = 0;
    while (i < (*(unsigned char **)a0)[3]) {
        if (rec[3] == key) {
            out[0] = rec[0];
            out[1] = rec[1];
            out[2] = rec[2];
            return 1;
        }
        i++;
        rec += 6;
    }
    return 0;
}
