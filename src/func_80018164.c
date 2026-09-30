extern unsigned char *D_8009D2F0;
extern void func_8002FF78(int a0, int a1);
extern void func_80030220(void *a0, int a1, int a2);

int func_80018164(unsigned char **a0) {
    unsigned char *s;

    s = D_8009D2F0;
    if (s[0xC] == 0) {
        func_8002FF78(*a0[0], *(int *)a0[1]);
    } else {
        func_80030220(s, *a0[0], *(int *)a0[1]);
    }
    return 1;
}
