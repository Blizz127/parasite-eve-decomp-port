extern unsigned char *D_8009D2F0;
extern int func_8002FE78(int a0);
extern int func_8003010C(void *a0, int a1);

int func_80018004(unsigned char **a0) {
    unsigned char *s;

    s = D_8009D2F0;
    if (s[0xC] == 0) {
        *(int *)a0[1] = func_8002FE78(*a0[0]);
    } else {
        *(int *)a0[1] = func_8003010C(s, *a0[0]);
    }
    return 1;
}
