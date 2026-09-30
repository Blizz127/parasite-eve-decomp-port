extern unsigned short D_800E2348;
extern unsigned short D_800E234C;
typedef struct { char p[0x2E]; short f2E; } S;
extern S *D_8009D254;
void func_800C811C(int a0, int a1, short *a2)
{
    int y;
    int c;
    a2[4] = D_800E2348;
    y = D_8009D254->f2E;
    *(volatile short *)&a2[5] = y;
    c = D_800E234C;
    *(volatile short *)&a2[2] = 0x7F;
    *(volatile short *)&a2[3] = 0x224;
    a2[6] = c;
}
