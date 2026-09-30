typedef struct { short vx; short vy; short vz; short pad; } SVECTOR;

extern unsigned char *D_800E27A0;
extern unsigned short D_800E2350;
extern unsigned short D_800E2352;
extern unsigned short D_800E2354;
extern int func_80071A54();
extern void func_80078C34();

void func_800C8F94(int a0, int a1, unsigned char *a2)
{
    SVECTOR v;

    v.vx = -(func_80071A54() % 3 + 9);
    v.vy = -(func_80071A54() % 3 + 9);
    v.vz = func_80071A54() % 5 - 2;
    func_80078C34(*(int *)(D_800E27A0 + 0x238), &v, a2 + 0x10);
    *(unsigned short *)(a2 + 8) = D_800E2350;
    *(unsigned short *)(a2 + 0xA) = D_800E2352;
    *(unsigned short *)(a2 + 0xC) = D_800E2354;
    a2[2] = 0x14;
    a2[1] = 0;
}
