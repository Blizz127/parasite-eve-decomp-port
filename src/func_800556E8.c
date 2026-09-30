extern int D_8009D040;
extern short D_800A1D9C[];
int func_800556E8(int a0) {
    if (a0 < 0)
        return 0;
    if (a0 >= D_8009D040)
        return 0;
    return D_800A1D9C[a0];
}
