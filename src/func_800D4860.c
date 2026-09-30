extern int D_800B0E20[];
extern int D_800F32D4;
extern int D_800F32D8;
extern int D_800F3418;
extern int D_800F342C;
extern int D_800F3474;
extern int D_800E22D4;

extern void func_800CECAC(void);
extern int func_8006E498(int a0, int a1);
extern void func_800CED3C(int a0);

int func_800D4860(void)
{
    func_800CECAC();
    D_800F32D4 = func_8006E498(D_800B0E20[0], 0x6DEAE684);
    D_800F32D8 = func_8006E498(D_800B0E20[0], 0x8AB5B684);
    D_800F3418 = func_8006E498(D_800B0E20[0], 0x5AB32504);
    D_800F342C = func_8006E498(D_800B0E20[0], 0x5AB32104);
    D_800F3474 = func_8006E498(D_800B0E20[0], 0x83ACD504);
    D_800E22D4 = func_8006E498(D_800B0E20[0], 0x57E8EB04);
    func_800CED3C(0);
    return 0;
}
