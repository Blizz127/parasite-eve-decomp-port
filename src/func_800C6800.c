extern int func_800C6CE0(unsigned char *a0);
extern int func_8006DCE4(void *a0, void *a1, int a2, int a3, int a4);

int func_800C6800(unsigned char *a0, void *a1, unsigned char *a2) {
    int r;

    r = 0;
    if (func_800C6CE0(a0) == 3 || func_800C6CE0(a0) == 4) {
        unsigned char *v = *(unsigned char **)(a0 + 8);
        unsigned char *w = *(unsigned char **)v;
        r = func_8006DCE4(a1, *(void **)(w + 8),
                          *(short *)(a2 + 0x0), *(short *)(a2 + 0x2),
                          *(short *)(a2 + 0x4));
    }
    return r;
}
