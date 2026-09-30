extern unsigned char D_800B0CE4;
extern unsigned char D_800B0CE5;
extern unsigned char D_800B0CE6;
extern unsigned char D_800B0CD8[];
extern int D_8009D1A0;
extern int D_8009D2E8;

int func_8006C4C4(int arg0) {
    unsigned char *var_a2;
    int var_a1;
    unsigned char temp_a0;
    unsigned int temp_v0;
    unsigned int temp_v1;

    var_a1 = arg0;
    var_a2 = D_800B0CD8;
    if (var_a1 == -1) {
        temp_a0 = D_800B0CE5;
        D_800B0CE4 = temp_a0;
        var_a1 = (signed char)temp_a0;
        D_800B0CE6 |= 3;
    }
    if ((D_8009D1A0 & 2) != 0) {
        goto block_1;
    }
    if ((*(int *)var_a2 & 2) == 0) {
        goto block_2;
    }
block_1:
    D_800B0CE6 |= 2;
    D_8009D2E8 &= ~2;
block_2:
    temp_v1 = *(unsigned char *)(var_a2 + 0xE);
    if ((temp_v1 & 4) != 0) {
        temp_v0 = (temp_v1 | 3) & 0xFB;
        *(unsigned char *)(var_a2 + 0xE) = temp_v0;
    }
    if ((unsigned int)(var_a1 - 1) < 8U) {
        if (var_a1 != *(signed char *)(var_a2 + 0xD)) {
            temp_v0 = *(unsigned char *)(var_a2 + 0xE);
            temp_v1 = var_a1;
            *(signed char *)(var_a2 + 0xD) = temp_v1;
            *(signed char *)(var_a2 + 0xC) = temp_v1;
            *(unsigned char *)(var_a2 + 0xE) = temp_v0 | 1;
        }
    }
    return 0;
}
