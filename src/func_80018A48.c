extern unsigned int *D_8009D2F0;
extern void func_8002FAA4(void *a0, int a1, int a2, int a3, int a4, int a5);

int func_80018A48(unsigned char **a0) {
    func_8002FAA4(D_8009D2F0, *a0[0], *a0[1], *a0[2], *a0[3],
                  *(unsigned short *)a0[4]);
    return 1;
}
