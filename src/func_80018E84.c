extern unsigned short D_800BD020;
extern unsigned short D_800BD022;
int func_80018E84(int * volatile *arg0) {
    D_800BD020 = *(volatile int *)arg0[0];
    D_800BD022 = *(volatile int *)arg0[1];
    return 1;
}
