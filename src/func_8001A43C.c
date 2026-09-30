extern int func_8005401C(void);
int func_8001A43C(unsigned char *a0) {
    int r = func_8005401C();
    *(unsigned int *)*(unsigned int *)a0 = r;
    return 1;
}
