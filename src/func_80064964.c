extern void func_80071A24(void *a0, int a1);
extern unsigned char D_800A3060[];
extern signed char D_800A3078;
extern signed char D_800A30A0;
extern signed char D_800A30B0;
extern signed char D_800A30B8;
extern signed char D_800A30C0;
extern signed char D_800A30C4;
extern signed char D_800A3124;
extern signed char D_800A3134;

void func_80064964(void)
{
    func_80071A24(D_800A3060, 0x120);
    D_800A3078 = -1;
    D_800A30A0 = -1;
    D_800A30B0 = -1;
    D_800A30B8 = -1;
    D_800A30C0 = -1;
    D_800A30C4 = -1;
    D_800A3124 = -1;
    D_800A3134 = -1;
}
