extern int D_8009D044;
extern short D_800A1E00[];
int func_80058E08(int a0) {
    if (a0 < 0)
        return 0;
    if (a0 >= D_8009D044)
        return 0;
    return D_800A1E00[a0];
}
