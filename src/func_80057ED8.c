extern int D_8009D078;
extern short D_800A1FD4[];
int func_80057ED8(int a0) {
    if (a0 < 0) return 0;
    if (a0 >= D_8009D078) return 0;
    return D_800A1FD4[a0];
}
