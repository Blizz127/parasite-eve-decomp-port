typedef struct {
    char c0;
    char c1;
    char c2;
    signed char f3;
    short f4;
    unsigned short f6;
    unsigned short f8;
    unsigned short fA;
} S;

extern short D_800F3380[];
extern short D_800F3382;
extern short D_800F3384;
extern int D_800F3390;
extern int D_800F3394;
extern int D_800F3398;
extern short D_800F33A8;

extern void func_800C2EAC(int a0);
extern void func_800C3098(int a0);
extern void func_800C2FF0(int a0, int a1);
extern void func_800C3238(int a0);
extern void func_800C3B04(short *a0);

void func_800CC6F8(int a0, int a1, S *a2)
{
    register int v asm("$2");
    unsigned char c;

    func_800C2EAC(3);
    func_800C3098(0x100);
    func_800C2FF0(0x20, 0x20);
    func_800C3238(2);
    v = a2->f6;
    D_800F3380[0] = v;
    v = a2->f8;
    D_800F3382 = v;
    v = a2->fA;
    D_800F3384 = v;
    v = a2->f4;
    D_800F3390 = v;
    v = a2->f4;
    D_800F3394 = v;
    v = a2->f4;
    D_800F3398 = v;
    c = a2->f3;
    D_800F33A8 = (signed char)c;
    func_800C3B04(D_800F3380);
}
