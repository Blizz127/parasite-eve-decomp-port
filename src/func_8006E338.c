extern signed char D_800930B4[];

int func_8006E338(unsigned int a0)
{
    int acc;
    int j;
    int pow;
    int k;

    acc = 0;
    for (j = 1; j < 5; j++) {
        pow = 1;
        for (k = 0; k < 4 - j; k++) {
            pow = pow * 10;
        }
        acc += (D_800930B4[(a0 >> ((5 - j) * 5 + 2)) & 0x1F] - 0x30) * pow;
    }
    return acc;
}
