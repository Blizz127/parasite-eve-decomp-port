extern int *D_8009D2F0;
extern int func_800370A8(int a0, int a1);
extern int func_8003708C(int a0, int a1);

int func_80017444(int **a0) {
    int sh;
    int dx;
    int dy;
    int dz;
    int t;

    D_8009D2F0[0x26] |= 2;
    dx = D_8009D2F0[0xA] - **a0;
    sh = *a0[3] << 16;
    dy = D_8009D2F0[0xB] - *a0[1];
    dz = D_8009D2F0[0xC] - *a0[2];
    D_8009D2F0[0x1A] = func_800370A8(dx, sh);
    D_8009D2F0[0x1C] = func_800370A8(dz, sh);
    D_8009D2F0[0x1B] = func_8003708C(sh + 0x10000, D_8009D2F0[0x23]) >> 1;
    t = func_800370A8(dy, sh);
    D_8009D2F0[0x1B] = t + D_8009D2F0[0x1B];
    return 1;
}
