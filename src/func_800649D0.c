extern int D_8009D16C;
extern void func_80071A24(void *a0, int a1);
extern unsigned char D_800A3060[0x120];
extern signed char D_800A3078[16];
extern signed char D_800A30A0[16];
extern signed char D_800A30B0[16];
extern signed char D_800A30B8[16];
extern signed char D_800A30C0[16];
extern signed char D_800A30C4[16];
extern signed char D_800A3124[16];
extern signed char D_800A3134[16];

void func_800649D0(int a0)
{
    D_8009D16C = a0;
    if (a0 == 0) {
        func_80071A24(D_800A3060, 0x120);
        D_800A3078[0] = -1;
        D_800A30A0[0] = -1;
        D_800A30B0[0] = -1;
        D_800A30B8[0] = -1;
        D_800A30C0[0] = -1;
        D_800A30C4[0] = -1;
        D_800A3124[0] = -1;
        D_800A3134[0] = -1;
    }
}
