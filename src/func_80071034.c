extern unsigned int D_8009D1A0;
extern void func_800716A4(void);
extern void func_8001A680(int *a0, int a1);

void func_80071034(int *a0, int *a1)
{
    if ((D_8009D1A0 & 2) != 0) {
        func_800716A4();
    } else if (*a1 != 0x15) {
        a0[26] = 0;
        a0[27] = 0;
        a0[28] = 0;
        func_8001A680(a0, 0x15);
        *a1 = 0x15;
    }
}
