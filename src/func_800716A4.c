extern unsigned int D_8009D1A0;
extern void func_8001A680(int *a0, int a1);
extern void func_8001A784(int *a0);

void func_800716A4(int *a0, int *a1)
{
    register int t asm("$5");

    if ((D_8009D1A0 & 2) == 0) {
        if (*a1 != 0x15) {
            a0[26] = 0;
            a0[27] = 0;
            a0[28] = 0;
            func_8001A680(a0, 0x15);
            *a1 = 0x15;
        }
    } else {
        t = *(unsigned char *)(*(int *)a0 + 0x12);
        if (*a1 != t) {
            func_8001A784(a0);
            a0[26] = 0;
            a0[27] = 0;
            a0[28] = 0;
            *a1 = *(unsigned char *)(*(int *)a0 + 0x12);
        }
    }
}
