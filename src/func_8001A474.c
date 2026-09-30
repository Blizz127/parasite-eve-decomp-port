extern int func_80052F70(void);
int func_8001A474(unsigned char *a0) {
    int r = func_80052F70();
    *(unsigned int *)*(unsigned int *)a0 = r;
    return 1;
}
