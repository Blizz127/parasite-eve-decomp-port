extern int D_8009D14C;
extern int D_8009D150[];

void func_800614AC(int a0)
{
    register int *p asm("$7");
    int b;
    int x;
    int y;
    int z;
    int res;
    int hi;

    D_8009D14C = a0 & 0xFFFFFF;
    b = (a0 >> 16) & 0xFF;
    x = (b + ((a0 >> 8) & 0xFF)) >> 1;
    p = D_8009D150;
    if (x >= 0x100) {
        x = 0xFF;
    }
    y = (b + (a0 & 0xFF)) >> 1;
    if (y < 0x100) {
        res = x | (y << 8);
    } else {
        res = x | 0xFF00;
    }
    z = (((a0 >> 8) & 0xFF) + (a0 & 0xFF)) >> 1;
    if (z >= 0x100) {
        hi = 0xFF0000;
    } else {
        hi = z << 16;
    }
    *p = res | hi;
}
