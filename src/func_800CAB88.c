extern unsigned short D_800E2360;
extern unsigned short D_800E2364;
typedef struct { char p[0x2E]; short f2E; } S;
extern S *D_8009D254;

void func_800CAB88(int a0, int a1, short *a2)
{
    int y;
    int c;

    a2[4] = D_800E2360;
    y = D_8009D254->f2E;
    *(volatile short *)&a2[5] = y;
    c = D_800E2364;
    *(volatile short *)&a2[2] = 0x7F;
    *(volatile short *)&a2[3] = 0x224;
    a2[6] = c;
}
