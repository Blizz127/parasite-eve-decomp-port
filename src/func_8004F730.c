extern int D_8009CF38;
extern unsigned char *D_8009CF58;

extern void func_800512AC(int, int *);
extern void func_800525EC();

int func_8004F730(int a0, int a1) {
    int buf;

    if ((a1 & 0x10040) != 0) {
        if (D_8009CF38 != 0) {
            D_8009CF38 = 0;
            buf = D_8009CF58[4];
            func_800512AC(0, &buf);
        } else {
            func_800512AC(9, 0);
        }
        func_800525EC();
    }
    return 1;
}
