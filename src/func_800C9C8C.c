typedef struct { short vx; short vy; short vz; short pad; } SVECTOR;

extern unsigned char *D_800E27A4;
extern unsigned short D_800E2358;
extern unsigned short D_800E235A;
extern unsigned short D_800E235C;
extern int func_80071A54();
extern void func_80078C34();

void func_800C9C8C(int a0, int a1, unsigned char *a2)
{
    SVECTOR v;

    v.vx = -(func_80071A54() % 3 + 9);
    v.vy = -(func_80071A54() % 3 + 9);
    v.vz = func_80071A54() % 5 - 2;
    func_80078C34(*(int *)(D_800E27A4 + 0x238), &v, a2 + 0x10);
    *(unsigned short *)(a2 + 8) = D_800E2358;
    *(unsigned short *)(a2 + 0xA) = D_800E235A;
    *(unsigned short *)(a2 + 0xC) = D_800E235C;
    a2[2] = 0x14;
    a2[1] = 0;
}
