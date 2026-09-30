extern short D_8019AF56;
extern short D_8019AF58;
extern short D_8019AF5A;
extern short D_8019AF5C;
extern short D_8019AF64;
extern char D_8019AEFC;

char *func_8019A5D4(int a0, int a1, int a2)
{
    register int t asm("$2");
    int v;

    D_8019AF56 = (short)a1;
    D_8019AF58 = (short)a1;
    a1 = a1 + 1;
    t = a1 << 1;
    t = t + a1;
    t = t << 1;
    D_8019AF5C = (short)t;
    D_8019AF5A = (short)a2;
    if (a2 == 0) {
        v = 0xE;
    } else {
        v = 0x52;
    }
    D_8019AF64 = (short)v;
    return &D_8019AEFC;
}
