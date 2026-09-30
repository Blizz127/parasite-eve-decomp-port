extern int *D_8009D2F0;
extern void *D_8009D254;
extern int func_8003708C(int a0, int a1);
extern int func_80077CF4(int a0);
extern int func_80077DC4(int a0);

int func_80018598(void) {
    int t;

    if (D_8009D2F0 == D_8009D254) {
        t = func_8003708C(0x50000, D_8009D2F0[8]);
    } else {
        t = D_8009D2F0[8];
    }
    t = func_8003708C(t, ((unsigned short *)D_8009D2F0)[0x13] << 4);
    D_8009D2F0[0x1A] = func_8003708C(-t, func_80077CF4(((short *)D_8009D2F0)[0x1D]) << 4);
    D_8009D2F0[0x1C] = func_8003708C(-t, func_80077DC4(((short *)D_8009D2F0)[0x1D]) << 4);
    return 1;
}
