extern signed char D_800B0DBA;
extern signed char D_800B0DBB;
extern unsigned short D_800B0DBC;

int func_8006EC08(void)
{
    signed char a;
    signed char b;
    int v;

    a = D_800B0DBA;
    if (a == 0) {
        goto zero;
    }
    b = a;
    asm volatile("" : "=r"(b) : "0"(b));
    if (b == 0) {
        v = -1 << 16;
    } else {
        v = (int)D_800B0DBC << 16;
    }
    if (v <= 0) {
        goto zero;
    }
    if (D_800B0DBB != 0) {
        return 2;
    }
    return 1;
zero:
    return 0;
}
