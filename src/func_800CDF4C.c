typedef struct {
    char c0;
    signed char f1;
    char c2;
    signed char f3;
    char c4[0xC];
    short x[8];
    short y[8];
    short z[8];
    short dx[8];
    short dy[8];
    short dz[8];
} S;

void func_800CDF4C(int a0, char *a1, S *a2)
{
    unsigned short i;

    a2->f1 -= 0x10;
    a2->f3 += 0xC;
    for (i = 0; i < 8; i++) {
        a2->x[i] += a2->dx[i];
        a2->y[i] += a2->dy[i];
        a2->z[i] += a2->dz[i];
    }
    if (a2->f1 < 0x10) {
        a1[1] = 2;
    }
}
