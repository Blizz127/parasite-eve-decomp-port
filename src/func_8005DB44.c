extern int D_800A8034;
extern int D_800A8038;
int func_8005DB44(unsigned int a0) {
    int *p = &D_800A8038;
    int end = *p;
    int start = D_800A8034;
    if (a0 >= ((unsigned int)(end - start) >> 5)) return 0;
    {
        int sh = a0 << 5;
        p -= 4;
        return start + (sh + (int)p);
    }
}
