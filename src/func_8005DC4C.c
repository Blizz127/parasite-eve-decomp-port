extern unsigned char D_800A8028;
extern unsigned int D_800A802C;

unsigned int func_8005DC4C(unsigned int a0) {
    unsigned char *rec = &D_800A8028 + D_800A802C;
    unsigned char *tbl = rec + *(int *)(rec + 4);
    if (a0 >= *(unsigned short *)tbl)
        return 0;
    return (unsigned int)(tbl + *(short *)(tbl + 2 + a0 * 2));
}
